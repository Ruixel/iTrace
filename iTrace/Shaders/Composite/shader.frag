#version 330
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord; 
layout(location = 0) out vec4 Lighting;

//base data: 
uniform sampler2D CombinedLighting; 
uniform sampler2D Depth; 

//Post Process
uniform sampler2D Glow; 
uniform sampler2D DoF; 
uniform sampler2D Lensflare; 

//upscaling tricksteries! 
uniform sampler2D LensDirt; 

uniform float znear; 
uniform float zfar; 

uniform bool DoGlow; 

float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
}


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

vec3 ACESFitted(vec3 Color, float Exposure)
{
    Color.rgb *= Exposure;
    
    Color.rgb = ACESInputMat * Color.rgb;
    Color.rgb = RRTAndODTFit(Color.rgb);
    Color.rgb = ACESOutputMat * Color.rgb;
    Color.rgb = clamp(Color.rgb, 0.0, 1.0);
    Color.rgb = pow( Color.rgb, vec3( 0.45 ) );

    return Color;
}












float DitherMask[4] = float[4](0.0,0.25,0.5,0.75); 


float Noise(vec2 position_screen)
{
  vec3 magic = vec3(0.06711056f, 0.00583715f, 52.9829189f);
  return fract(magic.z * fract(dot(position_screen, magic.xy)));
}





void main() {
    
    float Depth = LinearDepth(texture(Depth, TexCoord).x); 

	Lighting = texture(CombinedLighting, TexCoord);
    vec3 DofFetch = texture(DoF, TexCoord).xyz; 

    float LensDirtFetch = texture(LensDirt, TexCoord*.4).x * 0.5 + 0.5; 


    if(Depth > 10.0) {
    
        float Mix = min((Depth-10.0)/2.0,1.0); 


    }
    if(DoGlow) 
        Lighting += texture(Glow, TexCoord) * LensDirtFetch; 

    Lighting.xyz = ACESFitted(Lighting.xyz, 1.0); 

    int DitherAddon = (int(gl_FragCoord.x)%2) * 2 + (int(gl_FragCoord.y)%2); 

  //  Lighting = ivec4(Lighting * 16 + (Noise(gl_FragCoord.xy) * 2.0)) / 16.0f; 

    
}