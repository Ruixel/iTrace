#version 400

layout(location = 0) out vec4 Normal; 
layout(location = 1) out vec3 WorldPos; 
layout(location = 2) out vec4 Albedo; 
layout(location = 3) out vec4 HighFreqNormal; 
layout(location = 4) out float DirectMultiplier; 
layout(location = 5) out vec4 ParallaxData; 
layout(location = 6) out vec3 _TC; 
layout(location = 7) out vec3 SimpleLighting; 

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

uniform ivec2 PositionBias;  

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
uniform sampler2DArray RainDrop; 


uniform sampler1D TextureData; 
uniform sampler1D TextureExData; 
uniform sampler1D BlockData; 

uniform sampler3D LightData; 

uniform sampler2D InTexCoord; 
uniform sampler2D InDepth; 
uniform sampler2D Noise; 
uniform sampler2DShadow ShadowMap; 
uniform sampler2D InNormal; 
uniform sampler2D InTangent; 

uniform int RainFrames; 
uniform mat4 RainMatrix; 

uniform float WetNess; 

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

void HandleWeather(mat3 TBN, vec3 LowFrequencyNormal, vec3 WorldPos, vec3 NiceWorldPos, inout vec3 Normal, inout float Roughness, inout float Metalness, inout vec3 Albedo) {

	if(WetNess <= 1e-4) 
		return; //not wet enough to matter! 





	float NoiseFetch = texture(Noise, WorldPos.xz / 16.0).x; 
	float NoiseFetch2 = pow(texture(Noise, WorldPos.zx / 32.0).x,2.0); 

	float Time = fract(Time * 2.0 + NoiseFetch) * RainFrames; 
	
	int Lower = int(Time); 
	int Upper = (Lower + 1) % RainFrames; 

	vec2 TC = WorldPos.xz * 16.0; 

	vec4 RainDropFetch = mix(texture(RainDrop, vec3(TC, Lower)), texture(RainDrop, vec3(TC, Upper)), fract(Time)); 

	vec3 RainDropNormal = TBN * (RainDropFetch.xyz * 2.0 - 1.0); 
	
	vec4 ClipSpace = RainMatrix * vec4(NiceWorldPos, 1.0); 
	vec3 NDCSpace = ClipSpace.xyz /= ClipSpace.w; 
	NDCSpace.xyz = NDCSpace.xyz * 0.5 + 0.5; 
	NDCSpace.z -= 0.0001; 


	float Facing = max(LowFrequencyNormal.y,0.0) * NoiseFetch2 * texture(ShadowMap, NDCSpace.xyz); 

	Metalness = mix(0.25*Facing*WetNess+Metalness, 1.0, RainDropFetch.w * Facing); //<- not physically correct, but weather is not noticeable enough without it
	Metalness = clamp(Metalness, 0.0, 1.0); 
	Roughness = mix(Roughness, 0.0, min(RainDropFetch.w+WetNess,1.0) * Facing); 

	Normal = mix(mix(Normal,LowFrequencyNormal,WetNess*0.25), RainDropNormal, sqrt(RainDropNormal.y * Facing)); 

	Normal.xyz = normalize(Normal.xyz); 
	//Metalness = texelFetch(ShadowMap, ivec2(NDCSpace.xy * 2048), 0).x;
	
	//Metalness = Metalness < (NDCSpace.z-0.0000001) ? 0.0 : 1.0; 

	Albedo.xyz = mix(Albedo.xyz, min(2.0 * Albedo.xyz * vec3(0.3,0.6,1.0),vec3(0.3,0.6,1.0)), Facing * RainDropFetch.w * 0.5); 

}

vec4 cubic(float v){
    vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
    vec4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return vec4(x, y, z, w) * (1.0/6.0);
}

vec4 textureBicubic(sampler2DArray sampler, vec2 texCoords, int tex){

   vec2 texSize = textureSize(sampler, 0).xy;
   vec2 invTexSize = 1.0 / texSize;

   texCoords = texCoords * texSize - 0.5;


    vec2 fxy = fract(texCoords);
    texCoords -= fxy;

    vec4 xcubic = cubic(fxy.x);
    vec4 ycubic = cubic(fxy.y);

    vec4 c = texCoords.xxyy + vec2 (-0.5, +1.5).xyxy;

    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4 (xcubic.yw, ycubic.yw) / s;

    offset *= invTexSize.xxyy;

    vec4 sample0 = texture(sampler, vec3(offset.xz,tex));
    vec4 sample1 = texture(sampler, vec3(offset.yz,tex));
    vec4 sample2 = texture(sampler, vec3(offset.xw,tex));
    vec4 sample3 = texture(sampler, vec3(offset.yw,tex));

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return mix(
       mix(sample3, sample2, sx), mix(sample1, sample0, sx)
    , sy);
}

