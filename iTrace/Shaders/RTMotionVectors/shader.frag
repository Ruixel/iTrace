#version 330

in vec2 TexCoord; 
layout(location = 0) out vec3 MotionVectors; //z component stores light-based culling 


uniform sampler2D WorldPosPrevious; 
uniform sampler2D Normal; 
uniform sampler2D NormalPrevious; 
uniform sampler2D PreviousWorldPos;
uniform sampler2D CloudDepth; 

uniform sampler2D CurrentLighting; 
uniform sampler2D PreviousLighting;


uniform vec3 CameraPosition; 
uniform bool NewFiltering; 

uniform mat4 MotionMatrix; 
uniform mat4 IncidentMatrix; 

uniform vec2 Resolution; 

vec3 FindBestPixel(vec2 Coord, vec3 CurrentWorldPos, vec3 PreviousWorldPos, vec3 CurrentNormal, vec3 PreviousNormal, vec3 CurrentSimpleLight, vec3 PreviousSimpleLight) {
	vec3 Vector = CurrentWorldPos - PreviousWorldPos; 
	if(true) {

	
	float zBased =  -1000.0 * max(abs(CurrentSimpleLight.x - PreviousSimpleLight.x)-0.02,0.0) - 1000.0 * max(abs(CurrentSimpleLight.y - PreviousSimpleLight.y)-0.02,0.0) - 1000.0 * max(abs(CurrentSimpleLight.z - PreviousSimpleLight.z)-0.02,0.0); 
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

	float L = length(CurrentNormal.xyz); 

	bool Clouds = false; 

	if(L < 0.5 || L > 1.5) {
		Clouds = true; 
		WorldSpace = CameraPosition + texture(CloudDepth, TexCoord).x * normalize(vec3(IncidentMatrix * vec4(TexCoord * 2.0 - 1.0, 1.0, 1.0)));
	}



	vec4 ClipSpace = MotionMatrix * vec4(WorldSpace, 1.0); 
	ClipSpace.xyz /= ClipSpace.w; 

	MotionVectors = vec3(-1.0); 

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


}