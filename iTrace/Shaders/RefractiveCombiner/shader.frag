#version 430 core

in vec2 InTexCoord; 
layout(location = 0) out vec4 Lighting;
layout(location = 1) out vec3 Glow; 
layout(location = 2) out float DofDepth; //<- depth used for DoF 

layout(binding = 0, std140) buffer DepthMiddleBuffer {
	vec4 OcclusionDepth[]; 
};

uniform sampler2D CombinedLighting; 
uniform sampler2D CombinedGlow; 

uniform sampler2D Depth; 
uniform sampler2D RefractiveBlocks; 
uniform sampler2D PrimaryRefractionDepth; 
uniform sampler2D PrimaryRefractionColor; 
uniform sampler2D PrimaryRefractionNormal; 
uniform sampler2D PrimaryRefractionNormalLF; 
uniform sampler2D Volumetrics; 
uniform sampler2D WaterRefraction; 
uniform sampler2D WaterAlbedo; 
uniform sampler2D WaterDepth; 
uniform sampler2D WaterNormal; 
uniform sampler2D WaterPosition; 
uniform sampler2D IndirectSpecular; 
uniform sampler2D WaterReflection; 
uniform sampler2DArray WaterCaustics; 
uniform sampler2D WaterError; 

uniform sampler2DShadow ShadowMaps[4];



uniform ivec2 PixelFocusPoint; 

uniform mat4 IdentityMatrix;
uniform mat4 InverseView; 
uniform mat4 InverseProj; 
uniform mat4 ShadowMatrices[4]; 
uniform vec3 LightDirection; 
uniform vec3 SunColor; 
uniform vec3 CameraPosition; 

const bool ChromaticAbberation = false; 
const bool HitLocationDepth = true; //<- if you want to use the surface location or the hit location depth for the DoF (refraction) 
const vec3 CAMultiplier = vec3(1.02, 1.0, 1.04); //<- multipliers for chromatic abberation 

uniform float znear; 
uniform float zfar; 

float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
} 


vec3 GetWorldPosition(float z, vec2 TexCoord) {

	vec4 Clip = vec4(TexCoord * 2.0 - 1.0, z * 2.0 -1.0, 1.0); 
	vec4 ViewSpace = InverseProj * Clip; 
	ViewSpace /= ViewSpace.w; 

	vec4 World = (InverseView * ViewSpace); 
	return World.xyz; 
}



//takes in origin + direction and returns analytical traversal for a ray within that block 

float AnalyticalTraversal(vec3 Direction, vec3 Origin) {


	vec3 BlockPosition = floor(Origin); //<- assumes one block is one unit! 
	vec3 BlockPositionCeil = BlockPosition + 1; 



	//the 2 y planes 

	float Y = (BlockPosition.y-Origin.y) / Direction.y; 

	if(Y < 0.0) 
		Y = (BlockPositionCeil.y-Origin.y ) / Direction.y; 

	

	//the 2 x planes 

	float X = (BlockPosition.x-Origin.x) / Direction.x; 
	if(X < 0.0) 
		X = ( BlockPositionCeil.x-Origin.x) / Direction.x; 
	//the 2 z planes 

	float Z = ( BlockPosition.z-Origin.z) / Direction.z; 
	if(Z < 0.0) 
		Z = (BlockPositionCeil.z-Origin.z) / Direction.z; 

	if(Y < 0.0) 
		Y = 10000.0; 

	if(X < 0.0) 
		X = 10000.0; 

	if(Z < 0.0) 
		Z = 10000.0; 

	float t = min(X,min(Y,Z)); 

	return t; 

}

