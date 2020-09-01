#version 330

in vec2 TexCoord; 
layout(location = 0) out vec3 MotionVectors; //z component stores light-based culling 
layout(location = 1) out vec3 MotionVectorsSpecular; //<- same stucture as previous one 
layout(location = 2) out vec2 MotionVectorsClouds; //<- same stucture as previous one 

uniform sampler2D WorldPosPrevious; 
uniform sampler2D Normal; 
uniform sampler2D NormalPrevious; 
uniform sampler2D PreviousWorldPos;
uniform sampler2D CloudDepth; 

uniform sampler2D CurrentLighting; 
uniform sampler2D PreviousLighting;

uniform sampler2D WorldPosWaterPrevious;
uniform sampler2D PreviousWorldPosWater; 
uniform sampler2D NormalWater; 
uniform sampler2D PreviousNormalWater; 
uniform sampler2D Depth; 
uniform sampler2D WaterDepth; 


uniform vec3 CameraPosition; 
uniform bool NewFiltering; 

uniform mat4 MotionMatrix; 
uniform mat4 IncidentMatrix; 

uniform vec2 Resolution; 

vec3 FindBestPixel(vec2 Coord, vec3 CurrentWorldPos, vec3 PreviousWorldPos, vec3 CurrentNormal, vec3 PreviousNormal, vec3 CurrentSimpleLight, vec3 PreviousSimpleLight) {
	vec3 Vector = CurrentWorldPos - PreviousWorldPos; 
	if(true) {

	
	float zBased =  -4000.0 * max(abs(CurrentSimpleLight.x - PreviousSimpleLight.x)-0.02,0.0) - 1000.0 * max(abs(CurrentSimpleLight.y - PreviousSimpleLight.y)-0.02,0.0) - 1000.0 * max(abs(CurrentSimpleLight.z - PreviousSimpleLight.z)-0.02,0.0); 
	float DistanceSquared = -int(dot(Vector, Vector) * 100.0) + min(dot(CurrentNormal, PreviousNormal)*10.0,0.0); 

	return vec3(Coord + vec2(1.0) * DistanceSquared,zBased); 

	}
	else {
		float DistanceSquared = -int(dot(Vector, Vector) * 5.0);

		return vec3(Coord + vec2(1.0) * DistanceSquared,0.0); 
	}



}


void main() {
	
	vec3 WorldSpace = texture(WorldPosPrevious, TexCoord).xyz; 
	vec4 CurrentNormal = texture(Normal, TexCoord); 
	vec3 CurrentLighting = texture(CurrentLighting, TexCoord).xyz; 

	float BaseDepth = texelFetch(Depth, ivec2(gl_FragCoord)*2, 0).x; 
	float WaterDepth = texelFetch(WaterDepth, ivec2(gl_FragCoord)*2, 0).x; 



	float L = length(CurrentNormal.xyz); 

	bool Clouds = false; 

	vec3 CloudsPosition = CameraPosition + 200.0 * normalize(vec3(IncidentMatrix * vec4(TexCoord * 2.0 - 1.0, 1.0, 1.0)));

	if(L < 0.5 || L > 1.5) {
		Clouds = true; 

		WorldSpace = CloudsPosition;
	}



	vec4 ClipSpace = MotionMatrix * vec4(WorldSpace, 1.0); 
	ClipSpace.xyz /= ClipSpace.w; 

	MotionVectors = vec3(-1.0); 
	MotionVectorsClouds = vec2(-1.0); 

	if(abs(ClipSpace.x) <= 1.0 && abs(ClipSpace.y) <= 1.0) {
		ClipSpace.xy = ClipSpace.xy * 0.5 + 0.5; 

		
		vec4 PreviousWorldPos = texture(PreviousWorldPos, ClipSpace.xy); 

		if(Clouds) 
			PreviousWorldPos.xyz = WorldSpace; 

		vec4 PreviousNormal = texture(Normal, ClipSpace.xy); 
		vec4 PreviousLighting = texture(PreviousLighting, ClipSpace.xy); 

		vec3 Best = FindBestPixel(ClipSpace.xy, WorldSpace, PreviousWorldPos.xyz, CurrentNormal.xyz, PreviousNormal.xyz, CurrentLighting, PreviousLighting.xyz); 

		if(Best.x >= 0.0 && Best.y >= 0.0) 
			MotionVectors.xy = Best.xy - TexCoord; 
		MotionVectors.z = Best.z; 


	}
	MotionVectorsSpecular = MotionVectors; 

	if(WaterDepth < BaseDepth) {
		
		vec3 WorldPosSample = texture(WorldPosWaterPrevious, TexCoord).xyz; 

		
		vec4 ClipSpaceWater = MotionMatrix * vec4(WorldPosSample, 1.0); 
		ClipSpaceWater.xyz /= ClipSpaceWater.w; 

		if(abs(ClipSpaceWater.x) <= 1.0 && abs(ClipSpaceWater.y) <= 1.0) {
			
			MotionVectorsSpecular.xy = ClipSpaceWater.xy * 0.5 + 0.5; //<- for now, start with this 
			MotionVectorsSpecular.xy -= TexCoord; 

		}
		else {
			//weird, but okay... 
			//MotionVectorsSpecular = vec3(-1.0); 
		}


	}

	vec4 ClipSpaceClouds = MotionMatrix * vec4(CloudsPosition, 1.0); 
	ClipSpaceClouds.xyz /= ClipSpaceClouds.w; 

	if(abs(ClipSpaceClouds.x) < 1.0 && abs(ClipSpaceClouds.y) < 1.0) {
		
		ClipSpaceClouds.xy = ClipSpaceClouds.xy * 0.5 + 0.5; 
		
		MotionVectorsClouds = ClipSpaceClouds.xy - TexCoord; 
	}



}