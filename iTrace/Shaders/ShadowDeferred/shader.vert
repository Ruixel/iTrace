#version 330
layout (location = 0) in vec3 Vert; 
layout (location = 1) in vec2 Texc;
out vec2 TexCoord;  

uniform mat4 IdentityMatrix; 

void main(void) {
	TexCoord = Texc; 
	gl_Position = IdentityMatrix * vec4(Vert,1.0); 
}