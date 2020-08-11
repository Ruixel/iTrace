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

vec2 Kernels[51] = vec2[51](
vec2(0.00390625,-0),
vec2(0.0603367,-0.128936),
vec2(-0.128061,-0.15346),
vec2(-0.227301,0.0599167),
vec2(-0.048118,0.264502),
vec2(0.250687,0.178076),
vec2(0.280579,-0.158967),
vec2(-0.0290974,-0.373316),
vec2(-0.34729,-0.130682),
vec2(-0.291286,0.284268),
vec2(0.138152,0.396194),
vec2(0.435724,0.0445877),
vec2(0.24203,-0.404635),
vec2(-0.263341,-0.389367),
vec2(-0.517893,0.0822949),
vec2(-0.13934,0.485495),
vec2(0.406607,0.354163),
vec2(0.498269,-0.22081),
vec2(0.0111189,-0.557759),
vec2(-0.531133,-0.261174),
vec2(-0.453163,0.363994),
vec2(0.151844,0.621488),
vec2(0.598553,0.119764),
vec2(0.38213,-0.519114),
vec2(-0.309577,-0.567813),
vec2(-0.654743,0.0406989),
vec2(-0.253446,0.643877),
vec2(0.463911,0.489348),
vec2(0.699053,-0.232933),
vec2(0.0806435,-0.695484),
vec2(-0.625404,-0.385607),
vec2(-0.614048,0.40336),
vec2(0.106442,0.733428),
vec2(0.746403,0.225141),
vec2(0.504134,-0.563291),
vec2(-0.322943,-0.756055),
vec2(-0.77925,-0.0260326),
vec2(-0.369307,0.72618),
vec2(0.500768,0.640928),
vec2(0.796456,-0.183267),
vec2(0.179765,-0.839643),
vec2(-0.661411,-0.500714),
vec2(-0.79504,0.419711),
vec2(0.0413755,0.850904),
vec2(0.821128,0.336082),
vec2(0.65061,-0.60033),
vec2(-0.268957,-0.845017),
vec2(-0.923605,-0.119745),
vec2(-0.48111,0.757234),
vec2(0.521294,0.817409),
vec2(0.910716,-0.119648)
);
//6 for red, 3 for yellow, 4 for green, 8 for cyan, 10 for blue
int KernelPicks[6] = int[6](6,3,4,8,10,12); 

vec3 FetchDof(sampler2D Sampler, vec2 TexCoord, float Intensity) {

	float LD = texture(LensDirt, TexCoord).x; 

	vec3 Colors[2]; 

	for(int i = 0; i < 6; i++) {

		float Angle = float(_Fetch[i]) / float(6.0); 
		Angle += LD * 2.0; 

		Angle *= 3.14; 


		vec2 Offset = Kernels[KernelPicks[i]]  *.001912378; 

		Colors[i/3][i%3] = (FetchColor(_Fetch[i], Sampler, TexCoord + Offset * Intensity )); 
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