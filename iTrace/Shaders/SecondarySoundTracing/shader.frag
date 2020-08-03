#version 430 core
#extension GL_ARB_bindless_texture : enable

layout(location = 0) out vec3 GainsShared;
layout(location = 1) out vec3 ReflectivityRatios; 

layout(binding = 0, std140) buffer GainSharedBuffer {
	vec4 GainsSharedData[]; 
};

layout(binding = 1, std140) buffer ReflectivityRatiosBuffer {
	vec4 ReflectivityRatiosData[]; 
};


in vec2 TexCoord; 

uniform sampler2D SoundLocations; 
uniform vec3 PlayerPosition;
uniform int MaxSounds; 
uniform ivec2 PositionBias; 
const int Bounces = 4; 
uniform int NumRays; 

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

		if (CoordInt.x > -1 && CoordInt.x < 384 &&
			CoordInt.y > -1 && CoordInt.y < 128 &&
			CoordInt.z > -1 && CoordInt.z < 384) {

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

			Block = int(floor(texelFetch(VoxelData, ivec3(TexelCoord.zyx),0) * 255.0 + .9));
			Face = Side;

			if (Block != 0) { 
				//Position += vec3(PositionBias.x, 0, PositionBias.y); 
				return true;
			}
		}
		else return false;
	}

	//}



	return false;
}



#define GLOBAL_REFLECTIVITY 0.5


//the rays that are going to be shot out

vec3 rays[] = vec3[](
vec3(0,0,-1),
vec3(-0.169149,-0.154954,-0.973333),
vec3(0.0281697,0.320981,-0.946667),
vec3(0.238458,-0.311027,-0.92),
vec3(-0.442525,0.0782761,-0.893333),
vec3(0.420939,0.267768,-0.866667),
vec3(-0.140858,-0.523984,-0.84),
vec3(-0.268157,0.516315,-0.813333),
vec3(0.579917,-0.211784,-0.786667),
vec3(-0.600754,-0.247982,-0.76),
vec3(0.288157,0.615782,-0.733333),
vec3(0.211758,-0.675115,-0.706667),
vec3(-0.634383,0.367639,-0.68),
vec3(0.739412,0.16256,-0.653333),
vec3(-0.448186,-0.63751,-0.626667),
vec3(-0.102807,0.793367,-0.6),
vec3(0.626496,-0.528008,-0.573333),
vec3(-0.836635,-0.0346043,-0.546667),
vec3(0.605459,0.602511,-0.52),
vec3(-0.0401767,-0.868912,-0.493333),
vec3(-0.56667,0.679049,-0.466667),
vec3(0.889978,-0.119747,-0.44),
vec3(-0.747456,-0.520063,-0.413333),
vec3(0.202403,0.899734,-0.386667),
vec3(0.463844,-0.809475,-0.36),
vec3(-0.898208,0.286551,-0.333333),
vec3(0.864048,0.39922,-0.306667),
vec3(-0.370608,-0.885579,-0.28),
vec3(-0.327427,0.910282,-0.253333),
vec3(0.862146,-0.453129,-0.226667),
vec3(-0.947434,-0.249737,-0.2),
vec3(0.53265,0.828396,-0.173333),
vec3(0.167551,-0.974893,-0.146667),
vec3(-0.784917,0.60787,-0.12),
vec3(0.992234,0.0822209,-0.0933333),
vec3(-0.677553,-0.732446,-0.0666667),
vec3(0.0048786,0.999188,-0.04),
vec3(0.671826,-0.740589,-0.0133333),
vec3(-0.995645,0.0922704,0.0133333),
vec3(0.795916,0.604085,0.04),
vec3(-0.17857,-0.981666,0.0666667),
vec3(-0.530298,0.842658,0.0933334),
vec3(0.957467,-0.262407,0.12),
vec3(-0.880065,-0.451635,0.146667),
vec3(0.34235,0.923446,0.173333),
vec3(0.369434,-0.907479,0.2),
vec3(-0.880142,0.4171,0.226667),
vec3(0.924434,0.285033,0.253333),
vec3(-0.485402,-0.828242,0.28),
vec3(-0.199836,0.930603,0.306667),
vec3(0.768625,-0.54599,0.333333),
vec3(-0.925786,-0.115416,0.36),
vec3(0.597745,0.702275,0.386667),
vec3(0.0331742,-0.909975,0.413333),
vec3(-0.630332,0.639594,0.44),
vec3(0.883279,-0.0451639,0.466667),
vec3(-0.670541,-0.554073,0.493333),
vec3(0.118022,0.845973,0.52),
vec3(0.474867,-0.689679,0.546667),
vec3(-0.798461,0.183708,0.573333),
vec3(0.696052,0.394349,0.6),
vec3(-0.240451,-0.741264,0.626667),
vec3(-0.314178,0.688802,0.653333),
vec3(0.674991,-0.286335,0.68),
vec3(-0.666947,-0.236229,0.706667),
vec3(0.319236,0.600259,0.733333),
vec3(0.162606,-0.629253,0.76),
vec3(-0.517657,0.336432,0.786667),
vec3(0.573863,0.0957631,0.813333),
vec3(-0.334313,-0.427358,0.84),
vec3(-0.038786,0.497378,0.866667),
vec3(0.328397,-0.306775,0.893333),
vec3(-0.3919,0.00382704,0.92),
vec3(0.239697,0.215331,0.946667),
vec3(-0.0222833,-0.22831,0.973333)
);

