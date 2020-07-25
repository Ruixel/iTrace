#version 330

in vec2 TexCoord; 
layout(location = 0) out vec4 Clouds;

uniform sampler2D BasicBlueNoise; 
uniform sampler2D WeatherMap; 
uniform sampler2D PreviousCloudResult; 
uniform sampler2D Turbulence; 

uniform sampler3D CloudNoise; 
uniform sampler3D CloudShape; 


uniform vec3 LightDirection; 
uniform vec3 SunColor; 
uniform vec3 SkyColor; 
uniform vec3 AmbientColor; 


uniform float Time; 

uniform int SubFrame; 
uniform int Frame; 

int State = 0;

ivec2 Pixel; 

const vec3 PlayerOrigin = vec3(0,6200,0); 
const float PlanetRadius = 6373; 
const float AtmosphereRadius = 6573; 
const float Size = AtmosphereRadius - PlanetRadius; 
const int Steps = 4; 
const int LightSteps = 1; 
const float Epsilon = 1e-9; 

//Cloud properties 

const float SigmaS = 0.2 * 200 * 3.0; 
const float SigmaA = 0.2 * 50 * 3.0; 
const vec3 CloudColor = vec3(1.0); //<- is this really physically plausible? 

vec2 hash2() {
	return clamp(texelFetch(BasicBlueNoise, ivec2(Pixel + ivec2((State++) * 217))%256, 0).xy,0.0,0.99); 
}

float henyeyGreenstein(float cosAngle, float eccentricity) {
    float eccentricity2 = eccentricity*eccentricity;
    return ((1.0 - eccentricity2) / pow(1.0 + eccentricity2 - 2.0*eccentricity*cosAngle, 3.0/2.0)) / (4*3.141592);
}

float remap(in float val, in float inMin, in float inMax, in float outMin, in float outMax) {
    return (val - inMin)/(inMax - inMin) * (outMax - outMin) + outMin;
}
// clamps the input value to (inMin, inMax) and performs a remap
float clampRemap(in float val, in float inMin, in float inMax, in float outMin, in float outMax) {
    float clVal = clamp(val, inMin, inMax);
    return (clVal - inMin)/(inMax - inMin) * (outMax - outMin) + outMin;
}

uniform float ScatteringMultiplier; 
uniform float AbsorptionMultiplier; 

uniform float GlobalPower; 
uniform float DetailPower; 
uniform float NoisePower; 


float Density(vec3 Position) {
	
	Position.xz = Position.xz + vec2 (500); 

	Position.y -= PlanetRadius; 

	Position.x += Time * 6.0; 

	vec3 WeatherSample = texture(WeatherMap, (Position.xz) / 8192).xyz; 

	float WeatherNoise =  (1.0-pow(1.0-WeatherSample.x,3.0)); ; 

	if(WeatherNoise < 1.0/256.0)
		return 0.0; 

	vec4 NoiseFetch1 = texture(CloudNoise, ((Position + vec3(0.0,-Time*10,0.0)) * vec3(1.0,1,1.0)) / 4096); 
	
	float HighFrequencyNoise = dot(NoiseFetch1.yzw, vec3(0.25,0.125,0.625)); 

	float ShapeNoise2 = remap(NoiseFetch1.x, HighFrequencyNoise, 1.0, 0.0, 1.0); 

	if(ShapeNoise2 < 1.0/256.0)
		return 0.0; 
	
	

	float ShapeNoise = dot(vec2(NoiseFetch1.x, HighFrequencyNoise), vec2(0.4,0.6)); 


	float HeightSignal = Position.y / Size; 
	float AnvilSignal = HeightSignal-0.7; 
	if(AnvilSignal < 0.0) {
		AnvilSignal = max(-AnvilSignal-0.2,0.0) / 0.5; 
	}
	else {
		AnvilSignal = AnvilSignal / 0.3; 
	}

	float HeightShape = 1.0-pow(1.0-HeightSignal,5.0); 

	float BaseShape = (ShapeNoise)  * WeatherNoise * HeightShape; 
	//BaseShape = pow(BaseShape,1.5); 



	BaseShape = pow(BaseShape,0.0625 *8.0 *GlobalPower * (.4+16.0*pow(HeightSignal,8.0))); 

	if(BaseShape < 1.0/25.0)
		return 0.; 

	vec4 NoiseFetch2 = texture(CloudShape, (Position + vec3(0.0,Time*2,0.0)) / 384); 

	float ErosionNoise = dot(NoiseFetch2.xyz, vec3(0.25,0.125,0.625)); 

	float ErosionNoiseHighFrequency = dot(NoiseFetch2.yz, vec2(0.3,0.7)); 
	float ErosionNoise2 = remap(NoiseFetch2.x, ErosionNoiseHighFrequency, 1.0, 0.0, 1.0); 


	//BaseShape -= min(1.0,1.0) * pow((1.0-min(BaseShape*1.9,1.0)),3.0);
	BaseShape -= 30.0*pow(ErosionNoise,5.0) * pow((1.0-min(BaseShape*1.3,1.0)),7.0); 
	BaseShape = clamp(BaseShape, 0.0, 1.0); 
	return 0.0006*BaseShape*2.0; 

}

