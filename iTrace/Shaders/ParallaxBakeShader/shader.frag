#version 420

in vec2 TexCoord; 

//data is stored via a single byte (constant precision, ranges between 0 - 1, precision is 1/256) 
layout(location = 0) out float data;

uniform int TextureSize; 
uniform int RayCount; 
uniform float Strength; 

uniform sampler2D DepthTexture; 

const float StepSize = 1.0 / 512.0; 

void main() {

	//figure out our coordinate on the texture! 

	ivec2 Pixel = ivec2(gl_FragCoord.xy); 
	int Ray = Pixel.x / (TextureSize+2); 

	ivec2 PixelActual = Pixel;
	PixelActual.x = PixelActual.x % (TextureSize+2); 
	PixelActual.y = PixelActual.y % TextureSize; 
	
	if(PixelActual.x == 0) {
		PixelActual.x = TextureSize-1; 
	}
	else if(PixelActual.x == TextureSize+1) {
		PixelActual.x = 0; 
	}
	else {
		PixelActual.x -= 1; 
	}


	vec2 TextureCoordActual = vec2(PixelActual) / vec2(TextureSize); 

	float RayAngle = (float(Ray) / float(RayCount)) * 6.28318531; 

	vec2 Direction = vec2(cos(RayAngle), sin(RayAngle)); 

	float Height = 1.0; //how should height be related to traversal distance? 
	//start off by using a linear relationship, then tweak until it looks good. 

	vec2 Point = TextureCoordActual; 

	float TextureHeight = textureLod(DepthTexture, Point, 0.0).x; 


	data = 0.0; 

	while(Height > TextureHeight) {
	
		Height -= StepSize * mix(15.0,2.0,Strength); 
		data += StepSize; 
		Point += Direction * StepSize; 
		TextureHeight = textureLod(DepthTexture, Point, 0.0).x; 

	}
	
	data *= 2.0; 


}