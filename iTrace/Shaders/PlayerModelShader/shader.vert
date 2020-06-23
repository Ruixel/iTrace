#version 330
layout (location = 0) in vec3 Vert; 
out vec2 TexCoord;  
uniform mat4 IdentityMatrix; 
uniform vec3 CameraPosition; 

void main(void) {
	gl_Position = IdentityMatrix * vec4(CameraPosition + Vert.xyz * vec3(0.3,1.0,0.3) - vec3(0.0,1.0,0.0),1.0); 
}