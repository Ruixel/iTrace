#version 330 

layout(location = 0) out vec4 LightingDoF; 

in vec2 TexCoord; 

uniform sampler2D Lighting; 
uniform sampler2D Glow; 
uniform sampler2D Depth; 

uniform float znear; 
uniform float zfar; 

uniform float ImageDistance; 
uniform float FocusPoint; 
uniform float Aperture; 
uniform float MaxRadius; 

vec3 FetchDof(sampler2D Sampler, vec2 TexCoord, float Intensity) {
	
	return vec3( 
		texture(Sampler, TexCoord + vec2(.001,0.0) * Intensity).x, 
		texture(Sampler, TexCoord).y, 
		texture(Sampler, TexCoord - vec2(.0025,0.0) * Intensity).z); 

}


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

	ivec2 HighResPixel = ivec2(gl_FragCoord * 2); 


	LightingDoF.w = LinearDepth(texelFetch(Depth, HighResPixel,0).x); 

	float Intensity = min(0.15 * GetDofRadius(LightingDoF.w, FocusPoint),5.0); 


	LightingDoF.xyz = FetchDof(Lighting,TexCoord,Intensity).xyz + FetchDof(Glow,TexCoord,Intensity).xyz; 

}