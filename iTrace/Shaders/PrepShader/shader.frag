#version 330 

layout(location = 0) out vec3 LightingGlow; 

in vec2 TexCoord; 

uniform sampler2D Normal; 
uniform sampler2D Albedo; 
uniform sampler2D SkyReigh; 
uniform sampler2D Volumetrics; 
uniform sampler2D Glow;
uniform sampler2D Clouds; 


void main() {

	vec4 NormalFetch = texture(Normal, TexCoord); 

	float L = length(NormalFetch.xyz); 
	if(L > 0.25 && L < 1.75) {
		//LightingGlow = texture(Glow, TexCoord).xyz; 
	}	
	else {
		//LightingGlow = vec3(0.0); 
		//LightingGlow = mix(vec3(0.0), texture(SkyReigh, TexCoord).xyz, pow(texture(Clouds,TexCoord).w,1.0)); 
	}

	LightingGlow = texture(Glow, TexCoord).xyz; 

	LightingGlow = mix(vec3(0.0), LightingGlow, texture(Volumetrics, TexCoord).a); 
}