#version 330
#extension GL_ARB_bindless_texture : enable
in vec2 TexCoord;


#ifndef HAS_INJECTION

vec3 Multiplier[] = vec3[](vec3(0.0),vec3(1.0)); 
int RefractiveCount = 2; 

#endif 

ivec2 FragCoord = ivec2(0); 

//idea: Denoise lighting as a spherical harmonic. 
//Very useful because we can reproject the denoised diffuse 
//for specular lighting. We can also denoise using a low-frequency 
//Description of the normal, and then reproject the lighting based on high frequency normal 

layout(location = 0) out vec4 SHy;
layout(location = 1) out vec4 Normal;
layout(location = 2) out vec3 WorldPos;
layout(location = 3) out vec4 IndirectSpecular;
layout(location = 4) out vec3 Direct;
layout(location = 5) out vec4 Detail;
layout(location = 6) out float ShCg; 
layout(location = 7) out vec4 PackedSpatialData; 


uniform sampler2D Normals;
uniform sampler2D WorldPosition;
uniform sampler3D Voxels;
uniform sampler2D Sobol;
uniform sampler2D Ranking;
uniform sampler2D Scrambling;
uniform sampler2DArray DiffuseTextures;
uniform sampler2DArray EmissiveTextures;
uniform sampler2DArray DisplacementTextures; 
uniform sampler2DArray NormalTextures; 

uniform sampler2D Depth; 
uniform sampler2D ParallaxData; 
uniform sampler2D TCData; 
uniform sampler2D LowFrequencyNormal; 
uniform sampler2D Albedo; 

uniform sampler1D TextureData;
uniform samplerCube Sky;
uniform samplerCube SkyNoMie; 
uniform sampler3D LightingData; 
uniform sampler1D TextureExData; 
uniform sampler1D BlockData; 
uniform sampler2D BlockerData; 
uniform sampler2D ProjectedClouds;
uniform sampler2D WaterWorldPos; 
uniform sampler2D WaterNormal; 
uniform sampler2D WaterDepth; 

uniform sampler2D DeferredNormalData; 
uniform sampler2DArray Deferred; 

uniform sampler2DArrayShadow HemisphericalShadowMap; 
uniform mat4 HemisphericalMatrices[48]; 
uniform vec3 HemisphericalDirections[48]; 

uniform bool UseWhiteNoise;

uniform vec3 CameraPosition;
uniform int FrameCount;

uniform sampler2DShadow DirectionalCascades[4]; 
uniform sampler2D DirectionalCascadesRaw[4]; 
uniform sampler2D DirectionalRefractive[4]; 
uniform mat4 DirectionMatrices[4]; 
uniform vec3 SunColor; 
uniform vec3 SkyColor; 

uniform vec3 LightDirection; 

uniform int ParallaxDirections; 
uniform float Time; 
uniform int ParallaxResolution; 
uniform bool DoParallax; 
uniform ivec2 PositionBias; 
uniform int CheckerStep; 

uniform mat4 CameraMatrix; 




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

float PenumShifts[4] = float[4](1.0, 0.25, 0.0833333, 0.025); 

