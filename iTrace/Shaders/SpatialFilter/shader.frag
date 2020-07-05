#version 330

in vec2 TexCoord; 

layout(location = 0) out vec4 IndirectDiffuse;
layout(location = 1) out vec4 Volumetric; 
layout(location = 2) out vec4 IndirectSpecular;


uniform sampler2D InputPacked;
uniform sampler2D InputLighting; 
uniform sampler2D FrameCount; 
uniform sampler2D InputVolumetric;
uniform sampler2D InputSpecular; 
uniform sampler2D MotionVectors; 
uniform sampler2D Direct; 

uniform bool DoSpatial; 
uniform bool Final; 

uniform int Radius; 
uniform int StepSize; 
uniform mat4 IncidentMatrix; 
uniform int SubFrame; 
uniform bool NewFiltering; 

ivec2 States[] = ivec2[](
	ivec2(1, 1),
	ivec2(0, 1),
	ivec2(0, 0),
	ivec2(1, 0));

float Kernel[4] = float[](0.60933016268,0.80238141513,0.904644388297,1.0); 



float GetRoughness(inout vec3 Normal) {

	float L = length(Normal); 

	Normal /= L; 
	
	float Roughness = 1.0 - (L * 2.0 - 1.0); 
	return Roughness; 
}

float Luminance(vec3 x) {
	return dot(x, vec3(0.2126, 0.7152,0.0722)); 
}








float WeightHeavy(vec4 LightingFetch, vec4 BaseFetch, float FrameCount, float BaseWeight, int StepSize) {
	
	float Weight = BaseWeight; 

	float power = mix(1.0,1000.0,min((FrameCount-24.0)/30.0,1.0)); 

	float OldWeight = Weight; 

	Weight *= max(1.0-power*(abs(BaseFetch.w-LightingFetch.w)/min(BaseFetch.w,LightingFetch.w)),0.0); 

	if(BaseFetch.w < 0.001 || LightingFetch.w < 0.001) {
		return 0.0; 
	}
				
	//temporary, might not be used in the future! 
				
	float L1 = Luminance(LightingFetch.xyz); 
	float L2 = Luminance(BaseFetch.xyz); 

	Weight *= max(1.0-power*(abs(L1-L2)/min(L1,L2)),1.0); 

	if(StepSize <= 3) {
		Weight = mix(Weight, OldWeight, 0.125); 
	}

	return Weight; 

}