void MainPositional() {
	int CurrentRay = int(gl_FragCoord.x); 
	int CurrentSubray = int(gl_FragCoord.y); 

	vec3 SoundLocation = texelFetch(SoundLocations, ivec2(CurrentRay, 0), 0).xyz; 
	SoundLocation = PlayerPosition - vec3(0.0,1.5,0.0); 


	GainsShared = vec3(0.); 
	ReflectivityRatios = vec3(0.); 

	//if(abs(SoundLocation.x) < 0.01 && abs(SoundLocation.y) < 0.01 && abs(SoundLocation.z) < 0.01)
	//	return; 

	vec3 ToPlayerVector = normalize(PlayerPosition - SoundLocation); 

	vec3 RayOrigin = SoundLocation + ToPlayerVector * 0.05; 



	int NumberOcclusionHits = 0; 
	bool FirstHit = true; 
	bool ThisHitSolid = false; 
	float TotalOcclusion = 0.0; 
	float BounceReflectivityRatio = 0.0; 
	float RayLengthSum = 0.0;
	float RayLengthEnerySum = 0.0; 
	int NumRaysHit = 0; 
	float SecondaryRayLengthSum = 0.0; 
	int SecondaryRayHit = 0; 
	int RayHits = 0; 
	float SharedAirSpace = 0.0; 
	float SendGain0 = 0.0; 
	float SendGain1 = 0.0; 
	float SendGain2 = 0.0; 
	float SendGain3 = 0.0;
	float OneOverTotalRays = 1.0 / (NumRays*Bounces); 
	

	vec3 RayDir = rays[CurrentSubray]; 
	//RayDir = vec3(0.0,-1.0,0.0); 

	bool Hit;
	vec3 HitLocation, Normal, HitBlockLocation; 


	int Block, Face; 

	Hit = RawTrace(RayDir,RayOrigin - vec3(PositionBias.x, 0, PositionBias.y), Block, Face, Normal, HitLocation, 256,10000.0); 
	
	HitLocation +=  vec3(PositionBias.x, 0, PositionBias.y); 

	vec4 BounceReflectivityRatios = vec4(0.); 

	if(Hit) {
		
		float RayLength = distance(SoundLocation, HitLocation); 
		RayLengthEnerySum += 1.0 / float(NumRays); 
		RayLengthSum += 1.0; 
		NumRaysHit++; 

		vec3 LastPosHit = HitLocation + Normal * 0.2; 
		vec3 LastHitNormal = Normal; 
		vec3 LastRayDir = RayDir; 

		float TotalRayDistance = RayLength; 

		for(int Bounce = 0; Bounce < Bounces; Bounce++) {

			
			vec3 NewRayDir = normalize(reflect(LastRayDir, LastHitNormal)); 
			vec3 NewOrigin = LastPosHit; 

			Hit = RawTrace(NewRayDir, NewOrigin- vec3(PositionBias.x, 0, PositionBias.y), Block, Face, Normal, HitLocation, 256, 10000.0); 

			HitLocation += vec3(PositionBias.x, 0, PositionBias.y); 

			float EnergyTowardsPlayer = 0.5 * (GLOBAL_REFLECTIVITY *.5+.5); 
			float BouncePlayerDistance = distance(LastPosHit, PlayerPosition); 

			if(Hit) {
				

				float NewRayLength = distance(LastPosHit, HitLocation); 
				
				SecondaryRayLengthSum += NewRayLength; 
				SecondaryRayHit++; 

				RayHits++; 

				BounceReflectivityRatios[Bounce] += GLOBAL_REFLECTIVITY; 

				TotalRayDistance += NewRayLength; 

				LastPosHit = HitLocation; 
				LastHitNormal = Normal; 
				LastRayDir = NewRayDir; 

				//cast one ray towards the player to figure out if we share airspace 

				vec3 FinalRayStart = HitLocation + Normal * 0.1; 
				vec3 FinalRayDirection = normalize(PlayerPosition - HitLocation); 
		
			
				bool NewHit = RawTrace(FinalRayDirection, FinalRayStart - vec3(PositionBias.x, 0, PositionBias.y), Block, Face, Normal, HitLocation, 256,distance(FinalRayStart, PlayerPosition)); 

				if(!NewHit) 
					SharedAirSpace += 1.0; 

				HitLocation += vec3(PositionBias.x, 0, PositionBias.y); 


			}
			else {
				TotalRayDistance += distance(LastPosHit, PlayerPosition); 
			}

			float ReflectionDelay = max(TotalRayDistance,0.) * 0.7 * GLOBAL_REFLECTIVITY; 
			
			float Cross0 = 1.0 - clamp(abs(ReflectionDelay),0.0,1.0); 
			float Cross1 = 1.0 - clamp(abs(ReflectionDelay - 2.0),0.0,1.0); 
			float Cross2 = 1.0 - clamp(abs(ReflectionDelay - 4.0),0.0,1.0); 
			float Cross3 = 1.0 - clamp(ReflectionDelay - 8.0,0.0,1.0); 

			SendGain0 += Cross0 * OneOverTotalRays * 6.4f * EnergyTowardsPlayer;
			SendGain1 += Cross1 * OneOverTotalRays * 12.8 * EnergyTowardsPlayer; 
			SendGain2 += Cross2 * OneOverTotalRays * 12.8 * EnergyTowardsPlayer; 
			SendGain3 += Cross3 * OneOverTotalRays * 25.6 * EnergyTowardsPlayer;  


			if(!Hit)
				break; 

		}


	}



	GainsShared.x = uintBitsToFloat(packHalf2x16(vec2(SendGain0, SendGain1))); 
	GainsShared.y = uintBitsToFloat(packHalf2x16(vec2(SendGain2, SendGain3)));
	GainsShared.z = SharedAirSpace; 

	for(int i = 0; i < Bounces/2;i++) 
		ReflectivityRatios[i] = uintBitsToFloat(packHalf2x16(vec2(BounceReflectivityRatios[i*2], BounceReflectivityRatios[i*2+1]))); 

}