void ManageRefraction(inout vec2 TC, inout vec3 Multiplier, float Depth) {
	
	float RefractiveDepth = texelFetch(PrimaryRefractionDepth, ivec2(gl_FragCoord), 0).x; 
	
	if(RefractiveDepth < Depth) {
		
		

		vec3 Normal = texture(PrimaryRefractionNormal, TC).xyz; 

		vec3 WorldPosition = GetWorldPosition(RefractiveDepth, TC); 
		
		vec3 Incident = normalize(WorldPosition - CameraPosition); 

		vec3 RayDir = refract(Incident, Normal, 1.0/1.33); 

		vec3 NewPosition = WorldPosition + RayDir * 0.2; 

		float t = AnalyticalTraversal(RayDir, WorldPosition - Normal * 0.01); 

		vec3 PrimaryColor = texture(PrimaryRefractionColor, TC).xyz;; 
		//PrimaryColor = mix(vec3(1.0), pow(PrimaryColor,vec3(5.0)),t/sqrt(3)); 
		



		vec4 Clip = IdentityMatrix * vec4(NewPosition, 1.0);
		Clip.xyz /= Clip.w; 


		TC = Clip.xy * 0.5 + 0.5; 

		Multiplier *= PrimaryColor * texture(RefractiveBlocks, TC).xyz;  



	}

}

vec2 TCFromWorldPos(vec3 WorldPos) {

	vec4 Clip = IdentityMatrix * vec4(WorldPos, 1.0); 
	Clip.xyz /= Clip.w; 

	return Clip.xy * 0.5 + 0.5; 

}



float DirectBasic(vec3 Position) {

	vec3 NDC = vec3(-1.0); 

	int Cascade = -1; 

	for(int i = 0; i < 4; i++) {
		
		vec4 Clip = ShadowMatrices[i] * vec4(Position, 1.0); 
	
		NDC = Clip.xyz / Clip.w; 

		if((abs(NDC.x) < 0.9 && abs(NDC.y) < 0.9) || (abs(NDC.x) < 1.0 && abs(NDC.y) < 1.0 && i == 3)) {
			Cascade = i; 
			break; 
		}

	}
	if(Cascade == -1) 
		return 0.0; 

	return texture(ShadowMaps[Cascade], vec3(NDC.xy * 0.5 + 0.5, (NDC.z * 0.5 + 0.5)-0.00009)); 

}

float RaySphereIntersect(vec3 rd, vec3 s0, float sr) {
    // - r0: ray origin
    // - rd: normalized ray direction
    // - s0: sphere center
    // - sr: sphere radius
    // - Returns distance from r0 to first intersecion with sphere,
    //   or -1.0 if no intersection.
    float a = dot(rd, rd);
    vec3 s0_r0 = s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);
    if (b*b - 4.0*a*c < 0.0) {
        return 0.0;
    }
    return 1.0;
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

vec4 textureBicubic(sampler2D sampler, vec2 texCoords){

   vec2 texSize = textureSize(sampler, 0);
   vec2 invTexSize = 1.0 / texSize;

   texCoords = texCoords * texSize;

	ivec2 pixel = ivec2(texCoords); 


   vec4 sample0 = texelFetch(sampler, ivec2(pixel), 0);
   vec4 sample1 = texelFetch(sampler, ivec2(pixel) + ivec2(1,0), 0);
   vec4 sample2 = texelFetch(sampler, ivec2(pixel) + ivec2(0,1), 0);
   vec4 sample3 = texelFetch(sampler, ivec2(pixel) + ivec2(1,1), 0);
	
	vec2 interp = fract(texCoords); 


	if(sample0.x < 0.0) 
		sample0 = sample1; 
	if(sample1.x < 0.0) 
		sample1 = sample0; 
	if(sample3.x < 0.0) 
		sample2 = sample1; 
	if(sample2.x < 0.0) 
		sample3 = sample0; 

	if(sample0.x < 0.0) {
		sample1 = sample2; 
		sample0 = sample2; 
	}
	if(sample2.x < 0.0) {
		sample2 = sample0; 
		sample3 = sample0; 
	}

	if(sample0.x < 0.0 || sample1.x < 0.0 || sample2.x < 0.0 || sample3.x < 0.0) 
		return vec4(-1.0); 

    return mix(
       mix(sample2,sample3, interp.x), mix(sample0,sample1, interp.x)
    , 1.0-interp.y);
}

