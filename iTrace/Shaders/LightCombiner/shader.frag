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
uniform sampler2D SkyReigh; 
uniform sampler2D WaterDepth; 
uniform sampler2DArray WaterCaustics; 
uniform sampler2D Detail; 
uniform sampler2D Direction; 


uniform bool NoAlbedo; 

uniform mat4 IdentityMatrix;
uniform mat4 InverseView; 
uniform mat4 InverseProj; 
uniform mat4 ShadowMatrices[5]; 
uniform vec3 LightDirection; 
uniform vec3 SunColor; 
uniform vec3 CameraPosition; 
uniform float Time; 

uniform samplerCube SkyCube; 

vec4 TextureInterp(sampler2DArray Sampler, vec3 TC) {

	float BaseTime = mod(TC.z, 128.); 

	int Coord1 = int(floor(BaseTime)); 
	int Coord2 = int(ceil(BaseTime))%128; 

	return mix(texture(Sampler, vec3(TC.xy, Coord1)), texture(Sampler,vec3(TC.xy, Coord2)), fract(BaseTime)); 

}

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

vec3 SHToIrridiance(vec4 shY, vec2 CoCg, vec3 N)
{
    float d = dot(shY.xyz, N);
    float Y = 2.0 * (1.023326 * d + 0.886226 * shY.w);
    Y = max(Y, 0.0);

	CoCg *= Y * 0.282095 / (shY.w + 1e-6);

    float   T       = Y - CoCg.y * 0.5;
    float   G       = CoCg.y + T;
    float   B       = T - CoCg.x * 0.5;
    float   R       = B + CoCg.x;

    return max(vec3(R, G, B), vec3(0.0));

}

vec3 SHToIrradiance(vec4 shY, vec2 CoCg)
{
    float   Y       = shY.w / 0.282095;

    float   T       = Y - CoCg.y * 0.5;
    float   G       = CoCg.y + T;
    float   B       = T - CoCg.x * 0.5;
    float   R       = B + CoCg.x;

    return max(vec3(R, G, B), vec3(0.0));
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

	float Radius = 0.3; 

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

   texCoords = texCoords * texSize - 0.5;


    vec2 fxy = fract(texCoords);
    texCoords -= fxy;

    vec4 xcubic = cubic(fxy.x);
    vec4 ycubic = cubic(fxy.y);

    vec4 c = texCoords.xxyy + vec2 (-0.5, +1.5).xyxy;

    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4 (xcubic.yw, ycubic.yw) / s;

    offset *= invTexSize.xxyy;

    vec4 sample0 = texture(sampler, offset.xz);
    vec4 sample1 = texture(sampler, offset.yz);
    vec4 sample2 = texture(sampler, offset.xw);
    vec4 sample3 = texture(sampler, offset.yw);

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return mix(
       mix(sample3, sample2, sx), mix(sample1, sample0, sx)
    , sy);
}

float square(float x) { return x * x; }

float G_Smith_over_NdotV(float roughness, float NdotV, float NdotL)
{
    float alpha = square(roughness);
    float g1 = NdotV * sqrt(square(alpha) + (1.0 - square(alpha)) * square(NdotL));
    float g2 = NdotL * sqrt(square(alpha) + (1.0 - square(alpha)) * square(NdotV));
    return 2.0 *  NdotL / (g1 + g2);
}

float GGX(vec3 V, vec3 L, vec3 N, float roughness, float NoH_offset)
{
    vec3 H = normalize(L - V);
    
    float NoL = max(0, dot(N, L));
    float VoH = max(0, -dot(V, H));
    float NoV = max(0, -dot(N, V));
    float NoH = clamp(dot(N, H) + NoH_offset, 0, 1);

    if (NoL > 0)
    {
        float G = G_Smith_over_NdotV(roughness, NoV, NoL);
        float alpha = square(max(roughness, 0.02));
        float D = square(alpha) / (pi * square(square(NoH) * square(alpha) + (1 - square(NoH))));

        // Incident light = SampleColor * NoL
        // Microfacet specular = D*G*F / (4*NoL*NoV)
        // F = 1, accounted for elsewhere
        // NoL = 1, accounted for in the diffuse term
        return D * G / 4;
    }

    return 0;
}

