#version 330

in vec2 TexCoord; 
layout(location = 0) out vec4 Clouds;
layout(location = 1) out float Depth; 

uniform sampler2D BasicBlueNoise; 
uniform sampler2D WeatherMap; 

uniform sampler3D CloudNoise; 
uniform mat4 IncidentMatrix; 

uniform vec3 LightDirection; 
uniform vec3 SunColor; 
uniform vec3 AmbientColor; 
uniform vec3 CameraPosition; 


uniform float Time; 

uniform int SubFrame; 
uniform int Frame; 

uniform ivec2 TextureSize; 


ivec2 States[] = ivec2[](
	ivec2(1, 1),
	ivec2(0, 1),
	ivec2(0, 0),
	ivec2(1, 0));

int State = 0;

ivec2 Pixel; 

const vec3 PlayerOrigin = vec3(0,6200,0); 
const float PlanetRadius = 6373; 
const float AtmosphereRadius = 6573; 
const float Size = AtmosphereRadius - PlanetRadius; 
const int Steps = 8; 
const int LightSteps = 8; 
const float Epsilon = 1e-4; 

//Cloud properties 

const float SigmaS = 0.2 * 200; 
const float SigmaA = 0.2 * 200; 
const vec3 CloudColor = vec3(1.0); //<- is this really physically plausible? 

vec2 hash2() {
	return clamp(texelFetch(BasicBlueNoise, ivec2(Pixel + ivec2((State++) * 217))%256, 0).xy,0.0,0.99); 
}


//Gets the density at a single point

float GetDensity(vec3 P, float T, float Height) {
	 
	vec2 NewPDetail = P.xz + vec2(Time * 1, 0.0); 
	vec3 NewP = P += vec3(Time * .5,0.0,0.0); 

	vec4 DensityGrab = texture(CloudNoise, NewP / 128 ); 
	vec4 WeatherMap = texture(WeatherMap, (NewPDetail + 250.0)/ 1000); 




	return 1 * pow(DensityGrab.x * DensityGrab.x * DensityGrab.x  * DensityGrab.x  * DensityGrab.x  * DensityGrab.x * DensityGrab.y *  DensityGrab.y *  DensityGrab.y * DensityGrab.y  * DensityGrab.z * DensityGrab.z * DensityGrab.w *  DensityGrab.w *  DensityGrab.w * DensityGrab.w * DensityGrab.w,7.0) * pow(WeatherMap.x,5.0) * pow(abs((WeatherMap.x*0.5+0.3)-Height/Size),8.0); 
}


float GetSunShading(vec3 Point, float SigmaA, vec3 Direction, float Dither) {

	float End = (AtmosphereRadius - Point.y) / Direction.y;  
	End = min(End, 100.0); 
	vec3 StartPosition = Point; 
	vec3 EndPosition = Point + Direction * End; 

	float PreviousTraversal = 0.0; 

	float ReturnTransmittance = 1.0;

	for(int Step = 0; Step < LightSteps; Step++) {

		if(ReturnTransmittance < 0.01) 
			break; 
	
		float t = float(Step + Dither) / float(LightSteps); 

		vec3 Position = mix(StartPosition, EndPosition, t*t); 

		float Traversal = mix(0.0, End, t*t); 
		
		float StepSize = Traversal - PreviousTraversal; 

		//Grab the density at this point 

		float Height = Traversal * LightDirection.y; 

		float Density = GetDensity(Position,t*t,Height); 

		//is there a cloud? 

		if(Density > Epsilon) {
			float SampleSigmaE = SigmaA * Density; 

			float Transmittance = exp(-SampleSigmaE * StepSize); 

			ReturnTransmittance *= Transmittance; 

		}

		PreviousTraversal = Traversal; 

	}

	return ReturnTransmittance ; 

}


void main() {
	
	float SigmaE = SigmaS + SigmaA; 

	vec2 TexelSize = 1 / vec2(TextureSize); 

	ivec2 RawPixel = ivec2(gl_FragCoord.xy); 
	Pixel = RawPixel * 2 + States[SubFrame]; 
	vec2 NewTexCoord = vec2(Pixel) * TexelSize;  

	vec3 Direction = normalize(vec3(IncidentMatrix * vec4(NewTexCoord * 2.0 - 1.0, 1.0, 1.0)));

	//assume player is never in the atmosphere -> 

	//float Start = intersectSphere(PlayerOrigin,Direction, PlanetRadius); 
	//float End = intersectSphere(PlayerOrigin,Direction, AtmosphereRadius); 

	vec3 Origin = PlayerOrigin + CameraPosition; 

	float Start = (PlanetRadius - Origin.y) / Direction.y;  
	float End = (AtmosphereRadius - Origin.y) / Direction.y;  


	if(Start > End) {
		float temp = End; 
		End = Start; 
		Start = temp; 
	}
	Start = max(Start, 0.0); 
	
	Clouds = vec4(0.0,0.0,0.0,1.0);

	if(End < 0.0)  
		return; 
	
	float T = End-Start; 
	T = min(T, 300.0); 
	End = Start + T; 

	vec3 StartPosition = Origin + Direction * Start; 
	vec3 EndPosition = Origin + Direction * End; 

	float PreviousTraversal = 0.0; 
	State = Frame / 4; 
	vec2 hash = hash2(); 
	

	float DitherStart = hash.x; 

	Depth = 0.0; 
	float DepthWeight = 0.0; 

	for(int Step = 0; Step < Steps; Step++) {
	
		float t = float(Step+DitherStart) / float(Steps); 

		vec3 Position = mix(StartPosition, EndPosition, t); 
	
		float Traversal = mix(0.0, End-Start, t); 
		
		float StepSize = Traversal - PreviousTraversal; 

		//Grab the density at this point 

		float Height = Traversal * Direction.y; 

		float Density = GetDensity(Position,t,Height); 

		//is there a cloud? 

		if(Density > Epsilon) {
			
			float Mix = Height / Size; 

			float SampleSigmaS = SigmaS * Density; 
			float SampleSigmaE = SigmaE * Density; 

			vec3 BottomColor = mix(AmbientColor,vec3(2.0),0.25) * 0.5 * min((1.0-Density)*0.8+1.2,2.0); 
			vec3 TopColor = vec3(2.0); 

			vec3 AmbientColor = mix(TopColor,BottomColor, pow((1-Mix),3.0)); 

			vec3 LightingData =  CloudColor * AmbientColor * 2.0 + SunColor * 0.25 * GetSunShading(Position, (SigmaS  + SigmaA) * 0.0625, LightDirection, hash.y); //<- todo: add internal sun tracing  
			//Ambient color should probably be height based! 

			vec3 S = LightingData * SampleSigmaS; //<- for now, exclude the phase function 

			float Transmittance = exp(-SampleSigmaE * StepSize); 

			vec3 SIntegrated = (S - S * Transmittance) / SampleSigmaE; 

			Clouds.xyz += SIntegrated * Clouds.a; 
			Clouds.a *= Transmittance; 
			Depth += (Traversal + Start) * (Density / Epsilon); 
			DepthWeight += (Density / Epsilon); 
		}

		PreviousTraversal = Traversal; 

	}
	Depth = mix(Depth / max(DepthWeight,1e-4),End,pow(Clouds.a,12.0)); 
}