vec2 GetWaterTC(vec2 TC, float InDepth) {

	float RefractiveDepth = texelFetch(WaterDepth, ivec2(gl_FragCoord), 0).x ; 
	float Error = texture(WaterError, TC).x; 

	vec2 RawTC = TC; 

	if(RefractiveDepth < InDepth) {

		

		TC = textureBicubic(WaterRefraction, InTexCoord).xy; ; 
		if(TC.x < 0.0) 
			TC = InTexCoord; 

	
		TC = mix(TC, InTexCoord, Error); 

	vec3 WaterPosition = texture(WaterPosition, RawTC).xyz; 
	vec3 WaterNormal = normalize(texture(WaterNormal, RawTC).xyz); 

	vec3 Incident = normalize(WaterPosition-CameraPosition); 

	vec3 Direction = refract(Incident, WaterNormal, 1.0 / 1.2);

	vec4 Clip = IdentityMatrix * vec4(WaterPosition - WaterNormal * vec3(1.5,0.0,1.5), 1.0); 
	Clip.xyz /= Clip.w ; 
	Clip.xy = Clip.xy * 0.5 + 0.5; 

	Clip.xy = clamp(Clip.xy, vec2(0.0), vec2(1.0)); 
	Clip.xy = Clip.xy - RawTC; 

	TC += Clip.xy; 

	}

	


	return TC; 

}



void HandleRefraction(float InDepth, inout vec3 WaterColor, inout vec2 TC, out float RefractedDepth, out vec3 GlowAddon, out vec3 LightingAddon) {

	float RefractiveDepth = texelFetch(WaterDepth, ivec2(gl_FragCoord), 0).x ; 

	GlowAddon = vec3(0.0); 

	if(RefractiveDepth < InDepth) {
		
		vec3 WaterPosition = texture(WaterPosition, TC).xyz; 
		vec3 WaterNormal = texture(WaterNormal, TC).xyz; 
		vec3 Incident = normalize(WaterPosition-CameraPosition); 

		vec3 Direction = refract(Incident, WaterNormal, 1.0 / 1.33);
		
		vec4 Clip = IdentityMatrix * vec4(WaterPosition + Direction, 1.0); 
		Clip.xyz /= Clip.w ; 
		Clip.xy = Clip.xy * 0.5 + 0.5; 


		vec3 IndirectSpecular = texture(IndirectSpecular, TC).xyz; 

		float Fresnel = 1-abs(dot(Incident, WaterNormal)); 
		Fresnel = Fresnel * Fresnel * Fresnel * Fresnel; 

		vec4 RawAlbedoSample = texture(WaterAlbedo, TC); 
		float FoamFactor = RawAlbedoSample.w; 

		vec2 NewTC = textureBicubic(WaterRefraction, InTexCoord).xy; 
		//vec2 TCReflective = textureBicubic(WaterReflection, InTexCoord).xy; 

		//if(abs(TCReflective.x * 2.0 - 1.0) < 1.0 && abs(TCReflective.y * 2.0 - 1.0) < 1.0)
		//	IndirectSpecular.xyz = texture(CombinedLighting, TCReflective).xyz; 

		float SecondaryDepth = texelFetch(Depth, ivec2(NewTC * textureSize(Depth,0)), 0).x; 

		float Difference = abs(LinearDepth(RefractiveDepth)-LinearDepth(SecondaryDepth)); 

		Difference = clamp(1.0 - Difference*4.0,0.0,1.0); 

		FoamFactor *= (1.0-Difference); 

		WaterColor = texture(WaterAlbedo, TC).xyz * (1.0-Fresnel) * (1.0-FoamFactor); 
		




		

		WaterColor = mix(WaterColor, vec3(1.0), Difference); 
		TC = NewTC; 

		//figure out the 

		
		float ShadowFactor = DirectBasic(WaterPosition); 

		float SpecularWaterFactor = ShadowFactor * RaySphereIntersect(reflect(Incident, WaterNormal), LightDirection*10.0,0.5); 
		float DiffuseWaterFactor = ShadowFactor * max(dot(WaterNormal, LightDirection), 0.0); 

		//ShadowFactor *= pow(max(dot(reflect(Incident,WaterNormal), LightDirection),0.0),100.0); 


		GlowAddon = SunColor * SpecularWaterFactor * Fresnel * (1.0-FoamFactor);
		LightingAddon = GlowAddon; 
		LightingAddon += SunColor * DiffuseWaterFactor * FoamFactor * 0.08; 
		LightingAddon += IndirectSpecular * Fresnel; 
		//GlowAddon = SunColor * ShadowFactor; 

		LightingAddon = WaterNormal; 

	}

}

