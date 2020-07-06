#version 420
#extension GL_ARB_bindless_texture : enable
#extension GL_ARB_gpu_shader_fp64 : enable

in vec2 TexCoord; 
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

uniform bool NoAlbedo; 


uniform mat4 ShadowMatrices[5]; 
uniform vec3 LightDirection; 
uniform vec3 SunColor; 
uniform vec3 CameraPosition; 

int FetchFromTexture(sampler2D Texture, int Index) {
	
	int Width = 256; 
	int Height = textureSize(Texture, 0).y; 

	ivec2 Pixel = ivec2(Index % Width, Index / Width); 

	return clamp(int(texelFetch(Texture, ivec2(Pixel), 0).x * 255.0 + .1),0,255); 
	
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

	vec4 NormalFetch = texture(Normal, TexCoord); 

	float L = length(NormalFetch.xyz); 



	if(L > 0.75 && L < 1.25) {

		vec4 HighfreqNormalSample = texture(HighfreqNormal, TexCoord); 

		vec3 WorldPosFetch = texture(WorldPos, TexCoord).xyz; 

		vec3 Incident = normalize(WorldPosFetch - CameraPosition); 

		vec3 SpecDir = reflect(Incident, HighfreqNormalSample.xyz); 

		vec4 IndirectDiffuse = texture(Indirect, TexCoord); 
		vec4 IndirectSpecular = texture(IndirectSpecular, TexCoord); 


		float Roughness = HighfreqNormalSample.w; 


		vec4 AlbedoFetch =  texture(Albedo, TexCoord); 

		if(NoAlbedo) {
			AlbedoFetch.xyz = vec3(1.0); 
		}
		vec3 F0 = mix(mix(vec3(0.04), vec3(0.0), Roughness), AlbedoFetch.xyz, AlbedoFetch.w); 

		vec3 SpecularColor = Fresnel(-Incident, HighfreqNormalSample.xyz, F0, Roughness); 
		vec3 DiffuseColor = mix(AlbedoFetch.xyz,vec3(0.0),AlbedoFetch.w); 		


		float Shadow = texture(DirectMultiplier, TexCoord).x * IndirectSpecular.w; 

	//	vec3 Direct = Shadow * max(dot(LightDirection, HighfreqNormalSample.xyz),0.0);
	//	vec3 DirectSpecular = 10.0 * Shadow * pow(max(dot(LightDirection, SpecDir),0.0),1024.0); 

		vec3 Direct, DirectSpecular; 

		ManageDirect(WorldPosFetch, NormalFetch.xyz,normalize(HighfreqNormalSample.xyz), max(Roughness,0.03), F0, -Incident, Direct, DirectSpecular); 

		Direct *= Shadow; 
		DirectSpecular *= Shadow;    

		vec3 Kd = 1 - SpecularColor; //for energy conservation, Specular + Diffuse <= 1, so ensure this is the case! 

		Lighting.xyz = DiffuseColor * ((IndirectDiffuse.xyz * Kd + Direct) * IndirectDiffuse.www) + SpecularColor * (IndirectSpecular.xyz) + DirectSpecular + AlbedoFetch.xyz * NormalFetch.www;
		//Lighting.xyz =  DiffuseColor * ((IndirectDiffuse.xyz + Direct) * IndirectDiffuse.www ); 
		Glow.xyz = DirectSpecular * pow(1.0-Roughness,5.0) ; 
		//Lighting.xyz = DirectSpecular; 
	}
	else {
		vec4 CloudSample = texture(Clouds, TexCoord); 
		Lighting = mix(CloudSample,texture(Sky, TexCoord),pow(CloudSample.w,1.0)); 
	}
	vec4 Volumetrics = texture(Volumetrics, TexCoord); 

	Lighting.xyz =   mix(Lighting.xyz, Volumetrics.xyz,1.0-Volumetrics.w);
}