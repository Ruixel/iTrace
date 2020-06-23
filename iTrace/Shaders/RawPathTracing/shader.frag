#version 330
#extension GL_ARB_bindless_texture : enable
in vec2 TexCoord;


//idea: Denoise lighting as a spherical harmonic. 
//Very useful because we can reproject the denoised diffuse 
//for specular lighting. We can also denoise using a low-frequency 
//Description of the normal, and then reproject the lighting based on high frequency normal 

layout(location = 0) out vec4 IndirectDiffuse;
layout(location = 1) out vec4 Normal;
layout(location = 2) out vec3 WorldPos;
layout(location = 3) out vec4 IndirectSpecular;
layout(location = 4) out float Direct;


uniform sampler2D Normals;
uniform sampler2D WorldPosition;
uniform sampler3D Voxels;
uniform sampler2D Sobol;
uniform sampler2D Ranking;
uniform sampler2D Scrambling;
uniform sampler2DArray DiffuseTextures;
uniform sampler2DArray EmissiveTextures;
uniform sampler2D Depth; 

uniform sampler1D TextureData;
uniform samplerCube Sky;
uniform samplerCube SkyNoMie; 
uniform sampler3D LightingData; 
uniform sampler1D TextureExData; 
uniform sampler1D BlockData; 
uniform sampler2D BlockerData; 

uniform sampler2DArrayShadow HemisphericalShadowMap; 
uniform mat4 HemisphericalMatrices[48]; 
uniform vec3 HemisphericalDirections[48]; 

uniform bool UseWhiteNoise;

uniform vec3 CameraPosition;
uniform int FrameCount;

uniform sampler2DShadow DirectionalCascades[4]; 
uniform mat4 DirectionMatrices[4]; 
uniform vec3 SunColor; 
uniform vec3 LightDirection; 


int FetchFromTexture(sampler2D Texture, int Index) {

	int Width = 256;
	int Height = textureSize(Texture, 0).y;

	ivec2 Pixel = ivec2(Index % Width, Index / Width);

	return clamp(int(texelFetch(Texture, ivec2(Pixel), 0).x * 255.0 + .1), 0, 255);

}

float seed;

vec2 hash2White() {
	return fract(sin(vec2(seed += 0.1, seed += 0.1)) * vec2(43758.5453123, 22578.1459123));
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

//Idea of using vogel disk originally from https://www.gamedev.net/tutorials/programming/graphics/contact-hardening-soft-shadows-made-fast-r4906/ 
//(and gradient noise) 

vec2 VogelDisk(int SampleIdx, int SampleCount, float SampleCountRoot, float Phi) {
	
	float r = sqrt(SampleIdx + 0.5) / SampleCountRoot; 
	float Theta = SampleIdx * 2.4 + Phi; 

	return vec2(cos(Theta) * r, sin(Theta) * r); 

}

float GradientNoise(vec2 ScreenPos) {

	return fract(52.9829189 * fract(dot(ScreenPos, vec2(0.06711056f, 0.00583715)))); 

}

float DirectHQ(vec3 Position, float Penumbra, vec2 ScreenPos) {
		
	//first, find the correct shadow cascade! 

	vec3 NDC = vec3(-1.0); 

	int Cascade = -1; 

	for(int i = 0; i < 4; i++) {
		
		vec4 Clip = DirectionMatrices[i] * vec4(Position, 1.0); 
	
		NDC = Clip.xyz / Clip.w; 

		if((abs(NDC.x) < 0.9 && abs(NDC.y) < 0.9) || (abs(NDC.x) < 1.0 && abs(NDC.y) < 1.0 && i == 3)) {
			Cascade = i; 
			break; 
		}

	}

	if(Cascade == -1) 
		return 0.0; 

	float Shadow = 0.0; 

	float Noise = hash(ivec2(ScreenPos),FrameCount / 4,0) * 2.4; 


	for(int Sample = 0; Sample < 8; Sample++) {

		vec2 VogelFetch = VogelDisk(Sample, 8, 2.82842712475, Noise); 

		vec2 ShadowCoord = (NDC.xy) * 0.5 + 0.5 + VogelFetch * clamp(Penumbra * 1.45,0.0,0.05); 

		ShadowCoord = clamp(ShadowCoord, vec2(0.0), vec2(1.0)); 

		Shadow += texture(DirectionalCascades[Cascade], vec3(ShadowCoord.xy,(NDC.z*0.5+0.5) -0.000018)); 

	}

	return Shadow / 8.0; 	

}

//useful for indirect bounce purposes (as quality isn't going to be super relevant) 

float DirectBasic(vec3 Position) {

	vec3 NDC = vec3(-1.0); 

	int Cascade = -1; 

	for(int i = 0; i < 4; i++) {
		
		vec4 Clip = DirectionMatrices[i] * vec4(Position, 1.0); 
	
		NDC = Clip.xyz / Clip.w; 

		if((abs(NDC.x) < 0.9 && abs(NDC.y) < 0.9) || (abs(NDC.x) < 1.0 && abs(NDC.y) < 1.0 && i == 3)) {
			Cascade = i; 
			break; 
		}

	}
	if(Cascade == -1) 
		return 0.0; 




	return texture(DirectionalCascades[Cascade], vec3(NDC.xy * 0.5 + 0.5, (NDC.z * 0.5 + 0.5)-0.00009)); 

}




vec3 BlockNormals[6] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(-1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, -1.0, 0.0),
	vec3(0.0, 0.0, 1.0),
	vec3(0.0, 0.0, -1.0)
	);

