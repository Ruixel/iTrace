#version 330 

layout(location = 0) out vec4 LightingDoF; 

#define DOF_HQ 

in vec2 TexCoord; 

uniform sampler2D Lighting; 
uniform sampler2D Glow; 
uniform sampler2D Depth; 
uniform sampler2D LensDirt; 

uniform float znear; 
uniform float zfar; 

uniform float ImageDistance; 
uniform float FocusPoint; 
uniform float Aperture; 
uniform float MaxRadius; 


vec3 SpectraSplit[6] = vec3[6](
vec3(0.5,0.0,0.0),
vec3(2.0/6.0,2.0/6.0,-1.0/6.0),
vec3(0.0,0.5,0.0),
vec3(-1.0/6.0,2.0/6.0,2.0/6.0),
vec3(0.0,0.0,0.5),
vec3(2.0/6.0,-1.0/6.0,2.0/6.0)); 


float FetchColor(int ColorIdx, sampler2D Sampler, vec2 Coordinate) {
	
	vec3 RawColor = texture(Sampler, Coordinate).xyz; 

	vec3 Spectra = SpectraSplit[ColorIdx]; 

	return dot(Spectra, RawColor); 

}

vec3 CombineSpectra(vec3 ryg, vec3 cbv) {

	float r = ryg.r; 
	float y = ryg.g; 
	float g = ryg.b; 

	float c = cbv.r; 
	float b = cbv.g; 
	float v = cbv.b; 

	return vec3(r + (2.0*v+2.0*y-c)/3.0,g + (2.0*y + 2.0 * c - v) / 3.0, b + (2.0*c+2.0*v-y)/3.0);  

}

#ifdef DOF_HQ 

vec2 Offsets[6] = vec2[6](
vec2(0.0, 0.0), 
-vec2(.0025,0.0),
vec2(0.0,.001),
vec2(0.0,.0025),
vec2(.001,-0.025),
vec2(0.001,0.0)); 




int _Fetch[6] = int[6](0,1,2,3,4,5); 
float Strength[6] = float[6](.001912378,.001236,.0028123,.0016123,.00214718623,.001847586); 


vec3 FetchDof(sampler2D Sampler, vec2 TexCoord, float Intensity) {

	float LD = texture(LensDirt, TexCoord).x; 

	vec3 Colors[2]; 

	for(int i = 0; i < 6; i++) {

		float Angle = float(_Fetch[i]) / float(6.0); 
		Angle += LD * 2.0; 

		Angle *= 3.14; 


		vec2 Offset = vec2(cos(Angle), sin(Angle)) * Strength[i] * 0.5; 

		Colors[i/3][i%3] = (FetchColor(i, Sampler, TexCoord + Offset * Intensity )); 
	}
	return clamp(CombineSpectra(Colors[0], Colors[1]),vec3(0.0),vec3(1000.0)); 

}

#else 


vec3 FetchDof(sampler2D Sampler, vec2 TexCoord, float Intensity) {
	
	return vec3( 
		texture(Sampler, TexCoord + vec2(.001,0.0) * Intensity).x, 
		texture(Sampler, TexCoord).y, 
		texture(Sampler, TexCoord - vec2(.0025,0.0) * Intensity).z); 

}

#endif 



float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
}

float GetDofRadius(float CurrentDepth, float FocusDepth) {


	//1 / P + 1 / I = 1 / F

	float InverseImageD = 1.0 / ImageDistance; 
	float InverseFocus = 1.0 / FocusDepth; 

	float FocalLength = 1.0 / (InverseImageD + InverseFocus); 


	CurrentDepth = max(CurrentDepth, 0.01); 
	FocusDepth = max(FocusDepth, 0.01); 

	return min(abs(Aperture * (FocalLength * (CurrentDepth - FocusDepth)) / (CurrentDepth * (FocusDepth - FocalLength))), MaxRadius); 

}






void main() {

	ivec2 HighResPixel = ivec2(gl_FragCoord * 4); 


	LightingDoF.w = LinearDepth(texelFetch(Depth, HighResPixel,0).x); 

	float Intensity = min(0.15 * GetDofRadius(LightingDoF.w, FocusPoint),5.0); 



	LightingDoF.xyz = FetchDof(Glow,TexCoord,Intensity).xyz; 
}