#version 330

in vec2 TexCoord; 
layout(location = 0) out vec4 Packed;

uniform sampler2D InputNormal;
uniform sampler2D InputDepth; 

uniform float znear; 
uniform float zfar; 

float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
} 

void main() {

    vec4 RawNormal = texture(InputNormal, TexCoord); 

    float Roughness = RawNormal.w; 
    
    Packed = vec4(RawNormal.xyz, LinearDepth(texture(InputDepth, TexCoord).x)); 
}