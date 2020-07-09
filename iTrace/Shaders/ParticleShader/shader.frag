#version 420

in vec2 TexCoord; 
layout(location = 0) out vec4 Normal;

uniform sampler2D RainNormalMap; 

void main() {
		
	vec4 NormalFetch = texture(RainNormalMap, TexCoord); 

	if(NormalFetch.a < 0.95)
		discard; 

	

	Normal.xyz = NormalFetch.xyz * 2.0 - 1.0; 

	vec2 Vector = abs(TexCoord * 2.0 - 1.0); 

	Normal.w = dot(Vector, Vector); 


}