#version 330
layout (location = 0) in vec3 Vert; 
layout (location = 1) in vec2 Texc;
out vec2 TexCoord;  

uniform mat4 IdentityMatrix; 
out vec3 Pos; 


void main(void) {

	Pos = vec3(Vert.x * 500.0,58.9f, Vert.y * 500.0); 

	TexCoord = Texc; 
	gl_Position = IdentityMatrix * vec4(Pos,1.0); 
}