float WaterDiffuse(vec3 Normal, vec3 LightDirection, float Power) {
	return pow(dot(Normal, LightDirection) * 0.4 + 0.6, Power); 
}


void HandleWater(inout vec3 Lighting, inout vec3 Glow, vec2 RawTC, vec2 TC, float InDepth) {
	float RefractiveDepth = texelFetch(WaterDepth, ivec2(gl_FragCoord), 0).x ; 

	if(RefractiveDepth < InDepth) {
		
		vec3 RefractedFactor = Lighting; 
		vec3 RefractedGlowFactor = Lighting; 

		vec3 IndirectSpecular = texture(IndirectSpecular, RawTC).xyz; 

		vec3 WaterPosition = texture(WaterPosition, RawTC).xyz; 
		vec3 WaterNormal = normalize(texture(WaterNormal, RawTC).xyz); 
		vec3 Incident = normalize(WaterPosition-CameraPosition); 
		vec4 RawAlbedoFetch = texture(WaterAlbedo, RawTC); 

		vec3 Direction = refract(Incident, WaterNormal, 1.0 / 1.33);
		
		

		//TC += Clip.xy; 
	
		//vec2 TCReflective = textureBicubic(WaterReflection, InTexCoord).xy; 

		//if(abs(TCReflective.x * 2.0 - 1.0) < 1.0 && abs(TCReflective.y * 2.0 - 1.0) < 1.0)
		//	IndirectSpecular.xyz = texture(CombinedLighting, TCReflective).xyz; 


		float Density = 2.0; 
		vec3 SEA_BASE = vec3(0.0,0.09,0.18); 
		vec3 SEA_WATER_COLOR = vec3(0.8,0.9,0.6);
		vec3 ScatteringWater = vec3(0.01, 0.025, 0.05) * 2 * Density; 
		vec3 AbsorptionWater = vec3(0.8,0.08,0.01) * 0.5 * Density; 
		vec3 ExtinctionWater = ScatteringWater + AbsorptionWater; 

		float z = texelFetch(Depth, ivec2(TC * textureSize(Depth, 0)), 0).x; 

		vec3 WP = GetWorldPosition(z, TC); 

		float Traversal = distance(WP, WaterPosition); 

		vec3 Transmission = vec3(exp(-ExtinctionWater.x * Traversal),exp(-ExtinctionWater.y * Traversal),exp(-ExtinctionWater.z * Traversal)); 
		vec3 S = vec3(0.04); 

		vec3 SIntegrated = (S - S * Transmission) / ExtinctionWater; 

		


		//-> volume interaction 
		RefractedFactor = mix(SIntegrated*Lighting,Lighting,Transmission); 
		RefractedGlowFactor = mix(SIntegrated,Glow,Transmission); 

		//-> surface interaction 

		vec3 SEA_WATER = SEA_WATER_COLOR * pow(max(dot(LightDirection, WaterNormal),0.0) * 0.4 + 0.6,2.0);  

		float WaveFactor = pow(WaterNormal.y,256.0); 
		vec3 SEA_SURFACE = mix(SEA_BASE ,SEA_WATER, WaveFactor*pow(1.0-RawAlbedoFetch.w,3.0)) ; 

		float ShadowFactor = DirectBasic(WaterPosition); 

		float SpecularWaterFactor = ShadowFactor * RaySphereIntersect(reflect(Incident, WaterNormal), LightDirection*10.0,0.5); 
		float DiffuseWaterFactor = ShadowFactor * max(dot(WaterNormal, LightDirection), 0.0); 
		
		

		float Fresnel = pow(clamp(1-dot(WaterNormal, -Incident),0.0,1.0),4.0); 


		Lighting.xyz = mix(SEA_SURFACE * RefractedFactor, IndirectSpecular, Fresnel); 
		Glow.xyz = SEA_SURFACE * RefractedGlowFactor * (1.0-Fresnel); 
		
		Lighting.xyz += SpecularWaterFactor * SunColor * Fresnel; 
		Glow += SpecularWaterFactor * SunColor * Fresnel; 
		//Lighting.xyz *= vec3(WaveFactor * 0.7 + 0.3); 
		//Lighting.xyz = SEA_SURFACE; 
		

		//Lighting.z = 0.; 
		//Lighting.xy = abs(RawTC-Clip.xy); 
	

	}
	
}