vec3 DirectHQ(vec3 Position, float Penumbra, vec2 ScreenPos, float SmoothNess) {
		
	//first, find the correct shadow cascade! 

	vec3 NDC = vec3(-1.0); 

	int Cascade = -1; 

	float Edge = 0.9 - hash(ivec2(ScreenPos),0,13) * 0.05; 

	for(int i = 0; i < 4; i++) {
		
		vec4 Clip = DirectionMatrices[i] * vec4(Position, 1.0); 
	
		NDC = Clip.xyz / Clip.w; 

		if((abs(NDC.x) < Edge && abs(NDC.y) < Edge) || (abs(NDC.x) < 1.0 && abs(NDC.y) < 1.0 && i == 3)) {
			Cascade = i; 
			break; 
		}

	}

	if(Cascade == -1) 
		return vec3(0.0); 

	vec3 Shadow = vec3(0.0); 

	float Noise = hash(ivec2(ScreenPos),FrameCount/4,12) * 2.4; 


	for(int Sample = 0; Sample < 8; Sample++) {

		vec2 VogelFetch = VogelDisk(Sample, 8, 2.82842712475, Noise); 

		vec2 ShadowCoord = (NDC.xy) * 0.5 + 0.5 + VogelFetch * clamp(Penumbra * 0.1 * SmoothNess * PenumShifts[Cascade] * PenumShifts[Cascade],0.0,0.05); 

		ShadowCoord = clamp(ShadowCoord, vec2(0.0), vec2(1.0)); 

		vec3 ShadowCapture = vec3(texture(DirectionalCascades[Cascade], vec3(ShadowCoord.xy,(NDC.z*0.5+0.5) -0.000018 * clamp(Penumbra*100.0,1.0,50.0)))); 

		if(ShadowCapture.x > 0.0) 
			ShadowCapture *= texture(DirectionalRefractive[Cascade], vec2(ShadowCoord.xy)).xyz; 

		Shadow += ShadowCapture; 



	}

	return (Shadow / 8.0); 	

}

//useful for indirect bounce purposes (as quality isn't going to be super relevant) 

vec3 DirectBasic(vec3 Position) {

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
		return vec3(0.0); 




	return texture(DirectionalCascades[Cascade], vec3(NDC.xy * 0.5 + 0.5, (NDC.z * 0.5 + 0.5)-0.00009))* texture(DirectionalRefractive[Cascade], NDC.xy * 0.5 + 0.5).xyz; 

}




vec3 BlockNormals[6] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(-1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, -1.0, 0.0),
	vec3(0.0, 0.0, 1.0),
	vec3(0.0, 0.0, -1.0)
	);

const vec3 _BlockNormals[6] = vec3[](
				vec3(0.0, 0.0, 1.0),
				vec3(0.0, 0.0, -1.0),
				vec3(1.0, 0.0, 0.0),
				vec3(-1.0, 0.0, 0.0),
				vec3(0.0, 1.0, 0.0),
				vec3(0.0, -1.0, 0.0)
);


const vec3 _BlockTangents[6] = vec3[](
	vec3(-1.0,0.0,0.0),
	vec3(1.0,0.0,0.0),
	vec3(0.0,0.0,1.0),
	vec3(0.0,0.0,-1.0),
	vec3(1.0,0.0,0.0),
	vec3(1.0,0.0,0.0)
); 

