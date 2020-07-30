#version 330

in vec2 TexCoord; 
layout(location = 0) out vec4 SHy;
layout(location = 1) out vec4 IndirectSpecularCo;
#ifdef SPATIAL_FINAL 
layout(location = 2) out vec4 OutDirect; 
#else 
layout(location = 2) out float SHCg; 
#endif 

uniform sampler2D InputPacked;
uniform sampler2D InputLighting; 
uniform sampler2D FrameCount; 
uniform sampler2D InputSpecular; 
uniform sampler2D MotionVectors; 
uniform sampler2D Direct; 
uniform sampler2D Detail;
uniform sampler2D InputSHCg; 


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

float Kernel[4] = float[](0.0,1.0/3.0,2.0/3.0,1.0); 

float GetRoughness(inout vec3 Normal) {

	float L = length(Normal); 

	Normal /= L; 
	
	float Roughness = 1.0 - (L * 2.0 - 1.0); 
	return Roughness; 
}

float Luminance(vec3 x) {
	return dot(x, vec3(0.2126, 0.7152,0.0722)); 
}


float WeightNew(float Luminance, float LuminanceCenter, 
				float LuminanceSqr, float LuminanceSqrCenter,
				float LuminanceDetail, float LuminanceDetailCenter, vec3 Normal, vec3 NormalCenter, 
				float Depth, float DepthCenter, 
				float Traversal, float TraversalCenter, 
				float phiIndirect, float FrameCount, float Facing) {

	//normal weight -> 

	float FavorMixer = clamp((FrameCount-6.0)/4.0,0.0,1.0); 

	phiIndirect = mix(1000.0, phiIndirect, FavorMixer); 
	
	float WeightFactor = mix(0.5,6.0,Facing); 
	WeightFactor = 0.5; 
	
	float DepthWeight = 1.0 / pow(1.0 + WeightFactor * (abs(Depth - DepthCenter) / min(clamp(Depth, 0.1,DepthCenter),1.0)), 3.0); 
	
	float WeightIndirectLuminance = abs(Luminance - LuminanceCenter) / phiIndirect; 
	float WeightDetail = 1.0 / (1.0 + 4.0 * abs(LuminanceDetail-LuminanceDetailCenter)); 
	float WeightTraversal = 1.0 / (1.0 + 4.0 * abs(Traversal-TraversalCenter)/max(min(Traversal, TraversalCenter),0.01)); 

	FavorMixer = clamp((FrameCount-8.0)/24.0,0.0,1.0); 

	WeightDetail = mix(1.0, WeightDetail, FavorMixer); 
	WeightTraversal = mix(1.0, WeightTraversal, FavorMixer); 

	float NormalBias = mix(2.0,4.0,FavorMixer*FavorMixer); 

	float NormalWeight = pow(max(dot(Normal.xyz, NormalCenter.xyz), 0.0), NormalBias); 

	float CombinedWeight = max(DepthWeight * NormalWeight * WeightTraversal -WeightIndirectLuminance,0.0);  


	return CombinedWeight; 

	return 0.0; 
	
	
}

