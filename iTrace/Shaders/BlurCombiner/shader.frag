#version 330 

layout(location = 0) out vec3 LightingGlow; 

in vec2 TexCoord; 

uniform float Lod;
uniform float Mix; 
uniform sampler2D RawInput; 
uniform sampler2D GlowInput;

uniform bool LensFlare; 

vec3 TextureDistorted(sampler2D Image, vec2 TexCoord, vec2 DistortionVec, vec3 DistortionPower) {

	return vec3(
		texture(Image, TexCoord + DistortionVec * DistortionPower.x).x,
		texture(Image, TexCoord + DistortionVec * DistortionPower.y).y,
		texture(Image, TexCoord + DistortionVec * DistortionPower.z).z
	); 

}

vec3 SuperiorClamp(vec3 Input) {
	
	float MaxFactor = max(Input.x,max(Input.y,Input.z)); 
	
	if(MaxFactor < 1.0) {
		return Input; 
	}
	else {
		return Input / MaxFactor; 
	}

}

void main() {
	LightingGlow = mix(texture(GlowInput, TexCoord).xyz, textureLod(RawInput, TexCoord, Lod).xyz, Mix); 

}