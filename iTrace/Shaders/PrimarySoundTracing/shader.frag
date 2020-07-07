#version 420
#extension GL_ARB_bindless_texture : enable

layout(location = 0) out float TotalOcclusion;

uniform sampler2D SoundLocations; 
uniform vec3 PlayerPosition; 
uniform sampler3D VoxelData; 
uniform sampler1D VoxelReflectivites; 

vec3 BlockNormals[6] = vec3[](
	vec3(1.0, 0.0, 0.0),
	vec3(-1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, -1.0, 0.0),
	vec3(0.0, 0.0, 1.0),
	vec3(0.0, 0.0, -1.0)
	);

bool RawTrace(vec3 RayDirection, vec3 Origin, inout int Block, inout int Face, inout vec3 Normal, inout vec3 Position, int Steps, float MaxLength) {

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

		if(distance(Position, Origin) >= MaxLength)
			return false; 

		vec3 TexelCoord = (NextPlane - Clamped);

		ivec3 CoordInt = ivec3(TexelCoord.x, TexelCoord.y, TexelCoord.z);

		//	std::cout << CoordInt.x << ' ' << CoordInt.y << ' ' << CoordInt.z << '\n'; 

		if (CoordInt.x > -1 && CoordInt.x < 128 &&
			CoordInt.y > -1 && CoordInt.y < 128 &&
			CoordInt.z > -1 && CoordInt.z < 128) {

			vec3 UVWP = Position - floor(Position);

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

			Block = int(floor(textureLod(VoxelData, TexelCoord.zyx / vec3(128.0),0.0) * 255.0 + .9));
			Face = Side;


			if (Block != 0)
				return true;
		}
		else return false;
	}

	//}



	return false;
}


void main(void) {

	int CurrentRay = int(gl_FragCoord.x); 

	vec3 SoundLocation = texelFetch(SoundLocations, ivec2(CurrentRay, 0), 0).xyz; 

	if(abs(SoundLocation.x) < 0.01 && abs(SoundLocation.y) < 0.01 && abs(SoundLocation.z) < 0.01)
		return; 

	vec3 ToPlayerVector = normalize(PlayerPosition - SoundLocation); 

	vec3 RayOrigin = SoundLocation; 

	int NumberOcclusionHits = 0; 
	bool FirstHit = true; 
	bool ThisHitSolid = false; 
	TotalOcclusion = 0.0; 

	for(int i = 0; i < 10; i++) {
		bool Hit; 
		vec3 HitPosition, Normal; 
		int Block, Face; 

		Hit = RawTrace(normalize(PlayerPosition - RayOrigin), RayOrigin, Block, Face, Normal, HitPosition, 256, distance(PlayerPosition,RayOrigin)); 

		if(Hit) {
			NumberOcclusionHits++; 
			TotalOcclusion += 1.0; 

			RayOrigin = HitPosition + ToPlayerVector * 0.5; 

		}
		else {
			break; 
		}

	}

}
