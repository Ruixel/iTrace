#version 330 

layout(location = 0) out vec4 GlowDirection1; 
layout(location = 1) out vec4 GlowDirection2; 
layout(location = 2) out vec4 GlowDirection3; 
layout(location = 3) out vec4 GlowDirection4; 

in vec2 TexCoord; 

uniform float Lod;
uniform float Mix; 
uniform sampler2D RawInput; 
uniform sampler2D GlowInput1;
uniform sampler2D GlowInput2;
uniform sampler2D GlowInput3;
uniform sampler2D GlowInput4;

uniform bool First; 


void main() {

	vec3 InputMix = textureLod(RawInput, TexCoord, Lod).xyz;
	
	vec4 Input1 = texture(GlowInput1, TexCoord); 
	vec4 Input2 = texture(GlowInput2, TexCoord); 
	vec4 Input3 = texture(GlowInput3, TexCoord); 
	vec4 Input4 = texture(GlowInput4, TexCoord); 

	if(First) {
		Input1 = vec4(1.0); 
		Input2 = vec4(1.0); 
		Input3 = vec4(1.0); 
		Input4 = vec4(1.0); 

	}

	GlowDirection1.xyz = mix(Input1.xyz, InputMix.xyz * Input1.w, Mix); 
	GlowDirection2.xyz = mix(Input2.xyz, InputMix.xyz * Input2.w, Mix); 
	GlowDirection3.xyz = mix(Input3.xyz, InputMix.xyz * Input3.w, Mix); 
	GlowDirection4.xyz = mix(Input4.xyz, InputMix.xyz * Input4.w, Mix); 

	GlowDirection1.w = Input1.w; 
	GlowDirection2.w = Input2.w; 
	GlowDirection3.w = Input3.w; 
	GlowDirection4.w = Input4.w; 

}