void MainAmbience() {
	


	GainsShared = vec3(0.0); 
	ReflectivityRatios = vec3(0.0); 

	int CurrentSubray = int(gl_FragCoord.y); 

	vec3 RayDirection = rays[CurrentSubray]; 

	//make sure its always pointing upwards! 

	RayDirection.y = abs(RayDirection.y); 


	vec3 Origin = PlayerPosition; 
	GainsShared.z = 0.0; 
	for(int i = 0; i < 10; i++) {
		
		bool Hit; 
		vec3 HitPosition, Normal; 
		int Block, Face; 

		Hit = RawTrace(RayDirection, Origin - vec3(PositionBias.x, 0, PositionBias.y), Block, Face, Normal, HitPosition, 256, 10000.0); 

		if(Hit) {

			GainsShared.z += 0.5; 
			Origin = HitPosition + RayDirection * 0.1 +  vec3(PositionBias.x, 0, PositionBias.y); 

		}
		else {
			break; 
		}

	}


}


void main(void) {
	int CurrentRay = int(gl_FragCoord.x); 
	int CurrentSubray = int(gl_FragCoord.y); 

	int Index = CurrentRay * NumRays + CurrentSubray; 
	
	if(CurrentRay == 0) 
		MainAmbience(); 
	else 
		MainPositional(); 


	ReflectivityRatiosData[Index].xyz = ReflectivityRatios; 
	GainsSharedData[Index].xyz = GainsShared; 
}
