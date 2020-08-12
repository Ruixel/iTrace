#version 330

layout(location = 0) out vec4 TexCoordSideBlockType; 
layout(location = 1) out vec3 Normal; 
layout(location = 2) out vec3 Tangent; 

in vec4 Pos; 
in vec3 TexCoord;
in vec3 InNormal; 
in vec3 InTangent; 

uniform sampler1D TextureData; 
uniform sampler1D TextureExData; 
uniform sampler2DArray OpacityTextures; 

int GetTextureIdx(int Type, int Side) {
	return int(texelFetch(TextureData, Type*2+(Side/3), 0)[Side%3]*255.0+0.5); 
}

vec3 ToTBNSpace(int Side, vec3 WorldPosition) {
	if(Side == 0) {
		return -WorldPosition.xyz; 
	}
	else if(Side == 1) {
		return WorldPosition.xyz * vec3(1.0,-1.0,1.0); 
	}
	else if(Side == 2) {
		return WorldPosition.zyx * vec3(1.0,-1.0,1.0); 
	}
	else if(Side == 3) {
		return WorldPosition.zyx * vec3(-1.0,-1.0,1.0); 
	}
	else {
		return WorldPosition.xzy; 
	}
}

void main() {	

	//Find out if we should discard the current pixel 

	int BlockType = int(TexCoord.z+.5); 
	int BlockSide = int(Pos.w+.5); 

	int TextureIdx = int(texelFetch(TextureExData, GetTextureIdx(BlockType, BlockSide), 0).w * 255+.5); 

	//vec2 TC = ToTBNSpace(int(BlockSide),Pos.xyz).xy;

	float Opacity = texture(OpacityTextures, vec3(TexCoord.xy, TextureIdx-1)).x; 

	if(Opacity < 0.1) 
		discard; 

	TexCoordSideBlockType.xy = fract(TexCoord.xy); 
	TexCoordSideBlockType.zw = vec2(TexCoord.z,Pos.w); 
	Normal = InNormal; 
	Tangent = InTangent; 
}