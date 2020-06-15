#version 330

out vec4 TexCoordSideBlockType; 

in vec4 Pos; 
in vec3 TexCoord;

void main() {	
	TexCoordSideBlockType.xy = fract(TexCoord.xy); 
	TexCoordSideBlockType.zw = vec2(TexCoord.z,Pos.w); 
}