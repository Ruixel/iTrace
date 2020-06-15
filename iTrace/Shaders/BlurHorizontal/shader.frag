#version 330 

layout(location = 0) out vec3 Lighting; 

in vec2 TexCoord; 

uniform sampler2D Input; 
uniform vec2 InverseRes; 

uniform int j; 

float Weights[45] = float[45](
0.000007,0.000116,0.001227,0.008465,0.037975,0.110865,0.210786,0.261117,0.210786,0.110865,0.037975,0.008465,0.001227,0.000116,0.000007,0.000489,0.002403,0.009246,0.02784,0.065602,0.120999,0.174697,0.197448,0.174697,0.120999,0.065602,0.02784,0.009246,0.002403,0.000489,0.009033,0.018476,0.033851,0.055555,0.08167,0.107545,0.126854,0.134032,0.126854,0.107545,0.08167,0.055555,0.033851,0.018476,0.009033); 




void main() {
	Lighting = vec3(0.0); 

	float TotalWeight = 0.0; 

	for(int x = -7; x <= 7; x++) {

		float Weight = Weights[x+7 + j * 15]; 
		
		TotalWeight += Weight; 
		Lighting += textureLod(Input, TexCoord + vec2(x,0) * InverseRes,2.0).xyz * Weight; 		
	}
	Lighting /= TotalWeight; 

}