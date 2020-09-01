#version 330
in vec2 TexCoord; 

layout(location = 0) out vec4 UpscaledClouds; 
layout(location = 1) out vec4 UpscaledPrespatial; 
layout(location = 2) out vec4 TemporalPrespatial; 
layout(location = 3) out float FrameCount; 

uniform sampler2D Clouds; 
uniform sampler2D PreviousClouds; 
uniform sampler2D MotionVectorsClouds; 
uniform sampler2D MotionVectors; 
uniform sampler2D Detail; 
uniform sampler2D PreviousDetail; 
uniform sampler2D PreviousTemporalDetail; 
uniform sampler2D PreviousFrameCount; 

uniform int Frame; 

int Random[16] = int[16](2,15,4,1,3,8,14,11,12,5,0,6,10,7,13,9); 

ivec2 States[] = ivec2[](
	ivec2(1, 1),
	ivec2(0, 1),
	ivec2(0, 0),
	ivec2(1, 0));

void main() {
	
	ivec2 Pixel = ivec2(gl_FragCoord); 
	
	ivec2 TextureSize = ivec2(textureSize(PreviousClouds, 0)); 

	vec2 SampleMotionVectors = texelFetch(MotionVectorsClouds, Pixel, 0).xy; 

	ivec2 MotionPixel = Pixel + ivec2(SampleMotionVectors * TextureSize); 


	bool NewFrame = SampleMotionVectors.x < -.9 || SampleMotionVectors.y < -.9; 

	if(MotionPixel.x < 0 || MotionPixel.y < 0 || MotionPixel.x >= TextureSize.x || MotionPixel.y >= TextureSize.y)
		NewFrame = true; 

	vec4 Previous = texture(PreviousClouds, TexCoord + SampleMotionVectors, 0); 
	vec4 CurrentClouds = texelFetch(Clouds, Pixel/ 4, 0); 

	ivec2 PixelOffset = Pixel % 4; 


	int UpscaleFrame = Random[Frame%16]; 


	ivec2 Jitter = ivec2(UpscaleFrame % 4, (UpscaleFrame / 4)%4); 

	UpscaledClouds = Previous; 

	if(PixelOffset == Jitter || NewFrame) 
		UpscaledClouds = CurrentClouds; 

	vec4 CurrentDetail = texelFetch(Detail, Pixel / 2, 0); 

	SampleMotionVectors = texelFetch(MotionVectors, Pixel, 0).xy; 

	NewFrame = SampleMotionVectors.x < -.9 || SampleMotionVectors.y < -.9; 
	
	vec4 PreviousDetail = texture(PreviousDetail, TexCoord + SampleMotionVectors); 


	TemporalPrespatial = PreviousDetail; 

	PixelOffset = Pixel % 2; 

	if(PixelOffset == States[Frame%4] || NewFrame) 
		TemporalPrespatial = CurrentDetail; 

	UpscaledPrespatial = TemporalPrespatial; 

	FrameCount = texture(PreviousFrameCount, TexCoord + SampleMotionVectors).x + 1.0; 

	if(NewFrame)
		FrameCount = 0.0; 

	float MixFactor = min(float(FrameCount) / float(FrameCount+1.0), 0.975); 

	TemporalPrespatial = mix(UpscaledPrespatial, texture(PreviousTemporalDetail, TexCoord + SampleMotionVectors), MixFactor); 
	

}