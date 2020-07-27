#version 330

out vec4 Color; 
out vec3 Normal; //<- used for analytical interrefraction later 

in vec4 Pos; 
in vec3 TexCoord;

uniform sampler1D TextureData; 
uniform sampler2D Depth; 
uniform sampler2DArray DiffuseTextures; 
uniform sampler2DArray NormalTextures; //~^-^~

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

const vec3 BlockNormals[6] = vec3[](
				vec3(0.0, 0.0, 1.0),
				vec3(0.0, 0.0, -1.0),
				vec3(1.0, 0.0, 0.0),
				vec3(-1.0, 0.0, 0.0),
				vec3(0.0, 1.0, 0.0),
				vec3(0.0, -1.0, 0.0)
);


const vec3 BlockTangents[6] = vec3[](
	vec3(-1.0,0.0,0.0),
	vec3(1.0,0.0,0.0),
	vec3(0.0,0.0,1.0),
	vec3(0.0,0.0,-1.0),
	vec3(1.0,0.0,0.0),
	vec3(1.0,0.0,0.0)
); 

const vec3 BlockBiTangents[6] = vec3[](
	vec3(0.0,-1.0,0.0),
	vec3(0.0,-1.0,0.0),
	vec3(0.0,-1.0,0.0),
	vec3(0.0,-1.0,0.0),
	vec3(0.0,0.0,1.0),
	vec3(0.0,0.0,-1.0)
); 

void main() {	

	int BlockType = int(TexCoord.z+.5); 
	int BlockSide = int(Pos.w+.5); 

	vec2 TC = ToTBNSpace(int(BlockSide),Pos.xyz).xy;
	
	//Check opaque depth texture, compare with current depth. Discard if behind opaque depth -> 

	float OpaqueDepth = texelFetch(Depth, ivec2(gl_FragCoord), 0).x ; 

	if(OpaqueDepth < gl_FragCoord.z) 
		discard; 

	int TextureIdx = GetTextureIdx(BlockType, BlockSide); 

	Color = texture(DiffuseTextures, vec3(TC, TextureIdx)); 
	//-> now, do normal mapping (yeah yeah its not fast) 

	Normal.xyz = BlockNormals[BlockSide]; 

	vec3 Tangent = BlockTangents[BlockSide]; 

	vec3 Bitangent = BlockBiTangents[BlockSide]; 


	mat3 TBN = mat3(Tangent, Bitangent, Normal); 

	Normal.xyz = normalize(mix(Normal.xyz,normalize(TBN * (texture(NormalTextures, vec3(TC.xy, TextureIdx)).xyz * 2.0 - 1.0)),0.3)); 

}