int SidesTranslated[6] = int[](
	2, 3, 4, 5, 0, 1
	);

int GetTextureIdx(int Type, int Side) {

	return int(texelFetch(TextureData, Type * 2 + (Side / 3), 0)[Side % 3] * 255.0 + 0.5);

}

float GetEmissiveStrenght(int Type) {
	return texelFetch(BlockData, Type, 0).x; 
}


bool RawTraceOld(vec3 RayDirection, vec3 Origin, inout int Block, inout int Face, inout vec3 Normal, inout vec2 TexCoord, inout vec3 Position, int Steps) {

	Position = Origin;

	vec3 Clamped = vec3(RayDirection.x > 0.0 ? 1.0 : 0.0, RayDirection.y > 0.0 ? 1.0 : 0.0, RayDirection.z > 0.0 ? 1.0 : 0.0);

	vec3 NextPlane = floor(Position + Clamped);

	for (int Step = 0; Step < Steps; Step++) {



		vec3 Next = (NextPlane - Position) / RayDirection;

		int SideHit = 0;


		if (Next.x < min(Next.y, Next.z)) {
			Position += RayDirection * Next.x;
			Position.x = NextPlane.x;
			NextPlane.x += sign(RayDirection.x);

		}
		else if (Next.y < Next.z) {
			Position += RayDirection * Next.y;
			Position.y = NextPlane.y;
			NextPlane.y += sign(RayDirection.y);
			SideHit = 1;
		}
		else {
			Position += RayDirection * Next.z;
			Position.z = NextPlane.z;
			NextPlane.z += sign(RayDirection.z);
			SideHit = 2;
		}

		vec3 TexelCoord = (NextPlane - Clamped);

		ivec3 CoordInt = ivec3(TexelCoord.x, TexelCoord.y, TexelCoord.z);

		//	std::cout << CoordInt.x << ' ' << CoordInt.y << ' ' << CoordInt.z << '\n'; 

		if (CoordInt.x > -1 && CoordInt.x < 128 &&
			CoordInt.y > -1 && CoordInt.y < 128 &&
			CoordInt.z > -1 && CoordInt.z < 128) {

			vec3 UVWP = Position - floor(Position);

			if (SideHit == 0)
				TexCoord = UVWP.yz;
			else if (SideHit == 1)
				TexCoord = UVWP.xz;
			else
				TexCoord = UVWP.xy;



			int Side = ((SideHit + 1) * 2) - 1;

			if (SideHit == 0) {
				if (Position.x - TexelCoord.x > 0.5)
					Side = 0;
			}
			else if (SideHit == 1) {
				if (Position.y - TexelCoord.y > 0.5)
					Side = 2;
			}
			else {
				if (Position.z - TexelCoord.z > 0.5)
					Side = 4;
			}

			Normal = BlockNormals[Side];

			Block = int(floor(textureLod(Voxels, TexelCoord.zyx / vec3(128.0),0.0) * 255.0 + .9));
			Face = Side;


			if (Block != 0)
				return true;
		}
		else return false;
	}

	//}



	return false;
}



