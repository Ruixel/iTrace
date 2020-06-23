#version 400

layout(location = 0) out vec4 Normal; 
layout(location = 1) out vec3 WorldPos; 
layout(location = 2) out vec4 Albedo; 
layout(location = 3) out vec4 HighFreqNormal; 


in vec2 TexCoord;

uniform mat4 InverseProj; 
uniform mat4 InverseView; 
uniform mat4 IncidentMatrix; 
uniform vec3 CameraPosition; 
uniform float Time; 
uniform vec3 SunDirection; 
uniform int ParallaxDirections; 
uniform int ParallaxResolution; 

uniform bool DoParallax; 

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
uniform sampler2DArray DiffuseTextures; 
uniform sampler2DArray EmissiveTextures; 
uniform sampler2DArray DisplacementTextures; 
uniform sampler2DArray NormalTextures; 
uniform sampler2DArray RoughnessTextures; 
uniform sampler2DArray MetalnessTextures; 


uniform sampler1D TextureData; 
uniform sampler1D TextureExData; 
uniform sampler1D BlockData; 

uniform sampler3D LightData; 

uniform sampler2D InTexCoord; 
uniform sampler2D InDepth; 

uniform sampler2D ParallaxMap; 

uniform vec3 SkyColor; 

int GetTextureIdx(int Type, int Side) {
	return int(texelFetch(TextureData, Type*2+(Side/3), 0)[Side%3]*255.0+0.5); 
}

