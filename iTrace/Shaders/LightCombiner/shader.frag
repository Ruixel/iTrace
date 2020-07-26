#version 420
#extension GL_ARB_bindless_texture : enable
#extension GL_ARB_gpu_shader_fp64 : enable

in vec2 InTexCoord; 
layout(location = 0) out vec4 Lighting;
layout(location = 1) out vec3 Glow; 

uniform sampler2D Indirect; 
uniform sampler2D IndirectSpecular; 
uniform sampler2D Albedo; 
uniform sampler2D Normal; 

uniform sampler2D Sky; 
uniform sampler2D WorldPos; 

uniform sampler2D HighfreqNormal; 

uniform sampler2D ShadowMaps[5]; 

uniform sampler2D Volumetrics; 

uniform sampler2D DirectMultiplier; 
uniform sampler2D SimpleLight; 
uniform sampler2D Clouds; 
uniform sampler2D Particles; 
uniform sampler2D ParticleDepth; 
uniform sampler2D Depth; 
uniform sampler2D RefractiveBlocks; 
uniform sampler2D DirectShadow; 
uniform sampler2D PrimaryRefractionDepth; 
uniform sampler2D PrimaryRefractionColor; 
uniform sampler2D PrimaryRefractionNormal; 

uniform bool NoAlbedo; 

uniform mat4 IdentityMatrix;
uniform mat4 InverseView; 
uniform mat4 InverseProj; 
uniform mat4 ShadowMatrices[5]; 
uniform vec3 LightDirection; 
uniform vec3 SunColor; 
uniform vec3 CameraPosition; 

uniform samplerCube SkyCube; 


int FetchFromTexture(sampler2D Texture, int Index) {
	
	int Width = 256; 
	int Height = textureSize(Texture, 0).y; 

	ivec2 Pixel = ivec2(Index % Width, Index / Width); 

	return clamp(int(texelFetch(Texture, ivec2(Pixel), 0).x * 255.0 + .1),0,255); 
	
}

vec3 GetWorldPosition(float z, vec2 TexCoord) {

	vec4 Clip = vec4(TexCoord * 2.0 - 1.0, z * 2.0 -1.0, 1.0); 
	vec4 ViewSpace = InverseProj * Clip; 
	ViewSpace /= ViewSpace.w; 

	vec4 World = (InverseView * ViewSpace); 
	return World.xyz; 
}


float pi = 3.14159265; 
	
