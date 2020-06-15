#version 330 

layout(location = 0) out vec3 Lighting; 

in vec2 TexCoord; 

uniform sampler2D Input; 
uniform sampler2D Depth; 
uniform vec2 InverseRes; 

uniform float znear; 
uniform float zfar; 

float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
}

float LinearDepth(vec2 TC) {
	return LinearDepth(texture(Depth, TC).x); 
}



//precalculated offsets (for where in the kernel to pick) 

int Offsets[7] = int[](33, 30, 26, 21, 15, 8, 0); 

float Weights[35] = float[](
0.009033,0.018476,0.033851,0.055555,0.08167,0.107545,0.126854,0.134032, //for 15x15 blur 
0.018816,0.034474,0.056577,0.083173,0.109523,0.129188,0.136498, //for 13x13 blur
0.035822,0.05879,0.086425,0.113806,0.13424,0.141836, //for 11x11 blur
0.063327,0.093095,0.122589,0.144599,0.152781, //for 9x9 blur 
0.106595,0.140367,0.165569,0.174938, //for 7x7 blur
0.1784,0.210431,0.222338, //for 5x5 blur
0.327162,0.345675); //for 3x3 blur


//x is radius, y is kernel offset 
ivec2 GetBlurData(float Depth) {
	
	float Distance = max(Depth-10.0,0.0);

	int UseKernel = min(int(Distance/4.0), 6); 

	return ivec2(UseKernel+1, Offsets[UseKernel]); 
}


void main() {
	Lighting = vec3(0.0); 

	float TotalWeight = 0.0; 

	float BaseDepth = LinearDepth(TexCoord); 

	BaseDepth = min(BaseDepth, 34.0); 

	ivec2 Data = GetBlurData(BaseDepth); 

	for(int x = -Data.x; x <= Data.x; x++) {

		vec2 CurrentTC = TexCoord + vec2(x, 0) * InverseRes; 

		float CurrentDepth = LinearDepth(CurrentTC); 

		CurrentDepth = min(CurrentDepth, 34.0); 
		
		
		float WeightedDepth = 1.0 / (1.0 + abs(CurrentDepth-BaseDepth)/clamp(BaseDepth, 1.0, CurrentDepth)); 

		float Weight = Weights[(Data.x-abs(x)) + Data.y] * WeightedDepth; 
		
		TotalWeight += Weight; 
		Lighting += textureLod(Input, CurrentTC,1.0).xyz * Weight; 		
	}
	Lighting /= TotalWeight; 
}