//there are... a fuckton of optimizations available here 
//first of all, it branches too much (way too much) 
//secondly, we don't have any kind of acceleration structure set up 
//thirdly, we don't abuse the fact that we only care about one bit 
//with all these optimizations in place, it should be entirely possible 
//to make this new one AT LEAST 3 times faster than what we currently have

//but, first of all... make a proper damn profiler, so we can actually measure how long this takes 

vec4 IntersectBitMask(int Mask, vec3 Position, vec3 RayDirection, out vec2 TC) {

	vec3 Clamped = vec3(RayDirection.x > 0.0 ? 1.0 : 0.0, RayDirection.y > 0.0 ? 1.0 : 0.0, RayDirection.z > 0.0 ? 1.0 : 0.0);

	vec3 NextPlane = floor(Position + Clamped);

	for (int Step = 0; Step < 8; Step++) {

		vec3 Next = (NextPlane - Position) / RayDirection;

		vec3 TexelCoord = (NextPlane - Clamped);

		ivec3 CoordInt = ivec3(TexelCoord.x, TexelCoord.y, TexelCoord.z);

		if(CoordInt.x < 0 || CoordInt.y < 0 || CoordInt.z < 0 || CoordInt.x > 1 || CoordInt.y > 1 || CoordInt.z > 1) {
			
			return vec4(-1); 

		}

		

		bool Opaque = ((Mask >> (CoordInt.x * 4 + CoordInt.y * 2 + CoordInt.z)) & 1) != 0; 

		if(Opaque) {
		
			vec3 DeltaPos = Position - TexelCoord; 
			 
			vec3 aDeltaPos = abs(DeltaPos * 2.0 - 1.0); 

			int Side = 0; 

			if(aDeltaPos.x > max(aDeltaPos.y, aDeltaPos.z)) {
				Side = DeltaPos.x > 0.5 ? 0 : 1; 
			}
			else if(aDeltaPos.y > aDeltaPos.z) {
				Side = DeltaPos.y > 0.5 ? 2 : 3; 
			}
			else {
				Side = DeltaPos.z > 0.5 ? 4 : 5; 
			}
			vec3 UVWP = Position - floor(Position);

			if (Side == 0 || Side == 1)
				TC = UVWP.yz;
			else if (Side == 2 || Side == 3)
				TC = UVWP.xz;
			else
				TC = UVWP.xy;

			return vec4(Position, Side); 

		}

		int SideHit = 0;

		if (Next.x < min(Next.y, Next.z)) {
			Position += RayDirection * Next.x;
			Position.x = NextPlane.x;
			NextPlane.x += sign(RayDirection.x);

		}
		else if (Next.y < Next.z) {
			Position += RayDirection * Next.y;
			Position.y = NextPlane.y;
			NextPlane.y += sign(RayDirection.y);
			SideHit = 1; 
		}
		else {
			Position += RayDirection * Next.z;
			Position.z = NextPlane.z;
			NextPlane.z += sign(RayDirection.z);
			SideHit = 2; 
		}

		


	}

}
float RayBoxIntersection(vec3 Origin, vec3 InverseDir, vec3 Min, vec3 Max) {

	float t1 = (Min.x - Origin.x)*InverseDir.x;
	float t2 = (Max.x - Origin.x)*InverseDir.x;
	float t3 = (Min.y - Origin.y)*InverseDir.y;
	float t4 = (Max.y - Origin.y)*InverseDir.y;
	float t5 = (Min.z - Origin.z)*InverseDir.z;
	float t6 = (Max.z - Origin.z)*InverseDir.z;

	float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
	float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));



	return (tmax > 0.0 && tmax > tmin) ? tmin : -1.0;  

}

	


