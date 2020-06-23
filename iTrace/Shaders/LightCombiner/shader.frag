#version 330
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord; 
layout(location = 0) out vec4 Lighting;

uniform sampler2D Indirect; 
uniform sampler2D IndirectSpecular; 
uniform sampler2D Albedo; 
uniform sampler2D Normal; 

uniform sampler2D Sky; 
uniform sampler2D WorldPos; 

uniform sampler2D HighfreqNormal; 

uniform sampler2D ShadowMaps[5]; 

uniform sampler2D Volumetrics; 

uniform sampler2D DirectVisibility; 

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
	return Specular + (max(vec3(1-Roughness) - Specular,vec3(Specular))) * pow(max(1.0 - abs(dot(Incident,Normal)),0.0), 5.0);
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
		vec3 F0 = mix(vec3(0.04), AlbedoFetch.xyz, AlbedoFetch.w); 

		vec3 SpecularColor = Fresnel(Incident, HighfreqNormalSample.xyz, F0, Roughness); 
		vec3 DiffuseColor = mix(AlbedoFetch.xyz,vec3(0.0),AlbedoFetch.w); 		

		vec3 Direct = IndirectSpecular.w * max(dot(LightDirection, HighfreqNormalSample.xyz),0.0) * SunColor; 

		Lighting.xyz = DiffuseColor * ((IndirectDiffuse.xyz* IndirectDiffuse.www + Direct) * IndirectDiffuse.www ) + SpecularColor * (IndirectSpecular.xyz) + AlbedoFetch.xyz * NormalFetch.www;
		//Lighting.xyz = IndirectDiffuse.xyz * IndirectDiffuse.www; 
	}
	else {
		Lighting = texture(Sky, TexCoord); 
	}
	vec4 Volumetrics = texture(Volumetrics, TexCoord); 

	Lighting.xyz =   mix(Lighting.xyz, Volumetrics.xyz,1.0-Volumetrics.w);
}