float GetLodDensity(vec3 Position, float Offset) {

	return (
		Density(Position + vec3(Offset, 0.0,0.0))/ 0.0006 + 
		Density(Position + vec3(-Offset, 0.0,0.0))/ 0.0006 + 
		Density(Position + vec3(0.0, Offset,0.0))/ 0.0006 + 
		Density(Position + vec3(0.0, -Offset,0.0))/ 0.0006 + 
		Density(Position + vec3(0.0, 0.0,Offset))/ 0.0006 + 
		Density(Position + vec3(0.0, 0.0,-Offset))/ 0.0006
	) / 6.0; 

}

float InnerScatter(float InnerScatteringDepth, float Height) {
	float depthProbability = 0.05 + pow(InnerScatteringDepth, clampRemap(Height, 0.3, 0.85, 0.5, 2.0));
    // relax the attenuation over height
    float verticalProbability = pow(clampRemap(Height, 0.07, 0.14, 0.1, 1.0), 0.8);
    // both of those effects model the in-scatter probability
    float inScatterProbability = depthProbability * verticalProbability;
    return inScatterProbability;
}

float densityToLight(vec3 Point, vec3 Direction, float Dither) {
    float End = (AtmosphereRadius - Point.y) / Direction.y;  
	End = min(End, 2000.0); 
	vec3 StartPosition = Point; 
	vec3 EndPosition = Point + Direction * End; 

	float PreviousTraversal = 0.0; 

	float ReturnTransmittance = 1.0;

	float Accum = 0.0; 

	for(int Step = 0; Step < LightSteps; Step++) {

		if(ReturnTransmittance < 0.0001) 
			break; 
	
		float t = float(Step + Dither) / float(LightSteps); 

		vec3 Position = mix(StartPosition, EndPosition, t); 

		float Traversal = mix(0.0, End, t); 
		
		float StepSize = Traversal - PreviousTraversal; 

		//Grab the density at this point 

		float Height = Traversal * LightDirection.y; 

		Accum+= 13.0 * Density(Position) / 0.0006; 

		
		PreviousTraversal = Traversal; 

	}

	return Accum / float(LightSteps) ; 
}



float beerLambert(float d) {

    return max(exp(-d),0.47*exp(-d*.34));
}

float beerLambertPowder(float d) {
    return max(exp(-d),0.3*exp(-d*.45));
}

vec3 Radiance(vec3 Position, float Height, float SilverIntensity, float SilverSpread, float Hash, vec3 LightDir, float VdL) {

	float eccentricity = 0.0;
	
	float HG1 = henyeyGreenstein(VdL, eccentricity);
    float HG2 = henyeyGreenstein(VdL, 0.99 - SilverSpread);
    float hg = max(HG1, SilverIntensity*HG2);

	float d = densityToLight(Position, LightDir, Hash) ;
    float bl = beerLambert(d*1.4);

	float InnerScatteringDensity = GetLodDensity(Position, 5.0+Hash*100.0); 
	float InnerScatter = InnerScatter(InnerScatteringDensity, Height); 

	float innerpowder = 1-beerLambertPowder(d*0.1);
	float outerpowder = 1-beerLambertPowder(d*0.5); 
	float hpowder = 1.0-pow(1.0-Height,2.5); 

	vec3 sunLight = SunColor * 0.2 * (bl * hg);
   
	InnerScatter = clamp(InnerScatter,0.0,1.0); 


	float Powder = dot(vec2(outerpowder,innerpowder),vec2(0.3,0.7)); 
	float PowderScatter = Powder * InnerScatter; 
	float Scatter = InnerScatter; 

	float Mixed = dot(vec3(Powder*Powder*1.0, PowderScatter, Scatter), vec3(0.5,0.3,0.7)); 

	float InversePowder = 1.0 - clamp(2.0 * Powder*Powder,0.0,0.8); 

	vec3 ambientLight = InversePowder * mix(mix(SkyColor.zzz*0.8,SkyColor*1.3,0.5),SkyColor*1.3,1.0-pow(clamp(InnerScatter,0.0,1.0),2.0));

	ambientLight = mix(InversePowder*SkyColor,ambientLight,clamp(100.0*bl * hg,0.7,1.0)); 


	return ambientLight * Mixed * 1.5 + sunLight; 

	return mix(sunLight, ambientLight, 1-Height);

}

