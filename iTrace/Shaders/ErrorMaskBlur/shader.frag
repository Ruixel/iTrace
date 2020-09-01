#version 330

out float OutError; 
uniform sampler2D InError; 
uniform ivec2 Resolution; 
uniform bool Vertical; 

void main() {
	
	OutError = 0.0; 

	for(int Pixel = -6; Pixel <= 6; Pixel++) {
	
		ivec2 SamplePixel = ivec2(gl_FragCoord) + ivec2(!Vertical ? Pixel : 0, Vertical ? Pixel : 0); 

		float SampleError = texelFetch(InError, SamplePixel, 0).x; 

		//if(SamplePixel[int(Vertical)] < 0 || SamplePixel[int(Vertical)] > Resolution[int(Vertical)]) 
		//	SampleError = 1.0; 

		OutError += SampleError / pow(float(abs(Pixel)+1.0),1.5); 

	}

	OutError = clamp(OutError, 0.0, 1.0); 

}