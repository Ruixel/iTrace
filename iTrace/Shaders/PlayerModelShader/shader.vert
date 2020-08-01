#version 330
layout (location = 0) in vec3 Vert; 
out vec2 TexCoord;  
uniform mat4 IdentityMatrix; 
uniform vec3 CameraPosition; 

void main(void) {
	gl_Position = IdentityMatrix * vec4(Vert.xyz,1.0); 
}