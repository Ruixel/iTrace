#version 330

in vec2 TexCoord; 
layout(location = 0) out vec2 MotionVectors;


uniform sampler2D WorldPosPrevious; 
uniform sampler2D Normal; 
uniform sampler2D NormalPrevious; 
uniform sampler2D PreviousWorldPos; 
uniform vec3 CameraPosition; 
uniform bool NewFiltering; 

uniform mat4 MotionMatrix; 
uniform mat4 IncidentMatrix; 

uniform vec2 Resolution; 

vec2 FindBestPixel(vec2 Coord, vec3 CurrentWorldPos, vec3 PreviousWorldPos, vec3 CurrentNormal, vec3 PreviousNormal) {
	vec3 Vector = CurrentWorldPos - PreviousWorldPos; 
	if(true) {

	
	float DistanceSquared = -int(dot(Vector, Vector) * 100.0) + min(dot(CurrentNormal, PreviousNormal)*10.0,0.0); 

	return Coord + vec2(1.0) * DistanceSquared; 

	}
	else {
		float DistanceSquared = -int(dot(Vector, Vector) * 5.0);

		return Coord + vec2(1.0) * DistanceSquared; 
	}

}


void main() {
	
	vec3 WorldSpace = texture(WorldPosPrevious, TexCoord).xyz; 
	vec4 CurrentNormal = texture(Normal, TexCoord); 

	float L = length(CurrentNormal.xyz); 

	if(L < 0.75 || L > 1.25) {
		//WorldSpace = CameraPosition + 100.0 * normalize(vec3(IncidentMatrix * vec4(TexCoord * 2.0 - 1.0, 1.0, 1.0)));
	}



	vec4 ClipSpace = MotionMatrix * vec4(WorldSpace, 1.0); 
	ClipSpace.xyz /= ClipSpace.w; 

	MotionVectors = vec2(-1.0); 

	if(abs(ClipSpace.x) <= 1.0 && abs(ClipSpace.y) <= 1.0) {
		ClipSpace.xy = ClipSpace.xy * 0.5 + 0.5; 

		
		vec4 PreviousWorldPos = texture(PreviousWorldPos, ClipSpace.xy); 
		vec4 PreviousNormal = texture(Normal, ClipSpace.xy); 

		vec2 Best = FindBestPixel(ClipSpace.xy, WorldSpace, PreviousWorldPos.xyz, CurrentNormal.xyz, PreviousNormal.xyz); 

		if(Best.x >= 0.0 && Best.y >= 0.0) 
			MotionVectors = Best - TexCoord; 



	}


}