#version 330

layout(location = 0) out vec4 TexCoordSideBlockType; 
layout(location = 1) out vec3 Normal; 
layout(location = 2) out vec3 Tangent; 

in vec4 Pos; 
in vec3 TexCoord;
in vec3 InNormal; 
in vec3 InTangent; 

void main() {	
	TexCoordSideBlockType.xy = fract(TexCoord.xy); 
	TexCoordSideBlockType.zw = vec2(TexCoord.z,Pos.w); 
	Normal = InNormal; 
	Tangent = InTangent; 
}