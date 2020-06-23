#version 330
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord; 
layout(location = 0) out vec4 Volumetrics;


//some optimization techniques 
//push volumetrics to spatial filter 
//utilize a dither matrix (is that the term?) for hemispherical lighting 
//as sampling 48 shadow maps per fetch is just too expensive 

//The dither matrix should be 4x4 (as 48 is divisble by 4^2 16) 

uniform sampler2D WorldPosition; 
uniform sampler2D WindNoise; 
uniform sampler3D ChunkLighting; //used for volumetrics from player-based light sources
uniform sampler2D BasicBlueNoise; 
uniform sampler2DArrayShadow HemisphericalShadowMap; 
uniform samplerCube Sky; 
uniform sampler2D Normal; 
uniform sampler2D Wind; 
uniform mat4 IncidentMatrix; 
uniform float Time; 
uniform bool DoVolumetrics; 

uniform mat4 HemisphericalMatrices[48]; 
uniform vec3 HemisphericalDirections[48]; 

uniform sampler2DShadow DirectionalCascades[4]; 
uniform mat4 DirectionMatrices[4]; 
uniform vec3 SunColor; 
uniform vec3 LightDirection; 

uniform vec3 CameraPosition; 
uniform int Frame; 

//the max distance for the volumetrics (useful for the sky!) 
const float VOLUMETRIC_MAX_DISTANCE = 100.0; 
//the steps used for the volumetrics 
const int VOLUMETRIC_STEPS = 1;
//should step count or step size be constant 
const bool CONSTANT_STEP_SIZE = false;
//use blue-noise based dithering 
const bool USE_DITHERING = true; 


int State = 0;

ivec2 Pixel; 

vec2 hash2() {
	return clamp(texelFetch(BasicBlueNoise, ivec2(Pixel + ivec2((State++) * 217))%256, 0).xy,0.0,0.99); 
}




float PhaseFunction() {
	return 1.0 / (4.0 * 3.1415); 
}

float GetDensity(vec3 WorldPosition) {
	//return 1.0; 
	return 1.0 * (1.0 - clamp((WorldPosition.y - 40.0) * 0.05, 0.0, 1.0)) * texture(Wind, (WorldPosition.xz *.1 + Time * .1)* .01).x; 
}

ivec2 States[] = ivec2[](
	ivec2(1, 1),
	ivec2(0, 1),
	ivec2(0, 0),
	ivec2(1, 0));

vec3 GetHemisphericalShadowMaphit(vec3 WorldPos) {
	
	ivec2 PixelShift = ivec2(gl_FragCoord) % ivec2(2); 

	int Dither = ((PixelShift.x * 2 + PixelShift.y + Frame/4) % 4) * 4 ;

	
	vec3 Result = vec3(0.0); 

	for(int i = 0; i < 12; i++) {

		int Sample = Dither + i * 4; 

		vec4 ClipSpace = HemisphericalMatrices[Sample+i] * vec4(WorldPos, 1.0); 

		ClipSpace.xyz /= ClipSpace.w; 

		ClipSpace.xyz = ClipSpace.xyz * 0.5 + 0.5; 


		vec3 Direction = HemisphericalDirections[Sample+i]; 

		float Weight = 4.0 * max(Direction.y, 0.0); 

		Result += texture(Sky, Direction).xyz * texture(HemisphericalShadowMap, vec4(ClipSpace.xy,Sample+i,ClipSpace.z-0.00009)) * Weight; 

	}

	return Result / 12.0; 

}




vec3 GetHemisphericalShadowMaphit2(vec3 WorldPos) {
	
	ivec2 PixelShift = ivec2(gl_FragCoord) % ivec2(2); 

	int Dither = (PixelShift.x * 2 + PixelShift.y) * 3;

	
	vec3 Result = vec3(0.0); 

	for(int i = 0; i < 3; i++) {

		int Sample = Dither + i; 

		vec4 ClipSpace = HemisphericalMatrices[Sample+i] * vec4(WorldPos, 1.0); 

		ClipSpace.xyz /= ClipSpace.w; 

		ClipSpace.xyz = ClipSpace.xyz * 0.5 + 0.5; 


		vec3 Direction = HemisphericalDirections[Sample+i]; 

		float Weight = 4.0 * max(Direction.y, 0.0); 



		Result += texture(Sky, Direction).xyz * texture(HemisphericalShadowMap, vec4(ClipSpace.xy,Sample+i,ClipSpace.z-0.00009)) * Weight; 

	}

	return Result / 3.0; 

}


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


void main() {
	
	if(!DoVolumetrics) {
		Volumetrics = vec4(0.0,0.0,0.0,1.0); 
		return; 
	}

	float SigmaS = 0.2 * 0.25; 
	float SigmaA = 0.06667 * 0.25; 
	float SigmaE = SigmaS + SigmaA; 

	vec4 Volumetric = vec4(0.0,0.0,0.0,1.0); 

	int SubFrame = Frame % 4; 

	ivec2 Offset = States[0]; 

	if(SubFrame == 3) {
		Offset = States[1]; 
	}
	else if(SubFrame == 1) {
		Offset = States[3]; 
	}
	else if(SubFrame == 0) {
		Offset = States[2]; 
	}

	Pixel = ivec2(gl_FragCoord.xy) * 2 + Offset; 
	
	vec3 WorldPosSample = texelFetch(WorldPosition, Pixel * 2, 0).xyz; 

	vec3 Vector = WorldPosSample - CameraPosition; 

	float ActualDistance = length(Vector); 

	vec3 NormalSample = texelFetch(Normal, Pixel * 2, 0).xyz; 

	Vector /= ActualDistance; 

	float L = length(NormalSample); 
	ActualDistance = min(ActualDistance, VOLUMETRIC_MAX_DISTANCE); 
	if(L < 0.75 || L > 1.25) {
		Vector = normalize(vec3(IncidentMatrix * vec4(TexCoord * 2.0 - 1.0, 1.0, 1.0)));
		ActualDistance = VOLUMETRIC_MAX_DISTANCE; 
	}

	//State = ((Frame / 4) * 7195); 
		State = 0; 
	float StepSize = ActualDistance / float(VOLUMETRIC_STEPS); 

	if(CONSTANT_STEP_SIZE) {
		StepSize = VOLUMETRIC_MAX_DISTANCE / float(VOLUMETRIC_STEPS); 
	}

	float Traversal = hash2().x * StepSize; 

	vec3 Position = CameraPosition + Vector * Traversal; 

	while(Traversal < ActualDistance) {
		
		float Density = GetDensity(Position); 

		if(Density > 0.001) {
		
			float SampleSigmaS = SigmaS * Density; 
			float SampleSigmaE = SigmaE * Density; 
		
			//grab lighting at current point 

			vec3 LightingData = (textureLod(ChunkLighting, (Position).zyx / 128.0, 0.0).xyz * 10.0 + GetHemisphericalShadowMaphit(Position) * 2.0 + DirectBasic(Position) * SunColor); 

			vec3 S = LightingData * SampleSigmaS * PhaseFunction(); 

			float Tr = exp(-SampleSigmaE * StepSize); 

			vec3 SIntegrated = (S - S * Tr) / SampleSigmaE; 

			Volumetric.xyz += SIntegrated * Volumetric.a; 
			Volumetric.a *= Tr; 
		
		}


		Position += Vector * StepSize; 
		Traversal += StepSize; 
	}

	Volumetrics = Volumetric; 
}