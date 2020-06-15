#version 420

in vec2 TexCoord; 

layout(location = 0) out vec4 IndirectDiffuse;
layout(location = 1) out vec4 Volumetrics;

uniform sampler2D UpscaledDiffuse; 
uniform sampler2D PreviousDiffuse; 
uniform sampler2D MotionVectors; 
uniform sampler2D FrameCount; 

uniform sampler2D UpscaledVolumetrics; 
uniform sampler2D PreviousVolumetrics; 
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

void main() {

	

	vec2 MotionVectors = texture(MotionVectors, TexCoord).xy; 
	
	float FrameCount = max(texture(FrameCount, TexCoord).x - 1.0,0.0); 
	


	float NewMaxTemporal = FrameCount < 4.0 ? 0.75 : (NewFiltering ? 0.99 : 0.95); 
	float MaxTemporalShadow = FrameCount < 6.0 ? 0.0 : 0.95; 


	float MixFactor = min(FrameCount / (FrameCount+1.0),NewMaxTemporal);
	float MixFactorVolume = min(FrameCount / (FrameCount+1.0),min(MaxTemporalShadow, 0.95));

	if(!DoTemporal) {
		MixFactor = 0.0; 
		MixFactorVolume = 0.0; 
	}

	vec4 CurrentLightingSample = texture(UpscaledDiffuse, TexCoord); 
	vec4 CurrentVolumetricSample = texture(UpscaledVolumetrics, TexCoord); 


	IndirectDiffuse = mix(CurrentLightingSample, GetClamped(UpscaledDiffuse,PreviousDiffuse, TexCoord + MotionVectors,0.05), MixFactor); 
	Volumetrics = mix(CurrentVolumetricSample, GetClamped(UpscaledVolumetrics,PreviousVolumetrics, TexCoord + MotionVectors,0.05), MixFactorVolume); 
//	Volumetrics = CurrentVolumetricSample; 
	//IndirectDiffuse.xyz = int(gl_FragCoord.x) % 2 == 0 ? vec3(1.0) : vec3(0.0); 

	//IndirectDiffuse.xyz = vec3(FrameCount/30.0); 
	//IndirectDiffuse.xy = max(-MotionVectors,vec2(0.0)); 
	//IndirectDiffuse.z = 0.0; 

	IndirectDiffuse.xyz = max(IndirectDiffuse.xyz, vec3(0.0)); 

	


}