//an extension to Schlicks fresnel (although not super accurate, but at least it handles different roughnesses) 
vec3 Fresnel(vec3 Incident, vec3 Normal, vec3 Specular, float Roughness) {
	return Specular + (max(vec3(pow(1-Roughness,3.0)) - Specular,vec3(0.0))) * pow(max(1.0 - clamp(dot(Incident,Normal),0.0,1.0),0.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    double a = roughness*roughness;
    double a2 = a*a;
    double NdotH = max(dot(N, H), 0.0);
    double NdotH2 = NdotH*NdotH;

    double nom   = a2;
    double denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = pi * denom * denom;

    return float(nom / denom); // prevent divide by zero for roughness=0.0 and NdotH=1.0
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 GetPointSpecularShading(vec3 InNormal, vec3 LightDirection,vec3 Incident, vec3 F0,vec3 HalfVector, float Roughness) {
	return 	(DistributionGGX(InNormal, HalfVector, Roughness) * GeometrySmith(InNormal, Incident, LightDirection, Roughness) * F0) / (4.0 * max(dot(InNormal, Incident), 0.000001) * max(dot(InNormal, LightDirection), 0.00001) + 0.00001); 
}



//takes in origin + direction and returns analytical traversal for a ray within that block 

float AnalyticalTraversal(vec3 Direction, vec3 Origin) {


	vec3 BlockPosition = floor(Origin); //<- assumes one block is one unit! 
	vec3 BlockPositionCeil = BlockPosition + 1; 



	//the 2 y planes 

	float Y = (BlockPosition.y-Origin.y) / Direction.y; 

	if(Y < 0.01) 
		Y = (BlockPositionCeil.y-Origin.y ) / Direction.y; 

	

	//the 2 x planes 

	float X = (BlockPosition.x-Origin.x) / Direction.x; 
	if(X < 0.01) 
		X = ( BlockPositionCeil.x-Origin.x) / Direction.x; 
	//the 2 z planes 

	float Z = ( BlockPosition.z-Origin.z) / Direction.z; 
	if(Z < 0.01) 
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

void ManageDirect(vec3 WorldPos, vec3 BasicNormal, vec3 Normal, float Roughness, vec3 F0, vec3 Incident, out vec3 DirectDiffuse, out vec3 DirectSpecular) {

	vec3 R = reflect(Incident, Normal); 
	//Cook torrance brdf 

	float Radius = 0.7; 

	vec3 L = LightDirection * 10.0; 

	vec3 LinePoint = R * dot(R, L); 
	vec3 ToLinePoint = LinePoint - L; 
	vec3 ClosestPoint = L + ToLinePoint * clamp(Radius / length(ToLinePoint), 0.0, 1.0); 

	vec3 l = normalize(ClosestPoint); 
	vec3 HalfVector = normalize(l + Incident); 

	float NDF = DistributionGGX(Normal, HalfVector, max(Roughness,0.05)); //normal distribution function
	float G = GeometrySmith(Normal, Incident, l, Roughness); //geometry function
	vec3 F = Fresnel(Incident, HalfVector, F0, Roughness); 

	vec3 Specular = (NDF * G * F) / max(4.0 * max(dot(Normal,Incident),0.0) * max(dot(Normal,LightDirection),0.0) + .00001,0.000001); 

	float Alpha = Roughness * Roughness; 
	float AlphaDot = clamp(Alpha + Radius / (3.0 * length(ClosestPoint)),0.0,1.0); 
	
	float SphereNormalization = (Alpha) / (AlphaDot); 

	vec3 DiffuseMultiplier = vec3(1.0) - F; 


	float NDotL = max(dot(Normal, LightDirection),0.0); 

	DirectDiffuse = (SunColor / pi) * NDotL * DiffuseMultiplier; 



	DirectSpecular = SunColor * Specular * NDotL * (SphereNormalization * SphereNormalization) ; 

	//DirectSpecular = GetPointSpecularShading(Normal, l, Incident, F, HalfVector, Roughness) * SphereNormalization * SphereNormalization; 

}


void main() {

	vec3 Multiplier = vec3(1.0); 

	float DepthSample = texelFetch(Depth, ivec2(gl_FragCoord), 0).x; 
	float ParticleDepthSample = texelFetch(ParticleDepth, ivec2(gl_FragCoord), 0).x; 

	vec2 TexCoord = InTexCoord; 
	
	if(DepthSample > ParticleDepthSample) {
		
		vec4 ParticleSample = texelFetch(Particles, ivec2(gl_FragCoord), 0); 
		ParticleSample.w = pow(ParticleSample.w,0.25); 
		TexCoord += ParticleSample.xy * mix(0.0,0.05,ParticleSample.w); 
		Multiplier = mix(vec3(1.0),vec3(0.8,0.9,1.0)*0.9,ParticleSample.w); 

	}
	
	ManageRefraction(TexCoord, Multiplier, DepthSample); 

	vec4 NormalFetch = textureLod(Normal, TexCoord,0.0); 

	float L = length(NormalFetch.xyz); 

	if(L > 0.3 && L < 1.7) {

		vec4 HighfreqNormalSample = texture(HighfreqNormal, TexCoord); 

		vec3 WorldPosFetch = texture(WorldPos, TexCoord).xyz; 

		vec3 Incident = normalize(WorldPosFetch - CameraPosition); 

		vec3 SpecDir = reflect(Incident, HighfreqNormalSample.xyz); 

		vec4 IndirectDiffuse = texture(Indirect, TexCoord); 
		vec4 IndirectSpecular = texture(IndirectSpecular, TexCoord); 

		float Roughness = HighfreqNormalSample.w; 

		vec4 AlbedoFetch = texture(Albedo, TexCoord); 

		if(NoAlbedo) {
			AlbedoFetch.xyz = vec3(1.0); 
		}
		vec3 F0 = mix(mix(vec3(0.04), vec3(0.0), Roughness), AlbedoFetch.xyz, AlbedoFetch.w); 

		vec3 SpecularColor = Fresnel(-Incident, HighfreqNormalSample.xyz, F0, Roughness); 
		vec3 DiffuseColor = mix(AlbedoFetch.xyz,vec3(0.0),AlbedoFetch.w); 		


		vec3 Shadow = texture(DirectShadow, TexCoord).xyz; 

	//	vec3 Direct = Shadow * max(dot(LightDirection, HighfreqNormalSample.xyz),0.0);
	//	vec3 DirectSpecular = 10.0 * Shadow * pow(max(dot(LightDirection, SpecDir),0.0),1024.0); 

		vec3 Direct, DirectSpecular; 

		ManageDirect(WorldPosFetch, NormalFetch.xyz,normalize(HighfreqNormalSample.xyz), max(Roughness,0.03), F0, -Incident, Direct, DirectSpecular); 

		Direct *= Shadow; 
		DirectSpecular *= Shadow;    

		vec3 Kd = 1 - SpecularColor; //for energy conservation, Specular + Diffuse <= 1, so ensure this is the case! 

		Lighting.xyz = DiffuseColor * ((IndirectDiffuse.xyz * Kd + Direct)) + SpecularColor * (IndirectSpecular.xyz) + DirectSpecular + AlbedoFetch.xyz * NormalFetch.www;
		//Lighting.xyz =  DiffuseColor * ((IndirectDiffuse.xyz + Direct) * IndirectDiffuse.www ); 
		Glow.xyz = DirectSpecular * pow(1.0-Roughness,5.0) + AlbedoFetch.xyz * NormalFetch.www; 
		//Glow.xyz = vec3(0.); 
		//Lighting.xyz = IndirectSpecular.xyz; 
		//Lighting.xyz = vec3(Shadow); 


	}
	else {
		vec4 CloudSample = texture(Clouds, TexCoord); 
		Lighting = mix(texture(Sky, TexCoord),CloudSample,pow(1-CloudSample.w,2.0)); 
	}
	vec4 Volumetrics = texture(Volumetrics, TexCoord); 

	Glow.xyz = vec3(0.0); 
	Lighting.xyz *= Multiplier; 

	Lighting.xyz += Volumetrics.xyz; 

}