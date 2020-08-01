#version 330
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord; 
layout(location = 0) out vec4 Lighting;

uniform sampler2D Composited; 
uniform float Exposure; 

mat3 ACESInputMat = mat3(
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3 ACESOutputMat = mat3(
    1.60475, -0.10208, -0.00327,
    -0.53108, 1.10813, -0.07276,
    -0.07367, -0.00605, 1.07602
);

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACESFitted(vec3 Color)
{
    Color.rgb *= Exposure;
    
    Color.rgb = ACESInputMat * Color.rgb;
    Color.rgb = RRTAndODTFit(Color.rgb);
    Color.rgb = ACESOutputMat * Color.rgb;

    Color.rgb = clamp(Color.rgb, 0.0, 1.0);
    Color.rgb = pow( Color.rgb, vec3( 0.45 ) );

    return Color;
}

void main() {
	Lighting = vec4(ACESFitted(texture(Composited,TexCoord).xyz), 1.0);  
}