void main() {

	vec3 Multiplier = vec3(1.0); 

	float DepthSample = texelFetch(Depth, ivec2(gl_FragCoord), 0).x; 
	float RefractiveDepth = texelFetch(PrimaryRefractionDepth, ivec2(gl_FragCoord), 0).x; 
	
	vec2 TC = InTexCoord; 
	vec3 ColorSample = vec3(1.0); 

	vec3 WaterMultiplier = vec3(1.0); 

	//TC = texture(WaterRefraction, InTexCoord).xy; 
	//float WaterDepth; 
	//vec3 GlowAddon; 
	//vec3 LightingAddon; 
	//HandleRefraction(DepthSample, WaterMultiplier, TC, WaterDepth, GlowAddon,LightingAddon); 

	TC = GetWaterTC(TC,DepthSample); 

	DepthSample = texelFetch(Depth, ivec2(TC * textureSize(Depth,0).xy),0).x; 

	if(RefractiveDepth < DepthSample) {
		
		ColorSample = pow(texture(PrimaryRefractionColor, TC).xyz,vec3(2.2)); 

		vec3 Normal = texture(PrimaryRefractionNormal, TC).xyz; 
		vec3 LFNormal = texture(PrimaryRefractionNormalLF, TC).xyz; 

		vec3 WorldPosition = GetWorldPosition(RefractiveDepth, TC); 
		
		vec3 Incident = normalize(WorldPosition - CameraPosition); 

		if(ChromaticAbberation) { //<- temporarily removed 
			vec3 RayDirR = refract(Incident, Normal, 1.0/(1.3*CAMultiplier.r)); 
			vec3 RayDirG = refract(Incident, Normal, 1.0/(1.3*CAMultiplier.g)); 
			vec3 RayDirB = refract(Incident, Normal, 1.0/(1.3*CAMultiplier.b)); 


			//todo: add option for these traversals to be the same, improving performance
			//with only a small cost in accuracy 
			vec3 Traversals = vec3(
				AnalyticalTraversal(RayDirR, WorldPosition - Normal * 0.01),
				AnalyticalTraversal(RayDirG, WorldPosition - Normal * 0.01), 
				AnalyticalTraversal(RayDirB, WorldPosition - Normal * 0.01)); 

			vec2 TCR = TCFromWorldPos(WorldPosition + RayDirR * Traversals.r); 
			vec2 TCG = TCFromWorldPos(WorldPosition + RayDirG * Traversals.g); 
			vec2 TCB = TCFromWorldPos(WorldPosition + RayDirB * Traversals.b); 

			vec4 LightingFetchR = texture(CombinedLighting, TCR); 
			vec4 LightingFetchG = texture(CombinedLighting, TCG); 
			vec4 LightingFetchB = texture(CombinedLighting, TCB); 

			Glow = vec3(
				texture(CombinedGlow, TCR).r, 
				texture(CombinedGlow, TCG).g, 
				texture(CombinedGlow, TCB).b); 

			Lighting = vec4(
				LightingFetchR.r,
				LightingFetchG.g,
				LightingFetchB.b,
				LightingFetchR.w); 

			ivec2 Pixel = ivec2(TCG * textureSize(Depth, 0)); 

			if(HitLocationDepth)
				DofDepth = texelFetch(Depth, Pixel,0).x; 
			else 
				DofDepth = RefractiveDepth; 

			//Lighting = vec3(
		
		}
		else {
			vec3 RayDir = refract(Incident, Normal, 1.0/1.1); 
			
			float Traversal = AnalyticalTraversal(RayDir, (WorldPosition - LFNormal * 0.04) + RayDir * 0.001); 
	
			TC = TCFromWorldPos(WorldPosition + RayDir * Traversal); 
			float DepthFetchHere = texelFetch(Depth, ivec2(TC * textureSize(Depth, 0).xy),0).x; 

			if(DepthFetchHere < RefractiveDepth) 
				TC = InTexCoord; 


			Glow = texture(CombinedGlow, TC).xyz;
			Lighting = texture(CombinedLighting, TC); 

			ivec2 Pixel = ivec2(TC * textureSize(Depth, 0)); 

			if(HitLocationDepth)
				DofDepth = texelFetch(Depth, Pixel,0).x; 
			else 
				DofDepth = RefractiveDepth; 

			//Lighting.xyz = vec3(Traversal); 

			ColorSample = pow(ColorSample / 2.0, vec3((Traversal+.5)*1.0)) * 2.0; 
			ColorSample = clamp(ColorSample, vec3(0.0), vec3(1.0));  
			
		}

		ColorSample *= texture(RefractiveBlocks, TC).xyz; 
		//Lighting.xyz = ColorSample; 
		Lighting.xyz *= ColorSample; 

		Glow.xyz *= ColorSample; 



	}
	else {
	
		Lighting = texture(CombinedLighting, TC); 
		Glow = texture(CombinedGlow, TC).xyz; 
		DofDepth = DepthSample; 

	}
	vec4 Volumetrics = texture(Volumetrics, InTexCoord); 
	Lighting.xyz += Volumetrics.xyz * ColorSample; 
	gl_FragDepth = min(DofDepth,0.9999999); 

	ivec2 Pixel = ivec2(gl_FragCoord); 
	HandleWater(Lighting.xyz, Glow, InTexCoord, TC, DepthSample); 

	if(Pixel.x == PixelFocusPoint.x && Pixel.y == PixelFocusPoint.y) {
		
		OcclusionDepth[0] = vec4(DofDepth); 

	}
	//Lighting.w = LinearDepth(WaterDepth); 

	//Glow = vec3(0.0); 
	//Lighting.xy = texture(WaterRefraction, InTexCoord).xy; 
	//Lighting.z = 0.0; 
	//Lighting.xyz *= WaterMultiplier; 
	//Glow *= WaterMultiplier; 
	//Lighting.xyz += GlowAddon + LightingAddon; 
	//Glow += GlowAddon; 

	//Lighting.xyz = texture(IndirectSpecular, InTexCoord).xyz; 

	//Glow = vec3(0.0); 
	//Lighting.xyz = WaterMultiplier; 

	//Lighting.xyz *= (1.0-texture(WaterError, InTexCoord).xxx); 

}