bool RawTrace(vec3 RayDirection, vec3 Origin, inout int Block, inout int Face, inout vec3 Normal, inout vec2 TexCoord, inout vec3 Position, int Steps) {

	
	Position = Origin / 2.0;

	vec3 Clamped = vec3(RayDirection.x > 0.0 ? 1.0 : 0.0, RayDirection.y > 0.0 ? 1.0 : 0.0, RayDirection.z > 0.0 ? 1.0 : 0.0);

	vec3 NextPlane = floor(Position + Clamped);

	for (int Step = 0; Step < Steps; Step++) {



		vec3 Next = (NextPlane - Position) / RayDirection;

		int SideHit = 0;

		if (Next.x < min(Next.y, Next.z)) {
				Position += RayDirection * Next.x;
				Position.x = NextPlane.x;
				NextPlane.x += sign(RayDirection.x);

			}
			else if (Next.y < Next.z) {
				Position += RayDirection * Next.y;
				Position.y = NextPlane.y;
				NextPlane.y += sign(RayDirection.y);
				SideHit = 1;
			}
			else {
				Position += RayDirection * Next.z;
				Position.z = NextPlane.z;
				NextPlane.z += sign(RayDirection.z);
				SideHit = 2;
			}
		
		vec3 TexelCoord = (NextPlane - Clamped);

		ivec3 CoordInt = ivec3(TexelCoord.x, TexelCoord.y, TexelCoord.z);

		//	std::cout << CoordInt.x << ' ' << CoordInt.y << ' ' << CoordInt.z << '\n'; 

		

		if (CoordInt.x > -1 && CoordInt.x < 64 &&
			CoordInt.y > -1 && CoordInt.y < 64 &&
			CoordInt.z > -1 && CoordInt.z < 64) {

			Block = int(floor(texelFetch(Voxels, CoordInt.zyx,1) * 255.0 + .9));
			//Face = Side;


			if (Block != 0) {
				
				//try to intersect bitmask 

				vec3 RelativeOrigin = Origin - CoordInt * 2; 



				float TemporaryT = RayBoxIntersection(RelativeOrigin, 1.0 / RayDirection, vec3(0.0), vec3(2.0)); 
				
				vec4 Temp = IntersectBitMask(Block, RelativeOrigin + RayDirection * (TemporaryT+.01), RayDirection,TexCoord); 

				if(Temp.w != -1) {
					int Side = int(Temp.w);

					Position = CoordInt * 2 + Temp.xyz; 

					Normal = BlockNormals[Side]; 

					Block = int(floor(textureLod(Voxels, (Position.zyx-Normal.zyx*.5) / vec3(128.0),0.0) * 255.0 + .9));;
					
					Face = Side; 


				
					return true; 
				}
				


			}

			

		}
		else return false;

		


	}

	//}



	return false;

}

float Rand_Seed = 0.0;

