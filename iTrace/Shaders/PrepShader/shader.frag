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
	LightingGlow = textureLod(Glow, TexCoord,2.0).xyz; 
	LightingGlow = mix(vec3(0.0), LightingGlow, texture(Volumetrics, TexCoord).a); 
}