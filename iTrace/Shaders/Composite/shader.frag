#version 430 core
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord; 
layout(location = 0) out vec4 Lighting;

layout(binding = 0, std140) buffer AVGColorBuffer {
	vec4 AverageColor; 
};


//base data: 
uniform sampler2D CombinedLighting; 
uniform sampler2D Depth; 

//Post Process
uniform sampler2D Glow[4]; 
uniform sampler2D DoF; 
uniform sampler2D Lensflare; 

//upscaling tricksteries! 
uniform sampler2D LensDirt; 

uniform sampler2D PreviousComposite; 

uniform ivec2 FocusPoint; 

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

float RRTAndODTFit(float v) {
	float a = v * (v + 0.0245786f) - 0.000090537f;
    float b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

float ODTtoRRT(float v) {
	
	float f1 = 0.4329510f * v - 0.0245786f; 
	float f2 = 4.0 * (0.983729f*v-1) * (0.238081f*v+0.000090537f); 
	float f3 = 0.4329510f * v; 
	float f4 = 0.0245786f; 

	return (-sqrt(f1*f1 - f2) - f3 + f4) / (2.0 * (0.983729f*v-1)); 

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


vec4 cubic(float v){
    vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
    vec4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return vec4(x, y, z, w) * (1.0/6.0);
}

vec4 textureBicubic(sampler2D sampler, vec2 texCoords){

   vec2 texSize = textureSize(sampler, 0);
   vec2 invTexSize = 1.0 / texSize;

   texCoords = texCoords * texSize - 0.5;


    vec2 fxy = fract(texCoords);
    texCoords -= fxy;

    vec4 xcubic = cubic(fxy.x);
    vec4 ycubic = cubic(fxy.y);

    vec4 c = texCoords.xxyy + vec2 (-0.5, +1.5).xyxy;

    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4 (xcubic.yw, ycubic.yw) / s;

    offset *= invTexSize.xxyy;

    vec4 sample0 = texture(sampler, offset.xz);
    vec4 sample1 = texture(sampler, offset.yz);
    vec4 sample2 = texture(sampler, offset.xw);
    vec4 sample3 = texture(sampler, offset.yw);

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return mix(
       mix(sample3, sample2, sx), mix(sample1, sample0, sx)
    , sy);
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
    vec4 DofFetch = textureBicubic(DoF, TexCoord); 

	// DofFetch = texelFetch(DoF, ivec2(gl_FragCoord.xy)/4,0); 

    float LensDirtFetch = texture(LensDirt, TexCoord*.4).x * 0.5 + 0.5; 


    if(Depth > 10.0) {
    
        float Mix = min((Depth-10.0)/2.0,1.0); 


    }

	if(DoGlow) {
		vec4 Glow1 = texture(Glow[0], TexCoord); 
		vec4 Glow2 = texture(Glow[1], TexCoord); 
		vec4 Glow3 = texture(Glow[2], TexCoord); 
		vec4 Glow4 = texture(Glow[3], TexCoord); 

		Lighting.xyz += (Glow1.xyz / Glow1.w + Glow2.xyz / Glow2.w + Glow3.xyz / Glow3.w + Glow4.xyz / Glow4.w) * 0.25; 

	}
	Lighting.xyz = mix(DofFetch.xyz,Lighting.xyz,pow(1.0-min(DofFetch.w/2.0,1.0),2.0)); 


	ivec2 Pixel = ivec2(gl_FragCoord); 

	if(Pixel.x == FocusPoint.x && Pixel.y == FocusPoint.y) {
		AverageColor = textureLod(PreviousComposite, TexCoord, 10.0); 
	}

	//Lighting.xyz = textureLod(PreviousComposite, TexCoord, 10.0).xyz; 

  //  Lighting = ivec4(Lighting * 16 + (Noise(gl_FragCoord.xy) * 2.0)) / 16.0f; 

    
}