void main() {	
	DirectMultiplier = 1.0; 
	ParallaxData = vec4(-1.0); 

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

	vec4 RawNormalFetch = texture(InNormal, TexCoord); 

	Normal.xyz = RawNormalFetch.xyz; 
	Normal.w = 0.0; 
	vec3 Tangent = texture(InTangent, TexCoord).xyz; 

	vec3 Bitangent = normalize(cross(Tangent,Normal.xyz)); 
	Bitangent = BlockBiTangents[BlockSide]; 
	mat3 TBN = mat3(Tangent, Bitangent, Normal); 

	int TextureIdx = GetTextureIdx(int(BlockType), int(BlockSide));
	
	ivec3 TextureExData = ivec3(texelFetch(TextureExData, TextureIdx,0).xyz * 255); 

	vec2 TC = RawTCData.xy; 

	//if(BlockSide == 4u) {
		//TC = ToTBNSpace(int(BlockSide),WorldPos).xy;
		//TC.y = 1.0 - TC.y; 
	//} 
	
	vec3 BetterWorldPos = WorldPos; 

	float QueriedLod = RawNormalFetch.w; 
	if(TextureExData.x != 0 && DoParallax)  {

		float lod = QueriedLod; 
		

		vec3 IncidentProjected; 

		float Traversal = GetTraversal(TC.xy, Incident, BlockSide, TBN, uint(TextureExData.x)-1u,IncidentProjected,lod) ;  
		
		
		float Traversal3D = Traversal / sqrt(1.0-clamp(Incident.y*Incident.y,0.0,1.0));; 
		Traversal3D = clamp(Traversal3D, 0.0, 0.4 / abs(Incident.y)); 

		float MinTraversal = 1.0 / abs(Incident.y); 
		



		TC = (TC.xy + IncidentProjected.xy * Traversal).xy; 

		BetterWorldPos = BetterWorldPos + Incident * Traversal; 

		Albedo.xyz = pow(textureBicubic(DiffuseTextures,TC,TextureIdx).xyz,vec3(2.2));
		Albedo.xyz = pow(textureLod(DiffuseTextures, vec3(TC,TextureIdx),QueriedLod).xyz,vec3(2.2));

		ParallaxData = vec4(float(BlockSide), float(TextureExData.x),lod, Traversal); 

	}
	else {
		//todo: add setting for bicubic interpolation for albedo 
		Albedo.xyz = pow(textureBicubic(DiffuseTextures, TC.xy,TextureIdx).xyz,vec3(2.2));
		Albedo.xyz = pow(textureLod(DiffuseTextures, vec3(TC.xy, TextureIdx),QueriedLod).xyz,vec3(2.2));
	}
	//Albedo.w = 0.0; 

	HighFreqNormal.xyz = Normal.xyz; 

	
	if(TextureExData.y != 0) {
		HighFreqNormal.w = textureLod(EmissiveTextures, vec3(TC.xy, TextureExData.y-1),QueriedLod).x * texelFetch(BlockData, int(BlockType),0).x; 
	}

	if(TextureExData.z != 0) {
		Albedo.w = textureLod(MetalnessTextures, vec3(TC.xy, TextureExData.z-1),QueriedLod).x; 
	}

	HighFreqNormal.xyz = normalize(TBN * (normalize(textureLod(NormalTextures, vec3(TC.xy, TextureIdx),QueriedLod).xyz * 2.0 - 1.0)));

	HighFreqNormal.xyz = normalize(mix(Normal.xyz,HighFreqNormal.xyz,0.5)); 

//	HighFreqNormal.w = texture(RoughnessTextures, vec3(TC, TextureIdx)).x;	
	
	//HighFreqNormal.w = 0.0; 
	SimpleLighting = texture(LightData,(WorldPos.xyz - vec3(PositionBias.x, 0.0, PositionBias.y) + Normal.xyz * .5).zyx / vec3(384.0,128.0,384.0)).xyz; 
	SimpleLighting = 64.0 * SimpleLighting * SimpleLighting; 
	Normal.w  = textureLod(RoughnessTextures, vec3(TC, TextureIdx),QueriedLod).x; 
	//	Normal.w = 0.0; 
		//HighFreqNormal.xyz = Normal.xyz; 

	_TC.xy = fract(TC);
	_TC.z = TextureIdx; 

	HandleWeather(TBN, Normal.xyz, BetterWorldPos.xyz, WorldPos.xyz, HighFreqNormal.xyz, HighFreqNormal.w, Albedo.w, Albedo.xyz); 

	bool Facing = dot(Normal.xyz, Incident) < 0.0; 

	Normal.xyz = !Facing ? -Normal.xyz : Normal.xyz; 
	HighFreqNormal.xyz = !Facing ? -HighFreqNormal.xyz : HighFreqNormal.xyz; 
	//Normal.xyz = HighFreqNormal.xyz; 
	Normal.xyz = normalize(Normal.xyz); 
}