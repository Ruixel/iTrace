#version 330 

layout(location = 0) out float C; 

uniform sampler2D Image; 
in vec2 TexCoord;  

void main() {

	C = 0.0; 
	ivec2 Coord = ivec2(gl_FragCoord.xy); 
	for(int i = 0; i < 1000; i++) {
		C += texelFetch(Image,Coord + ivec2(i,0),0).x; 
	}
	
}