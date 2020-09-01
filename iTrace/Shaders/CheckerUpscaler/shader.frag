#version 330 
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord; 

layout(location = 0) out vec4 OutDetail;


uniform sampler2D Detail; 
uniform sampler2D Depth; 
uniform sampler2D Normal; 
uniform sampler2D UpscaleData; 

uniform int State; 
uniform int CheckerStep; 


uniform float znear; 
uniform float zfar; 

float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
} 

float GetRoughness(inout vec3 Normal) {

	float L = length(Normal); 

	Normal /= L; 
	
	float Roughness = 1.0 - (L * 2.0 - 1.0); 
	return Roughness; 
}


ivec2 Offsets[4] = ivec2[](ivec2(1,0),ivec2(-1,0),ivec2(0,1),ivec2(0,-1)); 


float GetWeightRT(vec3 Normal, vec3 CenterNormal, float Depth, float CenterDepth) {
	
	float NormalComponent = pow(dot(Normal,CenterNormal)*0.5+0.5,32.0); 
	float DepthComponent = 1.0/pow(1.0+10.0*abs(Depth-CenterDepth),3.0); 
	NormalComponent = max(NormalComponent, 1e-4f); 

	return NormalComponent * DepthComponent; 

}

float GetWeightSpecularRT(vec3 Normal, vec3 CenterNormal, float Depth, float CenterDepth, float Roughness, float CenterRoughness) {
	float RoughnessDiffWeight = 1.0 / pow(1.0 + 50.0 * abs(Roughness - CenterRoughness), 2.0);
	
	
	float Dot = dot(Normal,CenterNormal); 
	Dot = 1-clamp((1-Dot)*100, 0.0, 1.0); 
	
	float NormalComponent = pow(max(Dot,0.0),128.0); 
	float DepthComponent = 1.0/pow(1.0+10.0*abs(Depth-CenterDepth),3.0); 
	

	NormalComponent = max(NormalComponent, 1e-7); 



	return NormalComponent ; 
}


ivec2 States[] = ivec2[](
	ivec2(1, 1),
	ivec2(0, 1),
	ivec2(0, 0),
	ivec2(1, 0));

void main() {

	ivec2 Pixel = ivec2(gl_FragCoord.xy); 

	int Shift = int(Pixel.y % 2 == CheckerStep); 

	ivec2 BasePixel = Pixel / ivec2(2,1); 
	vec4 CenterData = texelFetch(UpscaleData, BasePixel,0); 

	if(Pixel.x % 2 == Shift) {
		
		//average based on 4 neighbors 

		float TotalWeightRT = 0.0;  

		
		float CenterRoughness = GetRoughness(CenterData.xyz); 

		OutDetail = vec4(0.0); 

		for(int i = 0; i < 4; i++) {
			
			ivec2 CurrentPixel = BasePixel + Offsets[i]; 
			ivec2 CurrentPixelHighRes = Pixel + Offsets[i]; 

			vec4 CurrentData = texelFetch(UpscaleData, CurrentPixel,0); 

			float CurrentRoughness = GetRoughness(CurrentData.xyz); 

			float WeightRT = GetWeightRT(CurrentData.xyz, CenterData.xyz, CurrentData.w, CenterData.w); 
			
			OutDetail += texelFetch(Detail, CurrentPixel, 0) * WeightRT; 

			TotalWeightRT += WeightRT; 

		
		}

		OutDetail /= TotalWeightRT; 

		
	}
	else {
		
		OutDetail = texelFetch(Detail, BasePixel, 0); 

	}
	

}