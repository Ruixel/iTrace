#version 330

out vec4 Color; 

in vec4 Pos; 
in vec3 TexCoord;

uniform sampler1D TextureData; 
uniform sampler2D Depth; 
uniform sampler2DArray DiffuseTextures; //<- really? 

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

	int BlockType = int(TexCoord.z+.5); 
	int BlockSide = int(Pos.w+.5); 

	vec2 TC = ToTBNSpace(int(BlockSide),Pos.xyz).xy;
	
	//Check opaque depth texture, compare with current depth. Discard if behind opaque depth -> 

	float OpaqueDepth = texelFetch(Depth, ivec2(gl_FragCoord), 0).x ; 

	if(OpaqueDepth < gl_FragDepth) 
		discard; 

	Color = texture(DiffuseTextures, vec3(TC, BlockType)); 

}