vec3 GetWorldPosition(float z) {

	vec4 Clip = vec4(TexCoord * 2.0 - 1.0, z, 1.0); 
	vec4 ViewSpace = InverseProj * Clip; 
	ViewSpace /= ViewSpace.w; 

	vec4 World = (InverseView * ViewSpace); 
	return World.xyz; 
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


float SampleParallaxMap(vec3 Point, uint Texture, float lod) {
	
	Point = fract(Point); 
	
	int z = int(Point.y * ParallaxDirections); 
	
	int NextZ = (z+1) % ParallaxDirections; 
	
	float Interp = fract(Point.y*ParallaxDirections); 
	

	vec2 PixelSize = 1.0 / vec2((ParallaxResolution+2)*ParallaxDirections, ParallaxResolution); 

	float AddonSize = 1.0 / ParallaxDirections; 

	float Multiply = float(ParallaxResolution) / float((ParallaxResolution+2)*ParallaxDirections); 
	Point.z = 1.0 - Point.z; 

	vec2 PixelPoint = Point.xz; 
	PixelPoint.x = (PixelPoint.x + PixelSize.x) * Multiply + AddonSize * z; 

	vec2 PixelPoint2 = Point.xz; 
	PixelPoint2.x = (PixelPoint2.x + PixelSize.x) * Multiply + AddonSize * NextZ; 
	return mix(textureLod(DisplacementTextures, vec3(PixelPoint, Texture),lod).x, textureLod(DisplacementTextures, vec3(PixelPoint2,Texture),lod).x, Interp); 
}

float GetTraversal(vec2 TC, vec3 Direction, uint Side, mat3 TBN, uint Type, inout vec3 DirectionProjected, float Lod) {
	DirectionProjected = ToTBNSpace(int(Side),Direction); 

	vec2 DirectionXZ = normalize(DirectionProjected.xy); 
	
	float Angle = atan(DirectionXZ.x, DirectionXZ.y); 

	return SampleParallaxMap(vec3(TC.x, (Angle-1.57079633) / 6.28318531,TC.y), Type, Lod) * 0.5; //temporary! 
}

void main() {	


	float Depth = texelFetch(InDepth, ivec2(gl_FragCoord),0).x; 

	vec3 Incident = normalize(vec3(IncidentMatrix * vec4(TexCoord * 2.0 - 1.0, 1.0, 1.0)));
	
	if(Depth == 1.0) {
		Normal.xyz = vec3(0.0); 

		WorldPos = CameraPosition + 100.0 * Incident;
		


		return; 
	}
	WorldPos = GetWorldPosition(Depth * 2.0 - 1.0); 
	vec4 RawTCData = texelFetch(InTexCoord, ivec2(gl_FragCoord),0); 


	

	uint BlockType = uint(RawTCData.z+.5); 
	uint BlockSide = uint(RawTCData.w+.5); 

	Normal.xyz = BlockNormals[BlockSide]; 
	Normal.w = 0.0; 
	vec3 Tangent = BlockTangents[BlockSide]; 

	vec3 Bitangent = BlockBiTangents[BlockSide]; 

	mat3 TBN = mat3(Tangent, Bitangent, Normal); 

	int TextureIdx = GetTextureIdx(int(BlockType), int(BlockSide));
	
	ivec3 TextureExData = ivec3(texelFetch(TextureExData, TextureIdx,0).xyz * 255); 

	vec2 TC = RawTCData.xy; 

	//if(BlockSide == 4u) {
		TC = ToTBNSpace(int(BlockSide),WorldPos).xy;
		//TC.y = 1.0 - TC.y; 
	//} 
	
	if(TextureExData.x != 0 && DoParallax)  {

		float lod = textureQueryLod(DiffuseTextures, TC.xy).x; 
		lod = 0.0; 
		if(BlockType == 31u) {
			TC += vec2(Time*0.1,Time*0.05); 
			TC = fract(TC); 
		}

		vec3 IncidentProjected; 

		float Traversal = GetTraversal(TC.xy, Incident, BlockSide, TBN, uint(TextureExData.x)-1u,IncidentProjected,lod) ;  
		
		if(BlockType == 31u) {
	
			vec3 IncidentProjected2; 

			vec2 TC2 = TC -  vec2(Time*0.1,Time*0.05); 

			TC2 = TC2 - vec2((Time+0.1)*0.05, (Time-0.7)*0.1); 

			TC2 = fract(TC2); 



			float Traversal2 = GetTraversal(TC2, Incident, BlockSide, TBN, uint(TextureExData.x)-1u,IncidentProjected2,lod) ;  

			if(Traversal2 < Traversal) {

				

				Traversal = Traversal2; 
				IncidentProjected = IncidentProjected2; 

				TC = mix(TC,TC2,min(2.0*abs(Traversal2-Traversal),1.0)); 

			}


		}
		
		
		float Traversal3D = Traversal / sqrt(1.0-clamp(Incident.y*Incident.y,0.0,1.0));; 
		Traversal3D = clamp(Traversal3D, 0.0, 0.4 / abs(Incident.y)); 

		float MinTraversal = 1.0 / abs(Incident.y); 
		



		TC = (TC.xy + IncidentProjected.xy * Traversal).xy; 

		Albedo.xyz = pow(texture(DiffuseTextures, vec3(TC,TextureIdx)).xyz,vec3(2.2));

		//Arbitrary directions are not possible, due to limited data. boohoo
		/*
		if(BlockSide == 4u) {
		

			float TraversalRequired = abs(Traversal3D * Incident.y) / SunDirection.y; 

			vec3 ProjectedPosition = Traversal3D * Incident + SunDirection * TraversalRequired; 


			vec2 SunTC = fract(TC + ProjectedPosition.xz); 

			vec3 ProjectedSunDirection; 

			float SunTraversal = GetTraversal(SunTC, SunDirection, BlockSide, TBN, uint(TextureExData.x)-1u, ProjectedSunDirection); 

			float SunTraversal3D = SunTraversal / sqrt(1.0-clamp(SunDirection.y*SunDirection.y,0.0,1.0));; 
			//SunTraversal3D = clamp(SunTraversal3D, 0.0, 0.4 / abs(SunDirection.y)); 

			//Albedo.xyz =Albedo.xyz * vec3(TraversalRequired < SunTraversal3D ? 1.0 : 0.0); 
			//Albedo.xyz *= (1.0-vec3(abs(Incident.y * Traversal3D))); 




		}
		*/



	}
	else {
		Albedo.xyz = pow(texture(DiffuseTextures, vec3(TC.xy, TextureIdx)).xyz,vec3(2.2));
	}
	Albedo.w = 0.0; 

	HighFreqNormal.xyz = Normal.xyz; 

	
	if(TextureExData.y != 0) {
		Normal.w = textureLod(EmissiveTextures, vec3(TC.xy, TextureExData.y-1),0.0).x * texelFetch(BlockData, int(BlockType),0).x; 
	}

	if(TextureExData.z != 0) {
		Albedo.w = textureLod(MetalnessTextures, vec3(TC.xy, TextureExData.z-1),0.0).x; 
	}

	HighFreqNormal.w = 0.5; 
	HighFreqNormal.xyz = normalize(TBN * (normalize(texture(NormalTextures, vec3(TC.xy, TextureIdx)).xyz * 2.0 - 1.0)));
	HighFreqNormal.w = texture(RoughnessTextures, vec3(TC, TextureIdx)).x;	


	//vec3 LightVolumeSample = WorldPos.xyz + normalize(mix(Normal.xyz,HighFreqNormal.xyz,0.4)) * .5; 

	
	
}