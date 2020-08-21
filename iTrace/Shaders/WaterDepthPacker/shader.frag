#version 330 

layout(location = 0) out float CombinedDepth;

uniform float znear; 
uniform float zfar; 

float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
}

uniform sampler2D Depth; 
uniform sampler2D RefractedDepth; 

void main() {
	
	ivec2 RawPixel = ivec2(gl_FragCoord) * 2; 

	float BaseDepth = texelFetch(Depth, RawPixel, 0).x; 
	float RefractedDepth = texelFetch(RefractedDepth, RawPixel, 0).x; 

	if(BaseDepth < RefractedDepth) {
		CombinedDepth = LinearDepth(BaseDepth); 
	}
	else {
		CombinedDepth = LinearDepth(RefractedDepth); 
	}
	CombinedDepth = LinearDepth(BaseDepth); 
}