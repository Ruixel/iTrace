#version 330
layout(location=0) in vec4 RawPos; 
layout(location=1) in vec3 RawTexCoord;

uniform mat4 IdentityMatrix; 


out vec4 Pos; 
out vec3 TexCoord; 

void main(void) {	
	Pos = RawPos; 
	TexCoord = RawTexCoord; 
	gl_Position = IdentityMatrix * vec4(Pos.xyz,1.0); 
}