void main() {
	
	float SigmaE = SigmaS * ScatteringMultiplier+ SigmaA * AbsorptionMultiplier; 

	ivec2 RawPixel = ivec2(gl_FragCoord.xy); 
	Pixel = RawPixel; 

	//assume player is never in the atmosphere -> 

	//float Start = intersectSphere(PlayerOrigin,Direction, PlanetRadius); 
	//float End = intersectSphere(PlayerOrigin,Direction, AtmosphereRadius); 

	vec3 Origin = vec3(TexCoord.x * 8192.0 - 4096, PlanetRadius, TexCoord.y * 8192.0 -4096); 
	vec3 Direction = vec3(0.0,1.0,0.0); 


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
	End = Start + T; 

	vec3 StartPosition = Origin + Direction * Start; 
	vec3 EndPosition = Origin + Direction * End; 

	float PreviousTraversal = 0.0; 
	State = Frame / 4; 
	vec2 hash = hash2(); 
	

	float DitherStart = hash.x; 

	float ndl = dot(Direction, LightDirection); 

	for(int Step = 0; Step < Steps; Step++) {
	
		float t = float(Step+DitherStart) / float(Steps); 

		vec3 Position = mix(StartPosition, EndPosition, t); 
	
		float Traversal = mix(0.0, End-Start, t); 
		
		float StepSize = Traversal - PreviousTraversal; 

		//Grab the density at this point 

		float Height = Traversal * Direction.y; 


		float Density = Density(Position); 

		//is there a cloud? 

		if(Density / 0.0006 > 0.1) {
			
			float Mix = Height / Size; 

			float SampleSigmaS = SigmaS * Density * ScatteringMultiplier; 
			float SampleSigmaE = SigmaE * Density; 

			vec3 BottomColor = mix(AmbientColor,vec3(2.0),0.25) * 0.5 * min((1.0-Density)*0.8+1.2,2.0); 
			vec3 TopColor = vec3(2.0); 

			vec3 AmbientColor = mix(TopColor,BottomColor, pow((1-Mix),3.0)); 

			vec3 LightingData =  Radiance(Position, (Height) / Size, 3.0, 0.8, hash.y, LightDirection, ndl);  
			//Ambient color should probably be height based! 

			vec3 S = LightingData * SampleSigmaS; //<- for now, exclude the phase function 

			float Transmittance = exp(-SampleSigmaE * StepSize); 

			vec3 SIntegrated = (S - S * Transmittance) / SampleSigmaE; 

			Clouds.xyz += SIntegrated * Clouds.a; 
			Clouds.a *= Transmittance; 
		}

		PreviousTraversal = Traversal; 

	}


	float MixFactor = min(float(Frame) / float(Frame+1), 0.975); 

	Clouds = mix(Clouds, texture(PreviousCloudResult, TexCoord), MixFactor); 
	Clouds = clamp(Clouds, vec4(0.0), vec4(100.0)); 
}
/*

float SampleCirrus(vec3 Position) { //gets cirrus density -> 
	return 1.0-pow(texture(Cirrus, Position.xz * 0.0021).x, 0.25 * GlobalPower); 
}

vec4 SampleCirrus(vec4 CurrentCloud, vec3 Position, float Hash, float SilverIntensity, float SilverSpread, float VdL) {
	
	float BaseDensity = SampleCirrus(Position); 

	//light traversal -> 

	const int CirrusLightSteps = 2; 

	float TotalDensity = 0.0; 
	
	float Step = 20.0 / float(CirrusLightSteps); 

	vec3 Pos = Position - LightDirection * Step * Hash; 

	for(int x = 0; x < CirrusLightSteps; x++) {
		TotalDensity += 7.0 * SampleCirrus(Pos); 

		Pos += -LightDirection * Step; 

	}

	TotalDensity /= float(CirrusLightSteps); 

	float eccentricity = 0.0;
	
	float HG1 = henyeyGreenstein(VdL, eccentricity);
    float HG2 = henyeyGreenstein(VdL, 0.99 - SilverSpread);
    float hg = max(HG1, SilverIntensity*HG2);

	float bl = beerLambert(TotalDensity);

	float powder = 1-beerLambert(TotalDensity*0.2); 

	vec3 sunLight = SunColor * 0.4 * (bl * hg);
    vec3 ambientLight = 0.9 * mix(SkyColor,SkyColor.zzz,0.3);


	vec4 Cirrus = vec4(sunLight + ambientLight * powder,BaseDensity); 

	return mix(CurrentCloud, Cirrus, CurrentCloud.a); 

}*/