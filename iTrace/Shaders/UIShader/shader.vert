#version 330
layout (location = 0) in vec3 Vert; 
layout (location = 1) in vec2 Texc;
out vec2 TexCoord;  
uniform vec2 Position; 
uniform vec2 Scale; 

void main(void) {
	TexCoord = Texc; 

	TexCoord.y = 1.0 - TexCoord.y; 

	gl_Position = vec4(Vert.xy * Scale + Position,0.0,1.0); 
}