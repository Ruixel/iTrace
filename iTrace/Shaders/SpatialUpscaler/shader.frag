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

	vec4 BaseData = vec4(BaseNormal.xyz, LinearDepth(texture(Depth,TexCoord).x)); 

	float BaseRoughness = BaseNormal.w; 

	float BestWeight = -1.0;
	float BestShadowWeight = BestWeight; 

	
	vec2 BestTC = TexCoord; 
	vec2 BestShadowTC = TexCoord; 


	vec2 Res = textureSize(Lighting, 0).xy; 


	vec3 BaseIncident = GetIncident(TexCoord); 

	vec3 BaseReflection = reflect(BaseIncident, BaseNormal.xyz); 
	
	

	float r = 0.0; 
	if(SpatialUpscaling)
	for(int x = -1; x <= 1; x++) {
		for(int y = -1; y <= 1; y++) {
			




			ivec2 CurrentPixel = ivec2(Pixel) + ivec2(x,y); 
			vec2 CurrentTC = TexCoord + vec2(x,y) / Res; 

			vec4 CurrentData = texture(PackedGeometryData, CurrentTC); 

			float CurrentRoughness = GetRoughness(CurrentData.xyz); 

			vec3 CurrentReflection = reflect(GetIncident(CurrentTC), CurrentData.xyz); 

			float Weight = 0.0; 
			
			float BaseWeight = abs(BaseData.w - CurrentData.w) + 0.25 * length(vec2(x,y));

			Weight = 100.0 * (1.0-pow(dot(CurrentData.xyz, BaseData.xyz),3.0)) + BaseWeight + 100.0*abs(BaseRoughness - CurrentRoughness); 


			if(Weight < BestWeight || BestWeight < 0.0f) {
				BestWeight = Weight; 
				BestTC = CurrentTC; 
				r = CurrentRoughness; 
			}

			if(BaseWeight < BestShadowWeight || BestShadowWeight < 0.0) {
				BestShadowWeight = BaseWeight; 
				BestShadowTC = CurrentTC; 
			}



		}
	}

	IndirectDiffuse = texture(Lighting, BestTC); 
	IndirectSpecular = texture(Specular, BestTC); 
	OutDirect.xyz = texture(Direct, BestShadowTC).xyz;
	OutDirect.w = texture(Direct, BestTC).w; 
}