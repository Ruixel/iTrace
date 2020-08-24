#version 420

in vec2 TexCoord; 

layout(location = 0) out vec4 IndirectDiffuse;
layout(location = 1) out vec4 IndirectSpecular;
layout(location = 2) out vec4 OutDirect; 

uniform float znear; 
uniform float zfar; 


uniform sampler2D Depth; 
uniform sampler2D Normal; 
uniform sampler2D Lighting; 
uniform sampler2D PackedGeometryData; 
uniform sampler2D Specular; 
uniform sampler2D Direct; 
uniform sampler2D PackedGeometryDataSpecular; 
uniform sampler2D DepthWater; 
uniform sampler2D NormalHF; 
uniform sampler2D WaterNormal; 

uniform mat4 IncidentMatrix; 
uniform bool SpatialUpscaling; 

float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
} 

float GetRoughness(inout vec3 Normal) {

	float L = length(Normal); 

	Normal /= L; 
	
	float Roughness = 1.0 - (L * 2.0 - 1.0); 

	return Roughness; 
}

vec3 GetIncident(vec2 TC) {
	
	return -normalize(vec3(IncidentMatrix * vec4(TC * 2.0 - 1.0, 1.0, 1.0))); ; 
}


void main() {

	ivec2 Pixel = ivec2(gl_FragCoord.xy)/2; 

	vec4 BaseNormal = texture(Normal, TexCoord); 

	float BaseDepth = texelFetch(Depth, ivec2(gl_FragCoord), 0).x; 
	float WaterDepth = texelFetch(DepthWater, ivec2(gl_FragCoord), 0).x; 

	vec4 BaseData = vec4(BaseNormal.xyz, LinearDepth(BaseDepth)); 
	float BaseRoughness = BaseNormal.w; 
	float BaseRoughnessSpecular = BaseNormal.w; 

	vec4 BaseDataSpecular; 

	if(WaterDepth < BaseDepth) {
		BaseDataSpecular = vec4(texture(WaterNormal, TexCoord).xyz, LinearDepth(WaterDepth)); 
		BaseRoughnessSpecular = 0.0; 
	}
	else {
		BaseDataSpecular = vec4(texture(NormalHF, TexCoord).xyz, BaseData.w); 
	}


	float BestWeight = -1.0;
	float BestShadowWeight = BestWeight; 
	float BestSpecularWeight = BestWeight; 

	
	vec2 BestTC = TexCoord; 
	vec2 BestShadowTC = TexCoord; 
	vec2 BestSpecularTC = TexCoord; 

	vec2 Res = textureSize(Lighting, 0).xy; 


	float r = 0.0; 
	if(SpatialUpscaling)
	for(int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {
			




			ivec2 CurrentPixel = ivec2(Pixel) + ivec2(x,y); 
			vec2 CurrentTC = TexCoord + vec2(x,y) / Res; 

			vec4 CurrentData = texture(PackedGeometryData, CurrentTC); 
			vec4 CurrentDataSpecular = texture(PackedGeometryDataSpecular, CurrentTC); 


			float CurrentRoughness = GetRoughness(CurrentData.xyz); 
			float CurrentRoughnessSpecular = GetRoughness(CurrentDataSpecular.xyz); 


			float Weight = 0.0; 
			
			float BaseWeight = abs(BaseData.w - CurrentData.w) + 0.25 * length(vec2(x,y));

			Weight = 100.0 * (1.0-pow(dot(CurrentData.xyz, BaseData.xyz),3.0)) + BaseWeight + 100.0*abs(BaseRoughness - CurrentRoughness); 


			float WeightSpecular = abs(BaseDataSpecular.w - CurrentDataSpecular.w) + 0.25 * length(vec2(x,y)); 
			WeightSpecular = 100.0 * (1.0-pow(dot(CurrentDataSpecular.xyz, BaseData.xyz),128.0)) + WeightSpecular; 


			if(Weight < BestWeight || BestWeight < 0.0f) {
				BestWeight = Weight; 
				BestTC = CurrentTC; 
				r = CurrentRoughness; 
			}

			if(BaseWeight < BestShadowWeight || BestShadowWeight < 0.0) {
				BestShadowWeight = BaseWeight; 
				BestShadowTC = CurrentTC; 
			}

			if(WeightSpecular < BestSpecularWeight || BestSpecularWeight < 0.0) {
				BestSpecularWeight = WeightSpecular; 
				BestSpecularTC = CurrentTC; 
			}
			



		}
	}

	IndirectDiffuse = texture(Lighting, BestTC); 
	IndirectSpecular.w = texture(Specular, BestTC).w;
	IndirectSpecular.xyz = texture(Specular, BestSpecularTC).xyz; 
	OutDirect.xyz = texture(Direct, BestShadowTC).xyz;
	OutDirect.w = texture(Direct, BestTC).w; 
}