const vec3 _BlockBiTangents[6] = vec3[](
	vec3(0.0,-1.0,0.0),
	vec3(0.0,-1.0,0.0),
	vec3(0.0,-1.0,0.0),
	vec3(0.0,-1.0,0.0),
	vec3(0.0,0.0,1.0),
	vec3(0.0,0.0,-1.0)
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

vec3 Colors[4] = vec3[4](vec3(1.0), vec3(0.0,1.0,0.0), vec3(1.0,0.0,0.0), vec3(0.0,0.0,1.0)); 

bool RawTraceOld(vec3 RayDirection, vec3 Origin, inout int Block, inout int Face, inout vec3 Normal, inout vec2 TexCoord, inout vec3 Position, int Steps, inout vec3 RefractiveColor) {

	Position = Origin;

	vec3 Clamped = vec3(RayDirection.x > 0.0 ? 1.0 : 0.0, RayDirection.y > 0.0 ? 1.0 : 0.0, RayDirection.z > 0.0 ? 1.0 : 0.0);

	vec3 NextPlane = floor(Position + Clamped);

	RefractiveColor = vec3(1.0); 

	for (int Step = 0; Step < Steps; Step++) {



		vec3 Next = (NextPlane - Position) / RayDirection;

		int SideHit = 0;

		if(Step != 0) {
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

		vec3 TexelCoord = (NextPlane - Clamped);

		ivec3 CoordInt = ivec3(TexelCoord.x, TexelCoord.y, TexelCoord.z);

		//	std::cout << CoordInt.x << ' ' << CoordInt.y << ' ' << CoordInt.z << '\n'; 

		if (CoordInt.x > -1 && CoordInt.x < 384 &&
			CoordInt.y > -1 && CoordInt.y < 128 &&
			CoordInt.z > -1 && CoordInt.z < 384) {

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

			Block = int(floor(texelFetch(Voxels, ivec3(TexelCoord.zyx),0) * 255.0 + .9));
			Face = Side;

			vec3 RefrColor = Multiplier[clamp(Block,0,RefractiveCount-1)]; 


			float PowFactor = 1.0; 

			

			//float distFactor = clamp(distance(TexCoord, vec2(0.5)) * 2.0,0.0,1.0); 

			//PowFactor = 1.0 + 8.0 * distFactor * distFactor; 

			if(Block != 0 && Block < RefractiveCount-1)
			RefractiveColor *= RefrColor; 

			if(Block >= RefractiveCount-1) 
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

		

		if (CoordInt.x > -1 && CoordInt.x < 64*3 &&
			CoordInt.y > -1 && CoordInt.y < 64 &&
			CoordInt.z > -1 && CoordInt.z < 64*3) {

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

					Block = int(floor(texelFetch(Voxels, ivec3(Position.zyx-Normal.zyx*.5),0) * 255.0 + .9));;
					
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
	
	float Noise = hash(FragCoord, FrameCount*maxi+i, 20); 

	int Sample = clamp(int(Noise * 48), 0, 47); 

	vec4 ClipSpace = HemisphericalMatrices[Sample] * vec4(WorldPos, 1.0); 

	ClipSpace.xyz /= ClipSpace.w; 

	ClipSpace.xyz = ClipSpace.xyz * 0.5 + 0.5; 


	vec3 Direction = HemisphericalDirections[Sample]; 

	float Weight = 4.0 * max(dot(HemisphericalDirections[Sample], Normal), 0.0); 



	return texture(SkyNoMie, Direction).xyz * texture(HemisphericalShadowMap, vec4(ClipSpace.xy,Sample,ClipSpace.z-0.00009)) * Weight; 

}

uniform bool DoRayTracing; 


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

float GetTraversal(vec2 TC, vec3 Direction, uint Side, uint Type, inout vec3 DirectionProjected, float Lod) {
	DirectionProjected = ToTBNSpace(int(Side),Direction); 

	vec2 DirectionXZ = normalize(DirectionProjected.xy); 
	
	float Angle = atan(DirectionXZ.x, DirectionXZ.y); 

	return SampleParallaxMap(vec3(TC.x, (Angle-1.57079633) / 6.28318531,TC.y), Type, Lod) * 0.5; //temporary! 
}




uniform float znear; 
uniform float zfar; 

float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
} 

vec4 ScreenSpaceTrace(vec3 Origin, vec3 Direction, float MaxTraversal, int Steps, float Hash, out vec3 Normal, out vec3 WorldPos, float Precision, bool EA) {

	vec3 Step = (Direction * MaxTraversal) / float(Steps); 
	vec3 Pos = Origin + Step * Hash; 
	vec4 PreviousClip = CameraMatrix * vec4(Pos, 1.0); 
	PreviousClip.xyz /= PreviousClip.w; 
	if(abs(PreviousClip.x) > 1 || abs(PreviousClip.y) > 1 || abs(PreviousClip.z) > 1) 
			return vec4(-1.0); 
	PreviousClip.xyz = PreviousClip.xyz * 0.5 + 0.5; 
	

	float LinPrev = LinearDepth(PreviousClip.z); 

	ivec2 TextureSize = textureSize(Depth, 0).xy; 

	ivec2 Pixel = ivec2(-1); 

	for(int i = 0; i < Steps; i++) {
		
		Pos += Step; 

		vec4 CurrentClip = CameraMatrix * vec4(Pos, 1.0); 
		CurrentClip.xyz /= CurrentClip.w; 

		if(abs(CurrentClip.x) > 1 || abs(CurrentClip.y) > 1 || abs(CurrentClip.z) > 1) 
			return vec4(-1.0); 

		CurrentClip.xyz = CurrentClip.xyz * 0.5 + 0.5; 

		

		float zFetch = texelFetch(Depth, ivec2(CurrentClip.xy * TextureSize),0).x; 

		//were we behind it and are we NOW ahead of it?

		float LinCur = LinearDepth(CurrentClip.z); 
		if(CurrentClip.z > zFetch)
		if(abs(LinearDepth(zFetch)-LinCur) < max(Precision * abs(LinCur-LinPrev),0.1)) {
			Pixel = ivec2(CurrentClip.xy * TextureSize); 
			break; 
		}
		else if(EA){
			return vec4(-1.0); 
		}
		LinPrev = LinCur;  
		PreviousClip = CurrentClip; 

	}

	if(Pixel.x < 0) 
		return vec4(-1.0); 

	vec4 NormalFetch = texelFetch(LowFrequencyNormal, Pixel, 0);
	vec4 WorldPosFetch = texelFetch(WorldPosition, Pixel, 0);

	Normal.xyz = NormalFetch.xyz; 
	WorldPos.xyz = WorldPosFetch.xyz; 

	return vec4(texelFetch(Albedo, Pixel, 0).xyz, texelFetch(Normals, Pixel, 0).w); 

}


float ScreenSpaceTraceShadows(vec3 Origin, vec3 Direction, float MaxTraversal, int Steps, float Hash) {

	vec3 Step = (Direction * MaxTraversal) / float(Steps); 
	vec3 Pos = Origin + Step * Hash; 
	vec4 PreviousClip = CameraMatrix * vec4(Pos, 1.0); 
	PreviousClip.xyz /= PreviousClip.w; 
	if(abs(PreviousClip.x) > 1 || abs(PreviousClip.y) > 1 || abs(PreviousClip.z) > 1) 
			return 1.0; 
	PreviousClip.xyz = PreviousClip.xyz * 0.5 + 0.5; 
	

	float LinPrev = LinearDepth(PreviousClip.z); 

	ivec2 TextureSize = textureSize(Depth, 0).xy; 

	for(int i = 0; i < Steps; i++) {
		
		Pos += Step; 

		vec4 CurrentClip = CameraMatrix * vec4(Pos, 1.0); 
		CurrentClip.xyz /= CurrentClip.w; 

		if(abs(CurrentClip.x) > 1 || abs(CurrentClip.y) > 1 || abs(CurrentClip.z) > 1) 
			return 1.0; 

		CurrentClip.xyz = CurrentClip.xyz * 0.5 + 0.5; 

		

		float zFetch = texelFetch(Depth, ivec2(CurrentClip.xy * TextureSize),0).x; 

		//were we behind it and are we NOW ahead of it?

		float LinCur = LinearDepth(CurrentClip.z); 

		if(CurrentClip.z > zFetch && abs(LinearDepth(zFetch)-LinCur) < max(2.0 * abs(LinCur-LinPrev),0.1)) {
			return pow(float(i)/float(Steps),5); 
		}
		LinPrev = LinCur;  
		PreviousClip = CurrentClip; 

	}

	return 1.0; 

}
	
vec4 SampleCloud(vec3 Origin, vec3 Direction) {
	const vec3 PlayerOrigin = vec3(0,6200,0); 
	const float PlanetRadius = 6573 + 7773 * 0.1; 

	float Traversal = (PlanetRadius - (PlayerOrigin.y + Origin.y)) / Direction.y; 

	vec3 NewPoint = PlayerOrigin + Origin + Direction * Traversal; 

	//Fetch it! 

	float fade = exp(-Traversal*1.5e-4); 
	fade = clamp(fade, 0.0, 1.0); 
	vec4 Sample = texture(ProjectedClouds, fract(vec2((NewPoint.x-4096) / 8192, (NewPoint.z+4096) / 8192)));  

	Sample.a = mix(0.0,Sample.a,fade); 

	return Sample;  

}




vec4 GetRayShading(vec3 Origin, vec3 Direction, vec3 Normal, bool Specular, vec4 ParallaxData, vec3 TC, vec3 LowFrequencyNormal, out vec4 Detail, ivec2 Pixel, bool Water) {


	Detail = vec4(1.0); 
	if(!DoRayTracing) {
		
		vec3 Diffuse = GetHemisphericalShadowMaphit(Origin, Normal, 0, 1); 
		//Diffuse += textureLod(LightingData, (Origin + Normal * .5).zyx / vec3(384,128,384), 0.0).xyz;
		return vec4(Diffuse, 1.0); 
	
	
	}

	int Block, Face;
	vec3 OutNormal, Position;
	vec2 TexCoord;
	vec3 BlockColor = vec3(0.0); 
	vec4 LightingDataInc = textureLod(LightingData, (Origin + Normal * .5).zyx / 128.0, 0.0); 
	float Emissive = 0.0; 
	float DirectMultiplier = 1.0; 

	// return vec4(LightingDataInc.xyz,1.0); 
	//return vec4(GetHemisphericalShadowMaphit(Origin, Normal), 1.0); 
	 
	//bool Hit = RayBoxIntersect(Origin, Direction, 1.0/Direction, CameraPosition.xyz - vec3(0.3,1.49,0.3), CameraPosition.xyz + vec3(0.3,0.0,0.3), OutNormal, Position); 
	bool Hit = false; 
	
	//Parallax hit! 

	vec3 DirectionProjected; 

	
	float TraversalDirection = GetTraversal((TC.xy), Direction, uint(ParallaxData.x+.1), uint(ParallaxData.y+.1)-1u,DirectionProjected,ParallaxData.z); 


	//if((TraversalDirection+0.00390625) < ParallaxData.w * (1.0-pow(abs(DirectionProjected.z),4.0))) {
	if(false) {
		
		vec2 ProjectedTC = TC.xy + DirectionProjected.xy * TraversalDirection;
		
		BlockColor = pow(texture(DiffuseTextures, vec3(ProjectedTC, int(TC.z+.2))).xyz, vec3(2.2));  

		Position = Origin; 

		int Side = int(ParallaxData.x+.1); 

		mat3 TBN = mat3(_BlockTangents[Side], _BlockBiTangents[Side], _BlockNormals[Side]);
		
		OutNormal = normalize(TBN * (texture(NormalTextures, vec3(ProjectedTC, int(TC.z+.2))).xyz * 2.0 - 1.0));
		//Emissive = 30.0 * texture(EmissiveTextures, vec3(ProjectedTC, int(2))).x; 
		//return vec4(max(OutNormal,vec3(0.0)),0.0); 
		Hit = true; 

		vec3 SunProjected; 

		vec2 NewTC = TC.xy + DirectionProjected.xy * TraversalDirection; 

		float TraversalSun = GetTraversal(NewTC.xy, LightDirection, uint(ParallaxData.x+.1), uint(ParallaxData.y+.1)-1u, SunProjected, ParallaxData.z); 

		DirectMultiplier = ((TraversalSun+0.00390625) >= ParallaxData.w * (1.0-pow(abs(SunProjected.z),4.0)) ? 1.0 : sqrt(TraversalSun));


	}

	vec3 NormalSS, PositionSS; 

	if(true) {
		
		float Hash = hash(Pixel / 2, Water ? 0 : FrameCount / 4,35); 
		if(Water) 
			Hash = 0.0; 
		

		vec4 Trace = ScreenSpaceTrace(Origin, Direction, Specular ? (Water ? 4.0 : 2.0) : 1.0, Water ? 48 : 5, Hash, NormalSS, PositionSS, Specular ? 1.2 : 4.0, Specular); 
	
		if(Trace.x > -0.9) {
			Hit = true; 
			BlockColor = Trace.xyz; 
			Emissive = Trace.w; 
		}

	}
	
	//return vec4(ParallaxData.www,1.0);


	bool ParallaxHit = Hit; 

	vec3 Color  = vec3(1.0); 

	if(!Hit)
		Hit = RawTraceOld(Direction, Origin - vec3(PositionBias.x, 0, PositionBias.y), Block, Face, OutNormal, TexCoord, Position, Specular ? 64 : 64,Color);

		Position += vec3(PositionBias.x, 0, PositionBias.y); 

	if(ParallaxHit) {
		Position = PositionSS; 
		OutNormal = NormalSS; 
		Hit = true; 
	}

	if(!Hit) {
		//Hit = RawTrace(Direction, Origin, Block, Face, OutNormal, TexCoord, Position, 256);
	}
	


	vec4 Diffuse = vec4(0.0); 

	if (Hit) {
		
		
		if(!ParallaxHit) {

		

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
		if(!Water) 
		BlockColor = pow(texture(DiffuseTextures, vec3(TexCoord.xy, TextureIdx)).xyz,vec3(2.2)); 
		else 
		BlockColor = pow(textureLod(DiffuseTextures, vec3(TexCoord.xy, TextureIdx), 2.0).xyz,vec3(2.2)); 
		

		if(TextureExData.y != 0) {
			
			Emissive = textureLod(EmissiveTextures, vec3(TexCoord.xy, TextureExData.y-1),0.0).x * GetEmissiveStrenght(Block); 

		}

		}

		Diffuse.xyz = 2.0 * BlockColor * Emissive;  



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


			HemiSpherical +=texture(HemisphericalShadowMap, vec4(ClipSpace.xy,Sample,ClipSpace.z-0.0003)) * _Weight; 
			j = 0; 

			i++; 
			
			}


		}

		HemiSpherical /= 12.0; 
		HemiSpherical *= HemiSpherical*  SkyColor;  

		//HemiSpherical =  GetHemisphericalShadowMaphit(Position, OutNormal, 0, 1); 
	
		float DirectDensity = SampleCloud(Position.xyz, LightDirection).a; 

		DirectDensity = pow(DirectDensity, 4.0); 

		Diffuse.xyz += HemiSpherical * BlockColor; 

		Diffuse.xyz += DirectBasic(Position) * max(dot(OutNormal, LightDirection), 0.0) * SunColor * BlockColor  * DirectDensity; 

		vec4 LightingData = textureLod(LightingData, (Position - vec3(PositionBias.x, 0.0, PositionBias.y) + OutNormal * .5).zyx / vec3(384.0,128.0,384.0), 0.0); 
		
		//return LightingData.xyz; 

		Diffuse.xyz += BlockColor * 64.0 * (LightingData.xyz*LightingData.xyz) ; 

		//idea: if specular and roughness < treshhold, consider applying some super basic ambient light.


		//AO: 
		Diffuse.w = pow(min(distance(Origin, Position), 4.0), 1.0);
		Detail.w = Diffuse.w; 
		Detail.xyz = BlockColor * Color; 
		Diffuse.xyz *= Color; 
	}
	else {
		//GI: 
		if(Specular) {
			Diffuse.xyz = textureLod(SkyNoMie, Direction,0.0).xyz; 
		}
		else {
			Diffuse.xyz = textureLod(SkyNoMie, Direction,8.0).xyz; 
		}
		//Diffuse.xyz = vec3(0.0); 
		//AO: 

		vec4 Cloud = SampleCloud(Origin, Direction); 

		Diffuse.xyz = mix(Cloud.xyz, Diffuse.xyz, Cloud.w); 


		Diffuse.w = 4.0; 
		Detail = Diffuse * vec4(0.1,0.1,0.1,1.0); 
		Diffuse.xyz *= Color; 
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
		
		vec2 Xi = hash2(Pixel,FrameCount / 4,Try+3) * vec2(1.0, 0.2); 

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



vec3 SphericalCoordinate(vec2 Angles) {

	float CosX = cos(Angles.x); 

	return vec3( 
		cos(Angles.y) * CosX, 
		sin(Angles.x),
		sin(Angles.y) * CosX); 

}


float Luminance(vec3 x) {
	return dot(x, vec3(0.2126, 0.7152,0.0722)); 
}

vec2 IrridianceToSH(vec3 Radiance, vec3 Direction) {
	
	float Co = Radiance.x - Radiance.z; 
	float T = Radiance.z + Co * 0.5; 
	float Cg = Radiance.y - T;
	float Y  = max(T + Cg * 0.5, 0.0);
	
	float   L00     = 0.282095;
    float   L1_1    = 0.488603 * Direction.y;
    float   L10     = 0.488603 * Direction.z;
    float   L11     = 0.488603 * Direction.x;

	SHy = vec4 (L11, L1_1, L10, L00) * Y; 
	SHy = max(SHy, vec4(-100.0)); 
	return vec2(Co, Cg); 
}

vec4 PackData(vec3 Normal, float Roughness, float Depth) {
	return vec4(Normal * ((1.0-Roughness)*0.5+0.5), Depth); 
}

void main() {

	Rand_Seed = (TexCoord.x * TexCoord.y) * 500.0 * 20.0;

	FragCoord = ivec2(gl_FragCoord); 
	FragCoord.x *= 2; 
	FragCoord.x += int(FragCoord.y % 2 != CheckerStep); 

	ivec2 Pixel = FragCoord * 4 + States[FrameCount % 4] * 2;

	float RawDepthSample = texelFetch(Depth, Pixel, 0).x; 

	Rand_Seed = (Pixel.x * 540 + Pixel.y) * 0.01; 



	vec4 RawNormal = texelFetch(Normals, Pixel, 0); 

	WorldPos = texelFetch(WorldPosition, Pixel, 0).rgb;
	Normal.xyz = RawNormal.rgb;
	Normal.w = LinearDepth(RawDepthSample); 

	float Penum = 0.0; 
	float PenumWeight = 0.0; 

	for(int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {
		
			ivec2 Pixel = FragCoord / 2 + ivec2(x,y); 

			vec2 BlockerInfo = texelFetch(BlockerData, Pixel, 0).xy; 

			float Weight = 1.0 / (1.0 + 100.0 * abs(BlockerInfo.y - Normal.w) * abs(BlockerInfo.y - Normal.w)); 

			Penum += BlockerInfo.x * Weight; 
			PenumWeight += Weight; 
			


		}
	}

	Penum /= PenumWeight; 

	float DirectDensity = SampleCloud(WorldPos.xyz, LightDirection).a; 

	float SmoothNessFactor = clamp(pow(1.0-DirectDensity,2.0) * 20.0, 1.0,8.0); 


	DirectDensity = pow(DirectDensity, 4.0); 
	vec3 Incident = (WorldPos - CameraPosition);
	float IncidentLength = length(Incident); 
	Incident /= IncidentLength; 

	Direct.xyz = DirectHQ(WorldPos,max(Penum,0.007),vec2(Pixel) / 2, SmoothNessFactor) * DirectDensity; 
//	Direct = vec3(1.0); 
	Direct.xyz *= ScreenSpaceTraceShadows(WorldPos + Normal.xyz * mix(0.01,0.1,clamp(IncidentLength/30.0,0.0,1.0)), normalize(LightDirection), 0.75,20, hash(Pixel / 2, FrameCount / 4,32)); 



	vec4 ParallaxData = texelFetch(ParallaxData, Pixel, 0); 
	vec3 TC = texelFetch(TCData, Pixel, 0).xyz; 

	vec3 SunProjected; 



	float TraversalSun = GetTraversal(TC.xy, LightDirection, uint(ParallaxData.x+.1), uint(ParallaxData.y+.1)-1u, SunProjected, ParallaxData.z); 

	float DirectMultiplier = ((TraversalSun+0.00390625) >= ParallaxData.w * (1.0-pow(abs(SunProjected.z),4.0)) ? 1.0 : sqrt(TraversalSun));

	DirectMultiplier = mix(DirectMultiplier, 1.0, pow(clamp(0.05*distance(CameraPosition, WorldPos), 0.0, 1.0),2.0)); 

	Direct *= DirectMultiplier; 


	
	vec2 hash = hash2();

	if (!UseWhiteNoise) {
		hash = hash2(Pixel / 2, FrameCount / 4, 0);
	}

	
	vec4 LowFrequencyNormal = texelFetch(LowFrequencyNormal, Pixel, 0); 

	vec3 Direction = cosWeightedRandomHemisphereDirection(LowFrequencyNormal.xyz, hash);



	


	vec4 Diffuse = GetRayShading(WorldPos + Normal.xyz * 0.025, Direction,Normal.xyz, false, ParallaxData,TC,LowFrequencyNormal.xyz,Detail, Pixel, false); 
	

	vec3 SpecularWorldPos = WorldPos; 
	vec3 SpecularNormal = Normal.xyz; 
	vec3 SpecularDirection = GetSpecularRayDirection(reflect(Incident, Normal.xyz), Normal.xyz, Incident, LowFrequencyNormal.w, Pixel);

	bool Water = false; 

	if(texelFetch(WaterDepth, Pixel, 0).x < RawDepthSample) {
		
		SpecularNormal = texelFetch(WaterNormal, Pixel, 0).xyz; 
		SpecularWorldPos = texelFetch(WaterWorldPos, Pixel, 0).xyz; 

		SpecularDirection = reflect(normalize(WorldPos - CameraPosition), SpecularNormal); 
		Water = true; 
	}


	vec4 Specular = GetRayShading(SpecularWorldPos + SpecularNormal.xyz * 0.025, SpecularDirection, SpecularNormal.xyz, true, ParallaxData,TC,LowFrequencyNormal.xyz, Detail, Pixel, Water); 

	
	//Specular.xyz = abs(reflect(Incident, Normal.xyz)); 
	float L = length(Normal.xyz); 

	if(L < 0.5 || L > 1.5) {
		Diffuse.xyz = vec3(0.0);
		Specular.xyz = vec3(0.0); 
		Direction = vec3(0.0,1.0,0.0); 

	}
	
	IndirectSpecular = Specular; 

	Detail.y = Luminance(Detail.xyz); 
	Detail.x = Luminance(Diffuse.xyz); 
	Detail.z = Detail.x * Detail.x; 
	Detail.y = Specular.w; 

	vec2 SHCoCg = IrridianceToSH(Diffuse.xyz, Direction); 

	IndirectSpecular.w = SHCoCg.x; 
	ShCg = SHCoCg.y; 
	Normal.xyz = LowFrequencyNormal.xyz; 
	
	vec2 RawTC = vec2(Pixel) / vec2(textureSize(TCData, 0).xy); 

	//Direct.xyz = pow(texelFetch(DirectionalCascadesRaw[0], ivec2(RawTC * textureSize(DirectionalCascadesRaw[0],0)),0).xxx,vec3(1000.0)); 
	PackedSpatialData = PackData(Normal.xyz, LowFrequencyNormal.w, LinearDepth(texelFetch(Depth, Pixel, 0).x)); 
}