vec2 hash2() {
	return fract(sin(vec2(Rand_Seed += 0.1, Rand_Seed += 0.1)) * vec2(43758.5453123, 22578.1459123));
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


vec3 GetHemisphericalShadowMaphit(vec3 WorldPos, vec3 Normal, int i, int maxi) {
	
	float Noise = hash(ivec2(gl_FragCoord).xy, FrameCount*maxi+i, 20); 

	int Sample = clamp(int(Noise * 48), 0, 47); 

	vec4 ClipSpace = HemisphericalMatrices[Sample] * vec4(WorldPos, 1.0); 

	ClipSpace.xyz /= ClipSpace.w; 

	ClipSpace.xyz = ClipSpace.xyz * 0.5 + 0.5; 


	vec3 Direction = HemisphericalDirections[Sample]; 

	float Weight = 4.0 * max(dot(HemisphericalDirections[Sample], Normal), 0.0); 



	return texture(SkyNoMie, Direction).xyz * texture(HemisphericalShadowMap, vec4(ClipSpace.xy,Sample,ClipSpace.z-0.00009)) * Weight; 

}

uniform bool DoRayTracing; 



bool RayBoxIntersect(vec3 Origin, vec3 Direction, vec3 InverseDirection, vec3 Min, vec3 Max, inout vec3 Normal, inout vec3 Position) {
	
	//first, check for intersection! 

	float t = RayBoxIntersection(Origin, InverseDirection, Min, Max); 

	if(t > 0.0) {
	
		
		Position = Origin + Direct * t; 

		//compute normal! 

		
		vec3 Center = (Min + Max) / 2.0; 
		vec3 Size = (Max - Min); 

		vec3 Vector = (Position - Center) / Size; 
		vec3 AVector = abs(Vector); 

		if(AVector.x > max(AVector.y, AVector.z)) {
		
			Normal = AVector.x > 0.0 ? vec3(1.0,0.0,0.0) : vec3(-1.0,0.0,0.0); 

		}
		else if(AVector.y > AVector.z) {
			Normal = AVector.y > 0.0 ? vec3(0.0,1.0,0.0) : vec3(0.0,-1.0,0.0); 
		}
		else {
			Normal = AVector.z > 0.0 ? vec3(0.0,0.0,1.0) : vec3(0.0,0.0,-1.0); 
		}
		
		return true; 

	}



	return false; 

}



vec4 GetRayShading(vec3 Origin, vec3 Direction, vec3 Normal, bool Specular) {


	if(!DoRayTracing) {
		
		vec3 Diffuse = GetHemisphericalShadowMaphit(Origin, Normal, 0, 1); 
		Diffuse += textureLod(LightingData, (Origin + Normal * .5).zyx / 128.0, 0.0).xyz;
		return vec4(Diffuse, 1.0); 
	
	
	}

	int Block, Face;
	vec3 OutNormal, Position;
	vec2 TexCoord;

	vec4 LightingDataInc = textureLod(LightingData, (Origin + Normal * .5).zyx / 128.0, 0.0); 
		


	// return vec4(LightingDataInc.xyz,1.0); 
	//return vec4(GetHemisphericalShadowMaphit(Origin, Normal), 1.0); 
	 
	//bool Hit = RayBoxIntersect(Origin, Direction, 1.0/Direction, CameraPosition.xyz - vec3(0.3,1.49,0.3), CameraPosition.xyz + vec3(0.3,0.0,0.3), OutNormal, Position); 
	bool Hit = false; 
	bool PlayerHit = Hit; 

	if(!Hit)
		Hit = RawTraceOld(Direction, Origin, Block, Face, OutNormal, TexCoord, Position, 4);

	if(!Hit) {
		Hit = RawTrace(Direction, Origin, Block, Face, OutNormal, TexCoord, Position, 256);
	}



	vec4 Diffuse = vec4(0.0); 

	if (Hit) {
		vec3 BlockColor = vec3(0.745098039,0.549019608,0.521568627); 
		float Emissive = 0.0; 
		if(!PlayerHit) {

		

		if (Face == 0 || Face == 1) {
			TexCoord = TexCoord.yx;
		}
		if (Face != 2 && Face != 3) {
			TexCoord.y = 1.0 - TexCoord.y;
		}
		
		if(Face == 4 || Face == 1) {
			TexCoord.x = 1.0 - TexCoord.x; 
		}


		int TextureIdx = GetTextureIdx(Block, SidesTranslated[Face]); 
		ivec2 TextureExData = ivec2(texelFetch(TextureExData, TextureIdx,0).xy * 255); 
		BlockColor = pow(texture(DiffuseTextures, vec3(TexCoord.xy, TextureIdx)).xyz, vec3(2.2)); 

		

		if(TextureExData.y != 0) {
			
			Emissive = textureLod(EmissiveTextures, vec3(TexCoord.xy, TextureExData.y-1),0.0).x * GetEmissiveStrenght(Block); 

		}

		}

		Diffuse.xyz = BlockColor * Emissive;  



		//Sky lighting: 



		int s = 20; 

		vec3 HemiSpherical = vec3(0.0); 

		
		
		


		int j = 0; 
		int i = 0; 

		while(i<12) {


			int Sample = i * 6 + ((FrameCount) % 16);
			Sample = Sample % 48; 
			vec3 Direction = HemisphericalDirections[Sample]; 

			float _Weight = 4.0 * (dot(Direction, OutNormal)); 

			if(true) { 
			_Weight = max(_Weight, 0.0); 


			vec4 ClipSpace = HemisphericalMatrices[Sample] * vec4(Position, 1.0); 

			ClipSpace.xyz /= ClipSpace.w; 

			ClipSpace.xyz = ClipSpace.xyz * 0.5 + 0.5; 


			HemiSpherical += texture(Sky, Direction).xyz * texture(HemisphericalShadowMap, vec4(ClipSpace.xy,Sample,ClipSpace.z-0.00027)) * _Weight; 
			j = 0; 

			i++; 
			
			}


		}

		HemiSpherical /= 12.0; 

		//HemiSpherical =  GetHemisphericalShadowMaphit(Position, OutNormal, 0, 1); 
	
		

		Diffuse.xyz += BlockColor * HemiSpherical; 

		Diffuse.xyz += DirectBasic(Position) * max(dot(OutNormal, LightDirection), 0.0) * SunColor * BlockColor; 

		//Diffuse.xyz = OutNormal; 

		//For nearest: 
		//Diffuse.xyz += BlockColor * texelFetch(LightingData, ivec3(Position + OutNormal * .5).zyx, 0).xyz; 
		
		vec4 LightingData = textureLod(LightingData, (Position + OutNormal * .5).zyx / 128.0, 0.0); 
		
		//return LightingData.xyz; 

		Diffuse.xyz += (LightingData.xyz) * BlockColor; 

		//idea: if specular and roughness < treshhold, consider applying some super basic ambient light.


		//AO: 
		Diffuse.w = pow(min(distance(Origin, Position), 1.0), 5.0);
	}
	else {
		//GI: 
		if(Specular) {
			Diffuse.xyz = textureLod(Sky, Direction,0.0).xyz; 
		}
		else {
			Diffuse.xyz = textureLod(SkyNoMie, Direction,8.0).xyz; 
		}
		//Diffuse.xyz = vec3(0.0); 
		//AO: 
		Diffuse.w = 1.0; 
	}		


	return Diffuse; 

}



vec3 ImportanceGGX(vec2 xi, float roughness)
{
    float r_square = roughness * roughness;
    float phi = 6.2831 * xi.x;
    float cos_theta = sqrt((1 - xi.y) / (1 + (r_square * r_square - 1) * xi.y));
    float sin_theta = sqrt(1 - cos_theta * cos_theta);

    return vec3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
}


vec3 GetSpecularRayDirection(vec3 RawDirection, vec3 Normal, vec3 Incident, float Roughness, ivec2 Pixel) {

	vec3 v0 = abs(Normal.z) < 0.999f ? vec3(0.f, 0.f, 1.f) : vec3(0.f, 1.f, 0.f);

	vec3 Tangent = normalize(cross(v0, Normal));
	vec3 Bitangent = normalize(cross(Tangent, Normal));


	for(int Try = 0; Try < 3; Try++) {
		
		vec2 Xi = hash2(Pixel,FrameCount,Try+3) * vec2(1.0, 0.2); 

		vec3 rho = ImportanceGGX(Xi, clamp(sqrt(Roughness), 0.001f, 1.0f)); 

		vec3 TryDirection = normalize(0.001f + rho.x * Tangent + rho.y * Bitangent + rho.z * RawDirection); 

		if(dot(TryDirection, Normal) > 0.0005f) {
			return TryDirection; 
		}

	}
	return RawDirection; 
}

ivec2 States[] = ivec2[](
	ivec2(1, 1),
	ivec2(0, 1),
	ivec2(0, 0),
	ivec2(1, 0));

uniform float znear; 
uniform float zfar; 

float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
} 


