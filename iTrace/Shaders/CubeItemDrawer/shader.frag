#version 330

in vec2 TexCoord; 
in vec3 Normal; 
in vec3 Position; 
layout(location = 0) out vec4 Lighting;

uniform int BlockType; 
uniform bool Refractive; 
uniform sampler2DArray DiffuseTextures; 
uniform sampler2DArray DisplacementTextures; 
uniform sampler2DArray NormalTextures; 
uniform sampler2DArray RoughnessTextures; 
uniform sampler2DArray MetalnessTextures; 
uniform sampler2DArray EmissiveTextures; 
uniform sampler2DArray OpacityTextures; 

uniform sampler1D TextureData;
uniform sampler1D TextureExData; 
uniform sampler2D HDRI; 

uniform sampler2D Sobol;
uniform sampler2D Ranking;
uniform sampler2D Scrambling;

uniform sampler1D BlockData; 


uniform vec3 CameraPosition; 

int GetTextureIdx(int Type, int Side) {
	return int(texelFetch(TextureData, Type*2+(Side/3), 0)[Side%3]*255.0+0.5);
}



uniform int ParallaxDirections; 
uniform int ParallaxResolution; 


vec3 ToTBNSpace(int Side, vec3 WorldPosition) {


	if(Side == 1) {
		return -WorldPosition.xyz; 
	}
	else if(Side == 0) {
		return WorldPosition.xyz * vec3(1.0,-1.0,1.0); 
	}
	else if(Side == 3) {
		return WorldPosition.zyx * vec3(1.0,-1.0,1.0); 
	}
	else if(Side == 2) {
		return WorldPosition.zyx * vec3(-1.0,-1.0,1.0); 
	}
	else {
		return WorldPosition.zxy; 
	}


}

int FetchFromTexture(sampler2D Texture, int Index) {

	int Width = 256;
	int Height = textureSize(Texture, 0).y;

	ivec2 Pixel = ivec2(Index % Width, Index / Width);

	return clamp(int(texelFetch(Texture, ivec2(Pixel), 0).x * 255.0 + .1), 0, 255);

}


float samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_32spp(int pixel_i, int pixel_j, int sampleIndex, int sampleDimension)
{
	// wrap arguments
	pixel_i = pixel_i & 127;
	pixel_j = pixel_j & 127;
	sampleIndex = sampleIndex & 255;
	sampleDimension = sampleDimension & 255;

	// xor index based on optimized ranking


	int rankedSampleIndex = sampleIndex ^ FetchFromTexture(Ranking, sampleDimension + (pixel_i + pixel_j * 128) * 8);

	// fetch value in sequence
	int value = FetchFromTexture(Sobol, sampleDimension + rankedSampleIndex * 256);

	// If the dimension is optimized, xor sequence value based on optimized scrambling
	value = value ^ FetchFromTexture(Scrambling, (sampleDimension % 8) + (pixel_i + pixel_j * 128) * 8);

	// convert to float and return
	float v = (0.5f + value) / 256.0f;
	return v;
}

float hash(ivec2 Pixel, int Index, int Dimension) {

	return samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_32spp(Pixel.x, Pixel.y, Index, Dimension);

}

vec2 hash2(ivec2 Pixel, int Index, int Dimension) {

	vec2 Returnhash;
	Returnhash.x = hash(Pixel, Index, 1);
	Returnhash.y = hash(Pixel, Index, 2);

	return Returnhash;

}

vec3 cosWeightedRandomHemisphereDirection(const vec3 n, vec2 rv2) {

	vec3  uu = normalize(cross(n, vec3(0.0, 1.0, 1.0)));
	vec3  vv = normalize(cross(uu, n));

	float ra = sqrt(rv2.y);
	float rx = ra * cos(6.2831 * rv2.x);
	float ry = ra * sin(6.2831 * rv2.x);
	float rz = sqrt(1.0 - rv2.y);
	vec3  rr = vec3(rx * uu + ry * vv + rz * n);

	return normalize(rr);
}

vec3 ImportanceGGX(vec2 xi, float roughness)
{
    float r_square = roughness * roughness;
    float phi = 6.2831 * xi.x;
    float cos_theta = sqrt((1 - xi.y) / (1 + (r_square * r_square - 1) * xi.y));
    float sin_theta = sqrt(1 - cos_theta * cos_theta);

    return vec3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
}


