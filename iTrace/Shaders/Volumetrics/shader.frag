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
uniform sampler2D ProjectedClouds; 
uniform sampler2D BasicBlueNoise; 
uniform sampler2D CloudDepth; 
uniform samplerCube Sky; 
uniform sampler2D Normal; 
uniform sampler2D Wind; 
uniform mat4 IncidentMatrix; 
uniform float Time; 
uniform bool DoVolumetrics; 


uniform sampler2DShadow DirectionalCascades[4]; 
uniform mat4 DirectionMatrices[4]; 
uniform vec3 SunColor; 
uniform vec3 LightDirection; 

uniform vec3 CameraPosition; 
uniform int Frame; 

//the max distance for the volumetrics (useful for the sky!) 
const float VOLUMETRIC_MAX_DISTANCE = 200.0; 
//the steps used for the volumetrics 
const int VOLUMETRIC_STEPS = 16;
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
	 
	return .5 * (1.0 - clamp((WorldPosition.y - 70.0) * 0.02, 0.0, 1.0)) * pow(texture(Wind, (WorldPosition.xz *.001 + Time * .001)).x,2.0); 

	return .3 * pow(texture(Wind, (WorldPosition.xz *.1 + Time * .1) * .05).x,2.0); 
	return 1.0 * (1.0 - clamp((WorldPosition.y - 60.0) * 0.05, 0.0, 1.0)) * texture(Wind, (WorldPosition.xz *.1 + Time * .1)* .01).x; 
}

ivec2 States[] = ivec2[](
	ivec2(1, 1),
	ivec2(0, 1),
	ivec2(0, 0),
	ivec2(1, 0));

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

vec4 SampleCloud(vec3 Origin, vec3 Direction) {
	const vec3 PlayerOrigin = vec3(0,6200,0); 
	const float PlanetRadius = 6373; 

	float Traversal = (PlanetRadius - (PlayerOrigin.y + Origin.y)) / Direction.y; 

	vec3 NewPoint = PlayerOrigin + Origin + Direction * Traversal; 

	//Fetch it! 

	return texture(ProjectedClouds, fract(vec2((NewPoint.x + Time + 500) / 1024, (NewPoint.z + 500) / 1024))); 

}



void main() {
	
	if(!DoVolumetrics) {
		Volumetrics = vec4(0.0,0.0,0.0,1.0); 
		return; 
	}

	float SigmaS = 0.2 * 0.25; 
	float SigmaA = 0.1 * 0.25; 
	float SigmaE = SigmaS + SigmaA; 

	Volumetrics = vec4(0.0,0.0,0.0,1.0); 

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
	if(L < 0.75 || L > 1.25) {
		Vector = normalize(vec3(IncidentMatrix * vec4(TexCoord * 2.0 - 1.0, 1.0, 1.0)));
		ActualDistance = texelFetch(CloudDepth, ivec2(gl_FragCoord.xy), 0).x; 
	}
 
	State = Frame / 4; 
	float StepSize = ActualDistance / float(VOLUMETRIC_STEPS); 

	if(CONSTANT_STEP_SIZE) {
	//	StepSize = VOLUMETRIC_MAX_DISTANCE / float(VOLUMETRIC_STEPS); 
	}

	float Traversal = hash2().x * StepSize; 

	vec3 Position = CameraPosition + Vector * Traversal; 

	while(Traversal < ActualDistance) {
	
		float Density = GetDensity(Position); 

		if(Density > 0.0001) {
		
			float SampleSigmaS = SigmaS * Density; 
			float SampleSigmaE = SigmaE * Density; 

			float DirectDensity = SampleCloud(Position.xyz, LightDirection).a; 

			DirectDensity = pow(DirectDensity, 4.0); 

			vec3 LightFetch = DirectBasic(Position) * SunColor * DirectDensity; 

			

			vec3 S = SampleSigmaS * LightFetch; 

			float Transmittance = exp(-SampleSigmaE * StepSize); 

			vec3 SIntegrated = (S - S * Transmittance) / SampleSigmaE; 

			Volumetrics.xyz += SIntegrated * Volumetrics.a; 
			Volumetrics.a *= Transmittance; 

		}

		Position += Vector * StepSize; 
		Traversal += StepSize; 

	}

}