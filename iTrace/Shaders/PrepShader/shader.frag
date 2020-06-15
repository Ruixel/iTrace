#version 330 

layout(location = 0) out vec3 LightingGlow; 
layout(location = 1) out vec3 LightingDof; 

in vec2 TexCoord; 

uniform sampler2D Normal; 
uniform sampler2D Albedo; 
uniform sampler2D SkyReigh; 
uniform sampler2D Lighting; 
uniform sampler2D Volumetrics; 

void main() {

	vec4 NormalFetch = texture(Normal, TexCoord); 

	float L = length(NormalFetch.xyz); 
	if(L > 0.25 && L < 1.75) {
		LightingGlow = texture(Albedo, TexCoord).xyz * NormalFetch.w; 
	}	
	else {
		//LightingGlow = vec3(0.0); 
		LightingGlow = texture(SkyReigh, TexCoord).xyz; 
	}
	LightingDof = texture(Lighting, TexCoord).xyz; 

	LightingGlow = mix(vec3(0.0), LightingGlow, texture(Volumetrics, TexCoord).a); 

}