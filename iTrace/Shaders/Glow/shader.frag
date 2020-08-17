#version 330 

layout(location = 0) out vec4 Lighting1; 
layout(location = 1) out vec4 Lighting2; 
layout(location = 2) out vec4 Lighting3; 
layout(location = 3) out vec4 Lighting4; 
layout(location = 4) out vec3 LightingCombined; 

in vec2 TexCoord; 

uniform sampler2D Input1;
uniform sampler2D Input2; 
uniform sampler2D Input3; 
uniform sampler2D Input4; 
uniform sampler2D RawInput; 

uniform vec2 InverseRes; 
uniform int PixelStep; 


const float InverseSQRT2 = 0.70710678118; 

vec2 Directions[4] = vec2[4](
	vec2(InverseSQRT2,InverseSQRT2),
	vec2(InverseSQRT2,-InverseSQRT2),
	vec2(1.0,0.0),
	vec2(0.0,1.0)); 

void main() {

	
	for(int i = -7; i <= 7; i++) {
	
		float Distance = 1.0 / (10.0 + abs(i)*abs(i)*PixelStep*PixelStep); 
	
		vec2 Coordinate1 = TexCoord + float(i * PixelStep) * Directions[0] * InverseRes;
		vec2 Coordinate2 = TexCoord + float(i * PixelStep) * Directions[1] * InverseRes; 
		vec2 Coordinate3 = TexCoord + float(i * PixelStep) * Directions[2] * InverseRes; 
		vec2 Coordinate4 = TexCoord + float(i * PixelStep) * Directions[3] * InverseRes; 

		vec4 Fetch1 = texture(Input1, Coordinate1); 
		vec4 Fetch2 = texture(Input2, Coordinate2); 
		vec4 Fetch3 = texture(Input3, Coordinate3); 
		vec4 Fetch4 = texture(Input4, Coordinate4); 


		Lighting1 += Fetch1 * Distance; 
		Lighting2 += Fetch2 * Distance; 
		Lighting3 += Fetch3 * Distance; 
		Lighting4 += Fetch4 * Distance; 

	}

	LightingCombined = texelFetch(RawInput, ivec2(gl_FragCoord),2).xyz + (Lighting1.xyz / Lighting1.w + Lighting2.xyz / Lighting2.w + Lighting3.xyz / Lighting3.w + Lighting4.xyz / Lighting4.w) * 0.25; 



}