vec3 BuildApproximateSpecular(vec4 SHy, vec3 RawLighting, float Roughness, vec3 Incident, vec3 Normal) { //uses SH to project specular. 
	vec3 IncomingDir = SHy.xyz / SHy.w * (0.282095 / 0.488603);
	vec3 RawSpecularDir = reflect(Incident, Normal); 

	float IncomingLen = length(IncomingDir);
	float Directionality = IncomingLen;

	if(Directionality >= 1.0) {
			IncomingDir /= IncomingLen; 
	}
	else {
		IncomingDir = mix(RawSpecularDir, IncomingDir / (IncomingLen + 1e-6), vec3(Directionality));
	}

	float brdf = GGX(Incident, IncomingDir, Normal, max(Roughness,0.3), 0);
		
	return RawLighting * brdf; 

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
	
	//Multiplier *= texture(RefractiveBlocks, TexCoord).xyz; 

	vec4 NormalFetch = textureLod(Normal, TexCoord,0.0); 

	float L = length(NormalFetch.xyz); 

	if(L > 0.3 && L < 1.7) {

		vec4 HighfreqNormalSample = texture(HighfreqNormal, TexCoord); 

		vec3 WorldPosFetch = texture(WorldPos, TexCoord).xyz; 

		vec3 Incident = normalize(WorldPosFetch - CameraPosition); 

		vec3 SpecDir = reflect(Incident, HighfreqNormalSample.xyz); 

		vec4 IndirectDiffuse = texture(Indirect, TexCoord); 
		vec4 IndirectSpecular = texture(IndirectSpecular, TexCoord); 

		float Roughness = NormalFetch.w; 

		vec4 AlbedoFetch = texture(Albedo, TexCoord); 

		if(NoAlbedo) {
			AlbedoFetch.xyz = vec3(1.0); 
		}
		vec3 F0 = mix(mix(vec3(0.04), vec3(0.0), Roughness), AlbedoFetch.xyz, AlbedoFetch.w); 

		vec3 SpecularColor = Fresnel(-Incident, HighfreqNormalSample.xyz, F0, Roughness); 
		vec3 DiffuseColor = mix(AlbedoFetch.xyz,vec3(0.0),AlbedoFetch.w); 		


		vec4 Shadow = texture(DirectShadow, TexCoord); 

		if(WorldPosFetch.y < 58.9) {
		
			
			//run a ray-plane intersection function 

			float Traversal = (58.9 - WorldPosFetch.y) / LightDirection.y; 

			vec2 TC = WorldPosFetch.xz + LightDirection.xz * Traversal; 

			Shadow.xyz *= pow(TextureInterp(WaterCaustics, vec3(TC * 0.5, Time * 12.0 + TC.x*10.0)).x,4.0) * 4.0; 

		}
			


		vec2 ShCoCg = vec2(IndirectSpecular.w, Shadow.w); 
		vec4 SHy = IndirectDiffuse; 



		IndirectDiffuse.xyz = SHToIrridiance(SHy, ShCoCg, HighfreqNormalSample.xyz); 


		//if we are under water, the "real" specular is reserved for the water reflection 
		//But! Because we are using SH, we can just project an approximate specular 
		//-> not my original idea: the idea comes from the Quake2RTX project. Really clever stuff! 


		if(texelFetch(WaterDepth, ivec2(gl_FragCoord), 0).x <= DepthSample) 
			IndirectSpecular.xyz = BuildApproximateSpecular(SHy, IndirectDiffuse.xyz, Roughness, Incident, HighfreqNormalSample.xyz); 
	
		vec3 Direct, DirectSpecular; 

		ManageDirect(WorldPosFetch, NormalFetch.xyz,normalize(HighfreqNormalSample.xyz), max(Roughness,0.03), F0, -Incident, Direct, DirectSpecular); 

		Direct *= Shadow.xyz; 
		DirectSpecular *= Shadow.xyz;    

		vec3 Kd = 1 - SpecularColor; //for energy conservation, Specular + Diffuse <= 1, so ensure this is the case! 


		Lighting.xyz = DiffuseColor * ((IndirectDiffuse.xyz * Kd + Direct)) + SpecularColor * (IndirectSpecular.xyz) + DirectSpecular + AlbedoFetch.xyz * HighfreqNormalSample.www;
		Glow.xyz = DirectSpecular * pow(1.0-Roughness,5.0) + AlbedoFetch.xyz * HighfreqNormalSample.www + 
					pow(1.0-Roughness,5.0) * (max((SpecularColor * IndirectSpecular.xyz)-0.4,vec3(0.0))*1.2);
		//Lighting.xyz = IndirectSpecular.xyz; 	
		//Lighting.xyz = Shadow.xyz; 
		//Lighting.xyz = NormalFetch.xyz; 

		//Lighting.xyz = SHy.xyz; 

		//Lighting.xyz = texelFetch(Direction, ivec2(gl_FragCoord)/4,0).www; 
		//Lighting.xyz = IndirectSpecular.xyz; 
	}
	else {
		vec4 CloudSample = textureBicubic(Clouds, TexCoord); 
		Lighting = mix(texture(Sky, TexCoord),CloudSample,pow(1-CloudSample.w,1.0)); 
		Glow = mix(texture(SkyReigh, TexCoord).xyz, vec3(0.0), 1-CloudSample.w); 
	}
	vec4 Volumetrics = texture(Volumetrics, TexCoord); 

//	Glow.xyz *= Multiplier; 
//Glow.xyz = vec3(0.); 
}