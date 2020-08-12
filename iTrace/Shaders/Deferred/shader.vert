#version 330
layout(location=0) in vec4 RawPos; 
layout(location=1) in vec3 RawTexCoord;
layout(location=2) in vec3 Normal;
layout(location=3) in vec3 Tangent;

uniform mat4 IdentityMatrix; 


out vec4 Pos; 
out vec3 TexCoord; 
out vec3 InNormal; 
out vec3 InTangent; 

void main(void) {	
	Pos = RawPos; 
	TexCoord = RawTexCoord; 
	gl_Position = IdentityMatrix * vec4(Pos.xyz,1.0); 
	InNormal = Normal; 
	InTangent = Tangent; 
}