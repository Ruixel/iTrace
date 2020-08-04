#version 420

in vec2 TexCoord; 

layout(location = 0) out vec4 IndirectDiffuse;
layout(location = 1) out vec4 Volumetrics;
layout(location = 2) out vec4 IndirectSpecular;
layout(location = 3) out vec4 Clouds;
layout(location = 4) out vec4 Direct; 

uniform sampler2D UpscaledDiffuse; 
uniform sampler2D PreviousDiffuse; 
uniform sampler2D MotionVectors; 
uniform sampler2D FrameCount; 

uniform sampler2D UpscaledVolumetrics; 
uniform sampler2D PreviousVolumetrics; 

uniform sampler2D UpscaledSpecular; 
uniform sampler2D PreviousSpecular; 

uniform sampler2D UpscaledClouds; 
uniform sampler2D PreviousClouds; 

uniform sampler2D UpscaledDirect; 
uniform sampler2D PreviousDirect; 

uniform bool DoTemporal; 
uniform bool NewFiltering; 

//enable for higher performance GPUs! 


#ifdef TEMPORAL_HIGH_QUALITY 

float MaxTemporal = 0.98; //we can use a LOT more frames if we have clamping 

vec4 GetClamped(sampler2D Current, sampler2D Previous, vec2 PreviousTexCoord, float BiasFactor) {
	

	ivec2 Coord = ivec2(gl_FragCoord.xy); 

	vec4 Min = vec4(10000.0); 
	vec4 Max = vec4(-10000.0); 

	for(int x = -2; x <= 2; x++) {
		for(int y = -2; y <= 2; y++) {

			vec4 Fetch = texelFetch(Current,Coord + ivec2(x,y), 0); 

			Min = min(Min, Fetch); 
			Max = max(Max, Fetch); 
		}
	}

	Min = Min - BiasFactor; 
	Max = Max + BiasFactor; 
	
	return clamp(texture(Previous, PreviousTexCoord), Min, Max); 

}


#else

float MaxTemporal = 0.95; //trying to fight ghosting by not using too many frames 

//no clamping, for performance reasons 

vec4 GetClamped(sampler2D Current, sampler2D Previous, vec2 PreviousTexCoord, float BiasFactor) {
	return texture(Previous, PreviousTexCoord); 
}
 

#endif 

vec3 ClampDirect(sampler2D DirectSampler, vec3 Previous) {

	ivec2 Coord = ivec2(gl_FragCoord); 

	vec3 Min = vec3(10000.0); 
	vec3 Max = vec3(-10000.0); 

	for(int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {

			vec3 Fetch = texelFetch(DirectSampler,Coord + ivec2(x,y), 0).xyz; 

			Min = min(Min, Fetch); 
			Max = max(Max, Fetch); 
		}
	}

	return clamp(Previous, Min, Max); 

}




void main() {

	
	IndirectSpecular.xyz = vec3(1.0); 
	IndirectDiffuse.xyz = vec3(1.0); 
	
	vec2 MotionVectors = texture(MotionVectors, TexCoord).xy; 
	
	float FrameCount = max(texture(FrameCount, TexCoord + MotionVectors).x - 1.0,0.0); 
	
	if(MotionVectors.x < -.5 || MotionVectors.y < -.5) {
		FrameCount = 0; 
	}


	float NewMaxTemporal = FrameCount < 4.0 ? 0.0 : FrameCount < 12.0 ? 0.9 : (NewFiltering ? 0.99 : 0.95); 
	float MaxTemporalShadow = FrameCount < 6.0 ? 0.0 : 0.95; 
	float MaxTemporalClouds = FrameCount < 4.0 ? 0.0 : 0.9677; 

	float MixFactor = min(FrameCount / (FrameCount+1.0),NewMaxTemporal);
	float MixFactorVolume = min(FrameCount / (FrameCount+1.0),min(NewMaxTemporal, 0.0));
	float MixFactorClouds = min(FrameCount / (FrameCount+1.0),MaxTemporalClouds);

	if(!DoTemporal) {
		MixFactor = 0.0; 
		MixFactorVolume = 0.0; 
		MixFactorClouds = 0.0; 
	}

	float SpecularTrustFactor = 0.9; 
	
	//We know that the more we've moved, the less we can trust the specular motion vectors! 

	SpecularTrustFactor = mix(SpecularTrustFactor, 0.75, min(length(MotionVectors)*10000.0,1.0)); 

	vec4 CurrentLightingSample = texture(UpscaledDiffuse, TexCoord); 
	vec4 CurrentVolumetricSample = texture(UpscaledVolumetrics, TexCoord); 
	vec4 CurrentSpecularSample = texture(UpscaledSpecular, TexCoord); 
	vec4 CurrentClouds = texture(UpscaledClouds, TexCoord); 
	vec4 CurrentDirect = texture(UpscaledDirect, TexCoord); 

	vec4 PreviousDiffuse = GetClamped(UpscaledDiffuse,PreviousDiffuse, TexCoord + MotionVectors,0.05); 
	vec4 PreviousVolumetric = GetClamped(UpscaledVolumetrics,PreviousVolumetrics, TexCoord + MotionVectors,0.05);
	vec4 PreviousSpecular = GetClamped(UpscaledSpecular, PreviousSpecular, TexCoord + MotionVectors, 0.05); 
	vec4 PreviousClouds = texture(PreviousClouds, TexCoord + MotionVectors); 
	vec4 PreviousDirect = texture(PreviousDirect, TexCoord + MotionVectors); 

	IndirectDiffuse = mix(CurrentLightingSample, PreviousDiffuse, MixFactor); 
	Volumetrics = mix(CurrentVolumetricSample, PreviousVolumetric, min(MixFactorClouds,0.9));
	IndirectSpecular = mix(CurrentSpecularSample, PreviousSpecular, min(MixFactor, SpecularTrustFactor)); 
	Clouds = mix(CurrentClouds, PreviousClouds,MixFactorClouds);  
	Direct.xyz = CurrentDirect.xyz; 
	Direct.xyz = mix(CurrentDirect.xyz, ClampDirect(UpscaledDirect,PreviousDirect.xyz), min(MixFactor,0.95)); 
	Direct.w = mix(CurrentDirect.w, PreviousDirect.w, MixFactor); 
	IndirectSpecular.w = mix(CurrentSpecularSample.w, PreviousSpecular.w, MixFactor); 


//	Volumetrics = CurrentVolumetricSample; 
	//IndirectDiffuse.xyz = int(gl_FragCoord.x) % 2 == 0 ? vec3(1.0) : vec3(0.0); 

	//IndirectDiffuse.xyz = vec3(FrameCount/30.0); 
	//IndirectDiffuse.xy = max(-MotionVectors,vec2(0.0)); 
	//IndirectDiffuse.z = 0.0; 


	IndirectSpecular.xyz = max(IndirectSpecular.xyz, vec3(0.0));
	Clouds.xyz = clamp(Clouds.xyz, vec3(0.0),vec3(100.0)); 
	
}