float NewWeightSpecular(vec3 Normal, vec3 NormalCenter, 
				float Depth, float DepthCenter, 
				float Traversal, float TraversalCenter,
				float FrameCount, float Facing, 
				float Roughness, float CenterRoughness,
				float distanceSqr, 
				float Lum, float LumCenter) {
	float WeightFactor = mix(0.5,6.0,Facing); 
	
	float DepthWeight = 1.0 / pow(1.0 + WeightFactor * (abs(Depth - DepthCenter) / min(clamp(Depth, 0.1,DepthCenter),1.0)), 3.0); 
	float NormalWeight = pow(max(dot(Normal.xyz, NormalCenter.xyz), 0.0), 15.0); 
	float RoughnessDiffWeight = 1.0 / pow(1.0 + 50.0 * abs(Roughness - CenterRoughness), 2.0);
	float RoughnessSqrWeight = 1.0 / (1.0 + (1.0-min(Roughness, CenterRoughness)) * distanceSqr); 
	float TraversalFactor = sin(0.785398163 * Roughness) * max(Traversal, TraversalCenter); 

	TraversalFactor = clamp((TraversalFactor - max(Traversal, TraversalCenter)) * 100.0, 0.0, 1.0); 

	float TraversalWeight = 1.0 / (1.0 + 5.0 * abs(Traversal-TraversalCenter) / clamp(Traversal, 0.05, TraversalCenter)); 

	TraversalWeight = mix(1.0, TraversalWeight, TraversalFactor); 

	float LuminanceWeight = 0.2 * abs(Lum-LumCenter); 

	LuminanceWeight = mix( 0.0,LuminanceWeight, pow(1.0-min(Roughness,CenterRoughness),3.0)); 

	return max(DepthWeight * NormalWeight * RoughnessDiffWeight * RoughnessSqrWeight * TraversalWeight - LuminanceWeight,0.0); 

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

	vec4 BaseSHy = texelFetch(InputLighting, Pixel, 0); 
	vec4 BaseIndirectSpecular = texelFetch(InputSpecular, Pixel, 0); 
	float BaseSHCg = texelFetch(InputSHCg, Pixel, 0).x; 

	SHy = BaseSHy * TotalWeight; 
	IndirectSpecularCo = BaseIndirectSpecular * TotalWeight; //<- remember that the 

	#ifdef SPATIAL_FINAL 
	float SHCg = 0.0; 
	#endif 

	SHCg = BaseSHCg * TotalWeight; 

	float BaseRoughness = GetRoughness(BasePacked.xyz); 
	
	vec2 NewTexCoord = HighResPixel / vec2(textureSize(InputPacked,0).xy); 

	vec3 Direction = normalize(vec3(IncidentMatrix * vec4(NewTexCoord * 2.0 - 1.0, 1.0, 1.0)));
	
	if(!DoSpatial ) {
		SHy /= TotalWeight; 
		IndirectSpecularCo /= TotalWeightSpecular; 
		return; 
	}

	//construct variance estimate -> 

	vec4 BaseDetail = texelFetch(Detail, Pixel, 0); 

	float Var = abs(BaseDetail.x * BaseDetail.x - BaseDetail.z); 

	float phiIndirect = sqrt(max(0.0, 1e-10+Var)); 

	float Facing = abs(dot(BasePacked.xyz, Direction)); 


	for(int x = -2; x <= 2; x++) {
		for(int y = -2; y <= 2; y++) {
			
			if(x == 0 && y == 0)  
				continue; 
				
			ivec2 NewPixel = Pixel + ivec2(x,y) * StepSize;
			ivec2 NewPixelHighRes = HighResPixel + ivec2(x,y) * StepSize * 2; 

			vec2 Vec = ivec2(x,y) * StepSize; 

			float distSqr = dot(Vec,Vec) * 0.25; 

			vec4 CurrentDetail = texelFetch(Detail, NewPixel, 0); 

			vec4 CurrentPackedData = texelFetch(InputPacked, NewPixelHighRes, 0); 

			float CurrentRoughness = GetRoughness(CurrentPackedData.xyz); 

			float NormalWeight = pow(max(dot(CurrentPackedData.xyz, BasePacked.xyz), 0.0), 16.0); 

			float Weight = NormalWeight;  

			Weight /= pow(1.0 + 3.0 * abs(CurrentPackedData.w - BasePacked.w), 4.0); 


			vec4 SHySample = texelFetch(InputLighting, NewPixel, 0); 
			vec4 SpecularLightingSampleCo = texelFetch(InputSpecular, NewPixel, 0); 
			float SHCgSample = texelFetch(InputSHCg, NewPixel, 0).x; 

			float WeightSpecular = Weight * NormalWeight; 
			WeightSpecular /= pow(1.0 + 50.0 * abs(CurrentRoughness - BaseRoughness), 2.0); 

			Weight = WeightNew(CurrentDetail.x, BaseDetail.x, 
								CurrentDetail.z, BaseDetail.z, 
								CurrentDetail.y, BaseDetail.y, 
								CurrentPackedData.xyz, BasePacked.xyz, 
								CurrentPackedData.w, BasePacked.w, 
								CurrentDetail.w, BaseDetail.w, 
								phiIndirect, Frame, Facing);
								
			WeightSpecular = NewWeightSpecular(CurrentPackedData.xyz, BasePacked.xyz,
												CurrentPackedData.w, BasePacked.w, 
												CurrentDetail.y, BaseDetail.y, 
												Frame, Facing, 
												CurrentRoughness, BaseRoughness, 
												distSqr, 
												Luminance(SpecularLightingSampleCo.xyz), Luminance(BaseIndirectSpecular.xyz)); 

								

			float BaseKernelWeight = Kernel[3-abs(x)] * Kernel[3-abs(y)]; 
			float KernelWeight = BaseKernelWeight; 


			SHy += SHySample * Weight * KernelWeight; 
			SHCg += SHCgSample * Weight * KernelWeight; 
			IndirectSpecularCo.w += SpecularLightingSampleCo.w * Weight * KernelWeight; 
			IndirectSpecularCo.xyz += SpecularLightingSampleCo.xyz * WeightSpecular * KernelWeight; 
			
			TotalWeight += Weight * KernelWeight; 
			TotalWeightSpecular += WeightSpecular * KernelWeight; 



		}
	}
	SHy /= TotalWeight; 
	IndirectSpecularCo.w /= TotalWeight; 
	IndirectSpecularCo.xyz /= TotalWeightSpecular; 
	//IndirectSpecularCo.xyz = vec3(BaseRoughness); 
	//IndirectSpecularCo.xyz = BaseDetail.yyy; 
	SHCg /= TotalWeight; 

	#ifdef SPATIAL_FINAL 

	OutDirect = vec4(texelFetch(Direct, Pixel, 0).xyz, SHCg); 

	#endif 

}