void main() {

	Rand_Seed = (TexCoord.x * TexCoord.y) * 500.0 * 20.0;

	ivec2 Pixel = ivec2(gl_FragCoord.xy) * 4 + States[FrameCount % 4] * 2;

	Rand_Seed = (Pixel.x * 540 + Pixel.y) * 0.01; 

	vec4 RawNormal = texelFetch(Normals, Pixel, 0); 

	WorldPos = texelFetch(WorldPosition, Pixel, 0).rgb;
	Normal.xyz = RawNormal.rgb;
	Normal.w = LinearDepth(texelFetch(Depth, Pixel, 0).x); 

	float Penum = 0.0; 
	float PenumWeight = 0.0; 

	for(int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {
		
			ivec2 Pixel = ivec2(gl_FragCoord.xy) / 2 + ivec2(x,y); 

			vec2 BlockerInfo = texelFetch(BlockerData, Pixel, 0).xy; 

			float Weight = 1.0 / (1.0 + 100.0 * abs(BlockerInfo.y - Normal.w) * abs(BlockerInfo.y - Normal.w)); 

			Penum += BlockerInfo.x * Weight; 
			PenumWeight += Weight; 
			


		}
	}

	Penum /= PenumWeight; 



	Direct = DirectHQ(WorldPos,max(Penum,0.0002),vec2(Pixel) / 2); 
	//Direct = pow(texelFetch(Depth, Pixel, 0).x,3000.0); 
	//	Direct = texture(BlockerData, TexCoord).x; 



	vec2 hash = hash2();

	if (!UseWhiteNoise) {
		hash = hash2(Pixel / 2, FrameCount / 4, 0);
	}

	vec3 Incident = normalize(WorldPos - CameraPosition);
	vec3 Direction = cosWeightedRandomHemisphereDirection(Normal.xyz, hash);
	vec3 SpecularDirection = GetSpecularRayDirection(reflect(Incident, Normal.xyz), Normal.xyz, Incident, RawNormal.w, Pixel);
	vec4 Diffuse = GetRayShading(WorldPos + Normal.xyz * 0.0025, Direction,Normal.xyz, false); 
	vec4 Specular = GetRayShading(WorldPos + Normal.xyz * 0.0025, SpecularDirection, Normal.xyz, true); 

	float L = length(Normal.xyz); 

	if(L < 0.5 || L > 1.5) {
		Diffuse.xyz = vec3(0.0);
		Specular.xyz = vec3(0.0); 
		Direction = vec3(0.0,1.0,0.0); 

	}
	
	IndirectDiffuse = Diffuse; 
	IndirectSpecular = Specular; 

	Pixel = ivec2(gl_FragCoord.xy); 

	if(Pixel.x == (960) / 4 && Pixel.y == 540 / 4) {
	
		//IndirectDiffuse.xyz = vec3(50.0); 

	}
}