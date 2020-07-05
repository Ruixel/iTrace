#version 330 

layout(location = 0) out float C; 

uniform sampler2D Image; 
in vec2 TexCoord;  

void main() {

	float C = 0.0; 

	for(int i = 0; i < 1000; i++) 
		C += texture(Image,TexCoord).x; 
	C /= 1000.0; 
}