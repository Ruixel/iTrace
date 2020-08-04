#version 330 
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord; 

layout(location = 0) out vec4 Clouds;
layout(location = 1) out vec4 Volumetrics;
layout(location = 2) out vec4 DiffuseLighting; 
layout(location = 3) out vec4 SpecularLighting;
layout(location = 4) out vec4 Direct; 
layout(location = 5) out float CloudDepth; 


uniform sampler2D CloudsRaw; 
uniform sampler2D VolumetricsRaw; 
uniform sampler2D DiffuseLightingRaw; 
uniform sampler2D SpecularLightingRaw; 
uniform sampler2D DirectLightingRaw; 
uniform sampler2D CloudDepthRaw; 
uniform int CheckerStep; 

uniform sampler2D UpscaleData; 



float GetRoughness(inout vec3 Normal) {

	float L = length(Normal); 

	Normal /= L; 
	
	float Roughness = 1.0 - (L * 2.0 - 1.0); 
	return Roughness; 
}


ivec2 Offsets[4] = ivec2[](ivec2(1,0),ivec2(-1,0),ivec2(0,1),ivec2(0,-1)); 


float GetWeightRT(vec3 Normal, vec3 CenterNormal, float Depth, float CenterDepth) {
	
	float NormalComponent = pow(dot(Normal,CenterNormal)*0.5+0.5,32.0); 
	NormalComponent = max(NormalComponent, 1e-4f); 

	return NormalComponent; 

}

float GetWeightSpecularRT(vec3 Normal, vec3 CenterNormal, float Depth, float CenterDepth, float Roughness, float CenterRoughness) {
	float RoughnessDiffWeight = 1.0 / pow(1.0 + 50.0 * abs(Roughness - CenterRoughness), 2.0);
	float NormalComponent = pow(dot(Normal,CenterNormal)*0.5+0.5,32.0); 
	NormalComponent = max(NormalComponent, 1e-4f); 

	return NormalComponent * RoughnessDiffWeight; 
}



void main() {

	ivec2 Pixel = ivec2(gl_FragCoord.xy); 

	int Shift = int(Pixel.y % 2 != CheckerStep); 

	ivec2 BasePixel = Pixel / ivec2(2,1); 

	if(Pixel.x % 2 == Shift) {
		
		//average based on 4 neighbors 

		float TotalWeightRT = 0.0, TotalWeightSpecularRT = 0.0, TotalWeightVolumetrics = 0.0;  

		vec4 CenterData = texelFetch(UpscaleData, Pixel*2,0); 
		float CenterRoughness = GetRoughness(CenterData.xyz); 

		for(int i = 0; i < 4; i++) {
			
			ivec2 CurrentPixel = BasePixel + Offsets[i]; 
			ivec2 CurrentPixelHighRes = Pixel + Offsets[i]; 

			vec4 CurrentData = texelFetch(UpscaleData, CurrentPixelHighRes*2,0); 
			float CurrentRoughness = GetRoughness(CurrentData.xyz); 

			float WeightRT = GetWeightRT(CurrentData.xyz, CenterData.xyz, CurrentData.w, CenterData.w); 
			float WeightSpecularRT = GetWeightSpecularRT(CurrentData.xyz, CenterData.xyz, CurrentData.w, CenterData.w, CurrentRoughness, CenterRoughness); 
			float WeightVolumetric = 1.0f; 

			Clouds += texelFetch(CloudsRaw, CurrentPixel, 0); 
			Volumetrics += texelFetch(VolumetricsRaw, CurrentPixel, 0) * WeightVolumetric; 
			DiffuseLighting += texelFetch(DiffuseLightingRaw, CurrentPixel, 0) * WeightRT;
			SpecularLighting += texelFetch(SpecularLightingRaw, CurrentPixel, 0) * vec4(WeightSpecularRT,WeightSpecularRT,WeightSpecularRT,WeightRT);
			Direct += texelFetch(DirectLightingRaw, CurrentPixel, 0) * WeightRT;
			CloudDepth += texelFetch(CloudDepthRaw, CurrentPixel, 0).x;

			TotalWeightRT += WeightRT; 
			TotalWeightSpecularRT += WeightSpecularRT; 
			WeightVolumetric += WeightVolumetric; 

		
		}

		Volumetrics /= TotalWeightVolumetrics; 
		DiffuseLighting /= TotalWeightRT; 
		SpecularLighting.xyz /= TotalWeightSpecularRT; 
		SpecularLighting.w /= TotalWeightRT; 
		Direct /= TotalWeightRT; 
		Clouds /= 4.0f; 
		CloudDepth / 4.0f; 
	}
	else {
		
		Clouds = texelFetch(CloudsRaw, BasePixel, 0); 
		Volumetrics = texelFetch(VolumetricsRaw, BasePixel, 0); 
		DiffuseLighting = texelFetch(DiffuseLightingRaw, BasePixel, 0);
		SpecularLighting = texelFetch(SpecularLightingRaw, BasePixel, 0);
		Direct = texelFetch(DirectLightingRaw, BasePixel, 0);
		CloudDepth = texelFetch(CloudDepthRaw, BasePixel, 0).x;
 
	
	}
	
}