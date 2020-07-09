#version 330
layout (location = 0) in vec3 Vert; 
layout (location = 1) in vec2 Texc;
out vec2 InTexCoord;  

void main(void) {
	InTexCoord = Texc; 
	gl_Position = vec4(Vert.xy,0.0,1.0); 
}