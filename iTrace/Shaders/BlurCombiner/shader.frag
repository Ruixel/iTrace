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

	if(LensFlare) {
		
		vec2 GhostCoord = 1.0 - TexCoord; 

		vec2 GhostVec = (0.5 - GhostCoord) * 0.37; 

		for(int j = 1; j < 8; j++) {
			
			vec2 Offset = fract(GhostCoord + GhostVec * j); 
			
			float Weight = length(vec2(0.5) - Offset) / 0.707106781; 

			LightingGlow += SuperiorClamp(TextureDistorted(GlowInput, Offset, normalize(vec2(0.7,1.0)), vec3(.0,.005,.01))) * pow(1.0 - Weight, 50.0); 

		}



	}

}