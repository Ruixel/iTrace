#version 330

out vec4 Color; 

in vec4 Pos; 
in vec3 TexCoord;

uniform sampler1D TextureData; 
uniform sampler2D Depth; 
uniform sampler2D PrimaryRefractiveDepth; 
uniform sampler2DArray DiffuseTextures; //<- really? 

const vec3 BlockNormals[6] = vec3[](
				vec3(0.0, 0.0, 1.0),
				vec3(0.0, 0.0, -1.0),
				vec3(1.0, 0.0, 0.0),
				vec3(-1.0, 0.0, 0.0),
				vec3(0.0, 1.0, 0.0),
				vec3(0.0, -1.0, 0.0)
);


uniform bool IsPrimary; 
uniform bool IsShadow; 
uniform float Bias; 

uniform float zNear; 
uniform float zFar; 

uniform vec3 CameraPos; 

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

float LinearDepthOrtho(float z) {
	return z * (zFar-zNear) + zNear; 
}

float AnalyticalTraversal(vec3 Direction, vec3 Origin) {


	vec3 BlockPosition = floor(Origin); //<- assumes one block is one unit! 
	vec3 BlockPositionCeil = BlockPosition + 1; 



	//the 2 y planes 

	float Y = (BlockPosition.y-Origin.y) / Direction.y; 

	if(Y < 0.0) 
		Y = (BlockPositionCeil.y-Origin.y ) / Direction.y; 

	

	//the 2 x planes 

	float X = (BlockPosition.x-Origin.x) / Direction.x; 
	if(X < 0.0) 
		X = ( BlockPositionCeil.x-Origin.x) / Direction.x; 
	//the 2 z planes 

	float Z = ( BlockPosition.z-Origin.z) / Direction.z; 
	if(Z < 0.0) 
		Z = (BlockPositionCeil.z-Origin.z) / Direction.z; 

	if(Y < 0.0) 
		Y = 10000.0; 

	if(X < 0.0) 
		X = 10000.0; 

	if(Z < 0.0) 
		Z = 10000.0; 

	float t = min(X,min(Y,Z)); 

	return t; 

}

void main() {	

	int BlockType = int(TexCoord.z+.5); 
	int BlockSide = int(Pos.w+.5); 

	vec2 TC = ToTBNSpace(int(BlockSide),Pos.xyz).xy;
	
	//Check opaque depth texture, compare with current depth. Discard if behind opaque depth -> 

	float OpaqueDepth = texelFetch(Depth, ivec2(gl_FragCoord), 0).x ; 

	if(OpaqueDepth < gl_FragCoord.z) {
		discard; 
	}
	if(!IsPrimary) {
		
		float RefractiveDepth = texelFetch(PrimaryRefractiveDepth, ivec2(gl_FragCoord), 0).x; 
		
		if(RefractiveDepth >= (gl_FragCoord.z))
			discard;  

	}

	vec3 Incident = normalize(vec3(CameraPos - Pos.xyz)); 
	vec3 Normal = BlockNormals[BlockSide]; 

	vec3 TextureLookUp = vec3(0.0); 

	if(!IsShadow) {

		//inter-block refraction 

		TextureLookUp = texture(DiffuseTextures, vec3(TC, GetTextureIdx(BlockType, BlockSide))).xyz; 
		TextureLookUp = pow(TextureLookUp, vec3(2.2)); 
	
	}
	else {
		float Distance = LinearDepthOrtho(OpaqueDepth) - LinearDepthOrtho(gl_FragCoord.z); 

		float Lod = clamp(Distance-1.0,0.0,4.0); 

		


		TextureLookUp = textureLod(DiffuseTextures, vec3(TC, BlockType-1), Lod).xyz; 


	}

	float Traversal = AnalyticalTraversal(-Incident,Pos.xyz - Normal*0.01); 

	Color.xyz = vec3(Traversal); 

	Color.xyz = pow(TextureLookUp / 2.0, vec3(Traversal*2.0)) * 2.0; 
	Color.xyz = clamp(Color.xyz, vec3(0.0), vec3(1.0));  

	if(dot(Incident, Normal) < 0.0) 
		Color = vec4(1.0); 
				Color.a = 1.0; 

}