void main() {

	
	ivec2 Pixel = ivec2(gl_FragCoord.xy); 
	
	ivec2 HighResPixel = Pixel * 2 + States[SubFrame];

	vec4 BasePacked = texelFetch(InputPacked, HighResPixel, 0); 

	float TotalWeight = 1.0; 
	float TotalWeightAO = TotalWeight; 
	float TotalWeightVolumetric = TotalWeight; 
	float TotalWeightSpecular = TotalWeight; 


	vec2 MotionVectors = texelFetch(MotionVectors, HighResPixel, 0).xy; 

	vec2 NewCoordinate = vec2(HighResPixel) / textureSize(FrameCount, 0).xy + MotionVectors;  

	float Frame = texture(FrameCount, NewCoordinate, 0).x;

	if(MotionVectors.x < -.3 || MotionVectors.y < -.3) 
		Frame = 0; 

	vec4 BaseIndirectDiffuse = texelFetch(InputLighting, Pixel, 0); 
	vec4 BaseIndirectSpecular = texelFetch(InputSpecular, Pixel, 0); 

	IndirectDiffuse = BaseIndirectDiffuse * TotalWeight; 
	Volumetric = texelFetch(InputVolumetric, Pixel, 0) * TotalWeightVolumetric; 
	IndirectSpecular = BaseIndirectSpecular * TotalWeight; 

	float BaseRoughness = GetRoughness(BasePacked.xyz); 
	
	
	if((Frame > 7.0 && StepSize > 5) || !DoSpatial ) {
	
		IndirectDiffuse /= TotalWeight; 
		Volumetric /= TotalWeightVolumetric; 
		IndirectSpecular /= TotalWeightSpecular; 
		if(Final) {
		IndirectSpecular.w = texelFetch(Direct, Pixel,0).x; 
		}
		return; 
	}

	for(int x = -2; x <= 2; x++) {
		for(int y = -2; y <= 2; y++) {
			
			if(x == 0 && y == 0)  
				continue; 
				
			ivec2 NewPixel = Pixel + ivec2(x,y) * StepSize;
			ivec2 NewPixelHighRes = HighResPixel + ivec2(x,y) * StepSize * 2; 

			vec4 CurrentPackedData = texelFetch(InputPacked, NewPixelHighRes, 0); 

			float CurrentRoughness = GetRoughness(CurrentPackedData.xyz); 

			float NormalWeight = pow(max(dot(CurrentPackedData.xyz, BasePacked.xyz), 0.0), 16.0); 

			float Weight = NormalWeight;  

			Weight /= pow(1.0 + 3.0 * abs(CurrentPackedData.w - BasePacked.w), 4.0); 


			vec4 LightingSample = texelFetch(InputLighting, NewPixel, 0); 
			vec4 SpecularLightingSample = texelFetch(InputSpecular, NewPixel, 0); 
			
			float WeightSpecular = Weight * NormalWeight; 

			WeightSpecular /= pow(1.0 + 50.0 * abs(CurrentRoughness - BaseRoughness), 5.0); 

			if(Frame > 24.0 && NewFiltering) {

				Weight = WeightHeavy(LightingSample, BaseIndirectDiffuse, Frame, Weight, StepSize); 
				WeightSpecular = WeightHeavy(SpecularLightingSample, BaseIndirectSpecular, Frame, WeightSpecular, StepSize); 

			}

			float BaseKernelWeight = Kernel[3-abs(x)] * Kernel[3-abs(y)]; 
			float KernelWeight = BaseKernelWeight; 
			if((Frame > 24.0 && StepSize > 3 && NewFiltering) || (Frame > 48.0 && NewFiltering)) {
				KernelWeight = pow(KernelWeight,(StepSize > 3 && Frame > 48.0) ? 10.0 : 5.0); 
			}
				 


			IndirectDiffuse.xyz += LightingSample.xyz * Weight * KernelWeight; 
			IndirectSpecular.xyz += SpecularLightingSample.xyz * WeightSpecular * KernelWeight; 
			TotalWeight += Weight * KernelWeight; 
			TotalWeightSpecular += WeightSpecular * KernelWeight; 

			float WeightVolumetric = BaseKernelWeight; 

			WeightVolumetric /= pow(1.0 + 3.0 * abs(CurrentPackedData.w - BasePacked.w) / min(CurrentPackedData.w, BasePacked.w), 4.0); 

			Volumetric += texelFetch(InputVolumetric, NewPixel, 0) * WeightVolumetric;  
			TotalWeightVolumetric += WeightVolumetric; 

			//AO: 

			if((StepSize == 1 && Frame < 20.0) || (StepSize == 2 && Frame < 12.0) || Frame < 8.0) {

				float WeightAO = Weight * pow(Kernel[3-abs(x)] * Kernel[3-abs(y)], mix(1.0, 1.5, clamp(Frame/30.0,0.0,1.0))); 

				IndirectDiffuse.w += LightingSample.w * WeightAO; 
				IndirectSpecular.w += SpecularLightingSample.w * WeightAO; 
				TotalWeightAO += WeightAO; 



			}


		}
	}
	IndirectDiffuse.xyz /= TotalWeight; 
	IndirectDiffuse.w /= TotalWeightAO; 
	Volumetric /= TotalWeightVolumetric;
	IndirectSpecular.xyz /= TotalWeightSpecular; 
	IndirectSpecular.w /= TotalWeightAO; 

	if(Final) {
		IndirectSpecular.w = texelFetch(Direct, Pixel,0).x; 
	}



}