vec3 GetSpecularRayDirection(vec3 RawDirection, vec3 Normal, vec3 Incident, float Roughness, ivec2 Pixel, int Index) {

	vec3 v0 = abs(Normal.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(0.f, 1.f, 0.f);

	vec3 Tangent = normalize(cross(v0, Normal));
	vec3 Bitangent = normalize(cross(Tangent, Normal));


	for(int Try = 0; Try < 3; Try++) {
		
		vec2 Xi = hash2(Pixel,Index,Try+3) * vec2(1.0, 0.2); 

		vec3 rho = ImportanceGGX(Xi, clamp(sqrt(Roughness), 0.001f, 1.0f)); 

		vec3 TryDirection = normalize(0.001f + rho.x * Tangent + rho.y * Bitangent + rho.z * RawDirection); 

		if(dot(TryDirection, Normal) > 0.0005f) {
			return TryDirection; 
		}

	}
	return RawDirection; 
}



float SampleParallaxMap(vec3 Point, uint Texture) {
	
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
	return mix(textureLod(DisplacementTextures, vec3(PixelPoint, Texture), 0.0).x, textureLod(DisplacementTextures, vec3(PixelPoint2,Texture),0.0).x, Interp); 
}

float GetTraversal(vec2 TC, vec3 Direction, uint Side, uint Type, inout vec3 DirectionProjected) {
	DirectionProjected = ToTBNSpace(int(Side),Direction); 

	vec2 DirectionXZ = normalize(DirectionProjected.xy); 
	
	float Angle = atan(DirectionXZ.x, DirectionXZ.y); 

	return SampleParallaxMap(vec3(TC.x, (Angle-1.57079633) / 6.28318531,TC.y), Type) * 0.5; //temporary! 
}

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

vec3 SampleLighting(vec3 Direction) {
	return texture(HDRI, SampleSphericalMap(Direction)).xyz; 
}

const vec3 BlockTangents[3] = vec3[](
	vec3(1.0,0.0,0.0),
	vec3(0.0,0.0,-1.0),
	vec3(1.0,0.0,0.0)
); 

vec3 Fresnel(vec3 Incident, vec3 Normal, vec3 Specular, float Roughness) {
	return Specular + (max(vec3(1-Roughness) - Specular,vec3(Specular))) * pow(max(1.0 - abs(dot(Incident,Normal)),0.0), 5.0);
}

void main() {

	vec3 Incident = normalize(Position - CameraPosition); 

	vec3 Nabs = abs(Normal); 

	int Side = 0;
	
	vec2 ATC = TexCoord; 

	if(Nabs.x > max(Nabs.y, Nabs.z)) {

		ATC = ATC.yx; 

		ATC.y = 1.0 - ATC.y; 

		Side = 2 + int(max(-Nabs.x,0)+.5); 
	}
	else if(Nabs.z > Nabs.y) {

		ATC.y = 1.0 - ATC.y; 

		Side = int(max(-Nabs.z,0)+.5); 
	}
	else {

		ATC = ATC.yx; 
		ATC.x = 1.0 - ATC.x; 

		Side = 4 + int(max(-Nabs.y,0)+.5); 
	}

	
	

	int TextureIdx = GetTextureIdx(BlockType, Side); 

	ivec4 TextureExData = ivec4(texelFetch(TextureExData, TextureIdx,0) * 255); 

	if(TextureExData.w != 0) {
		float Opacity = texture(OpacityTextures, vec3(ATC.xy, uint(TextureExData.w)-1u)).x; 
		if(Opacity < 0.95) 
			discard; 
	}

	if(TextureExData.x != 0) {
		vec3 IncidentProjected; 

		float Traversal = GetTraversal(ATC.xy, Incident, uint(Side), uint(TextureExData.x)-1u,IncidentProjected) ;


		ATC = (ATC.xy + IncidentProjected.xy * Traversal).xy; 


	}
	vec3 Albedo = pow(texture(DiffuseTextures, vec3(ATC, TextureIdx)).xyz, vec3(2.2)); 

	float Emissive = 0.0, Roughness = 0.5, Metalness = 0.0; 
	
	if(TextureExData.y != 0) {
		Emissive = textureLod(EmissiveTextures, vec3(ATC.xy, TextureExData.y-1),0.0).x * texelFetch(BlockData, int(BlockType),0).x; 
	}

	if(TextureExData.z != 0) {
		Metalness = textureLod(MetalnessTextures, vec3(ATC.xy, TextureExData.z-1),0.0).x; 
	}
	Roughness = texture(RoughnessTextures, vec3(ATC, TextureIdx)).x;	

	vec3 Tangent = BlockTangents[Side/2]; 

	vec3 Bitangent = normalize(cross(Tangent, Normal.xyz)); 



	mat3 TBN = mat3(Tangent, Bitangent, Normal); 
	
	vec3 NormalMap = normalize(TBN * (texture(NormalTextures, vec3(ATC, TextureIdx)).xyz * 2.0 - 1.0)); 

	Lighting.w = 1.0; 

	if(Refractive) {
		vec3 RefractionVector = refract(Incident, NormalMap, 1.0/1.1); 

		Lighting.xyz = Albedo.xyz * SampleLighting(RefractionVector); 
		Lighting.xyz = clamp(Lighting.xyz * 2.0, vec3(0.0), vec3(1.0)); 
		return; 
	}


	vec3 ReflectionVector = reflect(Incident, NormalMap); 

	vec3 SpecularLighting = vec3(0.0), DiffuseLighting = vec3(0.0);
	
	ivec2 Pixel = ivec2(gl_FragCoord.xy); 

	for(int i = 0; i < 16; i++) {
		
		vec2 hash = hash2(Pixel, i, 0);

		vec3 Direction = cosWeightedRandomHemisphereDirection(Normal.xyz, hash);
		vec3 SpecularDirection = GetSpecularRayDirection(ReflectionVector, NormalMap, Incident, Roughness, Pixel, i);

		DiffuseLighting += SampleLighting(Direction); 
		SpecularLighting += SampleLighting(SpecularDirection); 

	}

	DiffuseLighting /= 16.0; 
	SpecularLighting /= 16.0; 

	vec3 F0 = mix(vec3(0.04), Albedo, Metalness); 
	vec3 SpecularColor = Fresnel(Incident, NormalMap.xyz, F0, Roughness); 
	vec3 DiffuseColor = mix(Albedo, vec3(0.0), Metalness); 

	Lighting.xyz = DiffuseColor * DiffuseLighting + SpecularLighting * SpecularColor + Emissive * Albedo; 
	Lighting.xyz = clamp(Lighting.xyz * 2.0, vec3(0.0), vec3(1.0)); 
	Lighting.w = 1.0; 

	

	//if(Side == 2) 
	//	Lighting.xyz = vec3(1.0); 

}