#version 330

in vec2 TexCoord; 
layout(location = 0) out vec4 Packed; 
layout(location = 1) out vec4 PackedSpecular; 

uniform sampler2D InputNormal;
uniform sampler2D InputDepth; 
uniform sampler2D InputNormalHF;
uniform sampler2D InputWaterDepth; 
uniform sampler2D InputWaterNormal; 

uniform float znear; 
uniform float zfar; 


float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
} 

void main() {

    vec4 RawNormal = texture(InputNormal, TexCoord); 

    float Roughness = RawNormal.w; 
    
	RawNormal.xyz = RawNormal.xyz * ((1.0-Roughness)*0.5+0.5); 

	float RawDepth = texelFetch(InputDepth, ivec2(gl_FragCoord.xy)*2,0).x; 
	float WaterDepth = texelFetch(InputWaterDepth, ivec2(gl_FragCoord)*2,0).x; 


    Packed = vec4(RawNormal.xyz, LinearDepth(RawDepth)); 

	if(WaterDepth < RawDepth) {
		PackedSpecular = vec4(texture(InputWaterNormal, TexCoord).xyz, LinearDepth(WaterDepth)); 
	}
	else {
		PackedSpecular = vec4(texture(InputNormalHF, TexCoord).xyz, LinearDepth(RawDepth)); 
	}
	
}