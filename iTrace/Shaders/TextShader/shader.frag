#version 330
in vec2 TexCoord; 
out vec3 Result; 

uniform sampler2D Font; 
uniform vec3 Color; 


void main() {
	float Sample = texture(Font, TexCoord).x; 
	if(Sample < 0.2) 
		discard; 
	Result = Color; 
}