#version 330

in vec2 TexCoord; 

layout(location = 0) out vec4 DirectionSample; 
layout(location = 1) out vec3 OriginSample; 

uniform sampler2D SpecularDirection; 
uniform sampler2D UpscaleDataSpecular; 
uniform int CheckerStep; 

ivec2 Offsets[4] = ivec2[](ivec2(1,0),ivec2(-1,0),ivec2(0,1),ivec2(0,-1)); 

float GetWeightSpecularRT(vec3 Normal, vec3 CenterNormal, float Depth, float CenterDepth) {
	float Dot = dot(Normal,CenterNormal); 
	//Dot = 1-clamp((1-Dot)*100, 0.0, 1.0); 
	
	float NormalComponent = pow(max(Dot,0.0),16.0); 
	float DepthComponent = 1.0/pow(1.0+10.0*abs(Depth-CenterDepth),3.0); 
	
	NormalComponent = max(NormalComponent, 1e-7); 

	return NormalComponent * DepthComponent; 
}

void main() {

	ivec2 Pixel = ivec2(gl_FragCoord.xy); 

	int Shift = int(Pixel.y % 2 == CheckerStep); 
	
	ivec2 BasePixel = Pixel / ivec2(2,1); 

	vec4 CenterData = texelFetch(UpscaleDataSpecular, BasePixel,0); 

	if(Pixel.x % 2 == Shift) {
		
		float Weight = 0.0; 

		DirectionSample = vec4(0.0); 

		for(int i = 0; i < 4; i++) {

			ivec2 CurrentPixel = BasePixel + Offsets[i]; 
		
			vec4 CurrentData = texelFetch(UpscaleDataSpecular, CurrentPixel,0); 

			float WeightSpecularRT = GetWeightSpecularRT(CurrentData.xyz, CenterData.xyz, CurrentData.w, CenterData.w); 

			DirectionSample += texelFetch(SpecularDirection, CurrentPixel, 0) * WeightSpecularRT; 
			Weight += WeightSpecularRT; 

		}

		DirectionSample /= Weight; 


	}
	else {
		DirectionSample = texelFetch(SpecularDirection, BasePixel, 0); 
	}


}