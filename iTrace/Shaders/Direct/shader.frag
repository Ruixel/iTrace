#version 330
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord; 

//stores the visibility term for the direct shadows 
layout(location = 0) out float Visibility;


uniform sampler2D WorldPosition; 
uniform sampler2D LowFrequencyNormal; 

uniform sampler2D ShadowMaps[5]; 
uniform mat4 ShadowMatrices[5]; 

uniform sampler2D Sobol;
uniform sampler2D Ranking;
uniform sampler2D Scrambling;

uniform sampler2D FrameCount; 

uniform sampler2DArray HemisphericalShadowMaps;  
uniform mat4 HemisphericalMatrices[48]; 
uniform vec3 HemisphericalDirections[48]; 

uniform int Frame; 

sampler2D GetFromIdx(int Idx) {
	return Idx == 0 ? ShadowMaps[0] : 
		Idx == 1 ? ShadowMaps[1] : 
		Idx == 2 ? ShadowMaps[2] : 
		Idx == 3 ? ShadowMaps[3] : 
		ShadowMaps[4]; 
}

float FetchSize = 0.01; 

int Radius = 2; 

int FetchFromTexture(sampler2D Texture, int Index) {

	int Width = 256;
	int Height = textureSize(Texture, 0).y;

	ivec2 Pixel = ivec2(Index % Width, Index / Width);

	return clamp(int(texelFetch(Texture, ivec2(Pixel), 0).x * 255.0 + .1), 0, 255);

}


float samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_32spp(int pixel_i, int pixel_j, int sampleIndex, int sampleDimension)
{
	// wrap arguments
	pixel_i = pixel_i & 127;
	pixel_j = pixel_j & 127;
	sampleIndex = sampleIndex & 255;
	sampleDimension = sampleDimension & 255;

	// xor index based on optimized ranking


	int rankedSampleIndex = sampleIndex ^ FetchFromTexture(Ranking, sampleDimension + (pixel_i + pixel_j * 128) * 8);

	// fetch value in sequence
	int value = FetchFromTexture(Sobol, sampleDimension + rankedSampleIndex * 256);

	// If the dimension is optimized, xor sequence value based on optimized scrambling
	value = value ^ FetchFromTexture(Scrambling, (sampleDimension % 8) + (pixel_i + pixel_j * 128) * 8);

	// convert to float and return
	float v = (0.5f + value) / 256.0f;
	return v;
}




float hash(ivec2 Pixel, int Index, int Dimension) {

	return samplerBlueNoiseErrorDistribution_128x128_OptimizedFor_2d2d2d2d_32spp(Pixel.x, Pixel.y, Index, Dimension);

}

vec2 hash2(ivec2 Pixel, int Index, int Dimension) {

	vec2 Returnhash;
	Returnhash.x = hash(Pixel, Index, Dimension*2);
	Returnhash.y = hash(Pixel, Index, Dimension*2+1);

	return Returnhash;

}



float MultiShadowPass(vec3 WorldPos, int Samples, float MaxRadius) {
	
	for(int Pass = 0; Pass < 5; Pass++) {
		
		vec4 ShadowClip = ShadowMatrices[Pass] * vec4(WorldPos, 1.0); 

		vec3 ShadowNDC = ShadowClip.xyz / ShadowClip.w; 

		if(abs(ShadowNDC.x) < .95 && abs(ShadowNDC.y) < .95) {
			
			int Total = (Radius * 2 + 1); 
			Total = Total * Total; 

			float Shadow = 0.0; 

			for(int index = 0; index < Samples; index++) {
					
				vec2 hash = hash2(ivec2(gl_FragCoord.xy), Frame, (index+1)); 

				float Angle = hash.x * 6.28; 

				float Radius = sqrt(hash.y); 

				vec2 Offset = vec2(cos(Angle), sin(Angle)) * min(Radius,MaxRadius) * FetchSize; 


				float zFetch = texture(GetFromIdx(Pass), ShadowNDC.xy * 0.5 + 0.5 + Offset).x;
					
				if(zFetch < 1.0 / ((ShadowNDC.z * 0.5 + 0.5) - (0.000009 + 0.000009 * Radius) * (Pass+1)))
					Shadow += 1.0; 

			}




			return Shadow / float(Samples); 

		}


	}

	return 1.0; 

}



float GetShadowMapVisibilityTerm(vec3 WorldPos) {
	

	int Frame = int(max(texture(FrameCount, TexCoord).x-2.0,0.0)); 

	float MaxRadius = mix(0.0, 1.0, float(Frame) / float(Frame + 1.0)); 
	MaxRadius *= MaxRadius * MaxRadius; 

	return MultiShadowPass(WorldPos, (Frame == 1 ? 3 : 1),MaxRadius); 
}

float GetLocalVisibilityTerm(vec3 WorldPos, vec3 Normal) {
	//first, check if we are at the edge of the screen. If we are close enough to the edge, 
	//we probably won't get anything useful out of the screen-space tracing, 
	//so, we can early exit 

	if(abs(TexCoord.x * 2.0 - 1.0) > 0.9 || abs(TexCoord.y * 2.0 - 1.0) > 0.9)
		return 1.0; //...early exit 

	//as this is temporally filtered after the fact, there is no reason to "smoothly" 
	//mix between early exiting and not, as any direct cutoffs will be temporally blurred anyway. 

	




}

float GetHemisphericalShadowMaphit(vec3 WorldPos, vec3 Normal) {
	
	float Noise = hash(ivec2(gl_FragCoord).xy, Frame, 0); 

	int Sample = Frame % 48; 

	vec4 ClipSpace = HemisphericalMatrices[Sample] * vec4(WorldPos, 1.0); 

	ClipSpace.xyz /= ClipSpace.w; 

	ClipSpace.xyz = ClipSpace.xyz * 0.5 + 0.5; 

	float zFetch = texture(HemisphericalShadowMaps, vec3(ClipSpace.xy, Sample)).x;

	float Weight = 3.14159265 * max(dot(HemisphericalDirections[Sample], Normal), 0.0); 

	return (zFetch + 0.00009 > ClipSpace.z) ? Weight : 0.0; 


}



void main() {

	vec3 WorldPos = texture(WorldPosition, TexCoord).xyz; 

	//step 1 -> compute non-local visibility term from shadow map 

	Visibility = GetShadowMapVisibilityTerm(WorldPos); 

	//step 2 -> if needed, compute local visibility term from screen-space shadow tracing 
	//this is a battle-tested method, used by many, engines and shader packs. 
		vec3 Normal = texture(LowFrequencyNormal, TexCoord).xyz; 

	if(Visibility > 0.01) {
		Visibility *= GetLocalVisibilityTerm(WorldPos,Normal); 
	}

	Visibility = GetHemisphericalShadowMaphit(WorldPos, Normal); 
}