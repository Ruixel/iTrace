#version 330
in vec2 TexCoord; 
out vec4 OutPut; 

uniform sampler2D Input; 
float GetWeight(float MeanDepth, float CurrentDepth, float StandardDeviation) {
	
	return 1.0 / pow(1.0 + 3.0 * abs(MeanDepth-CurrentDepth),1.0); 

}

ivec2 Offsets[3] = ivec2[3](ivec2(1,0), ivec2(0,1), ivec2(0,0)); 


float Deviation(float a, float b) {
	return (a-b) * (a-b); 
}

void main() {

	ivec2 Pixel = ivec2(gl_FragCoord.xy); 
	
	vec4 Data00 = texelFetch(Input, Pixel * 2, 0); 
	vec4 Data10 = texelFetch(Input, Pixel * 2 + ivec2(1,0), 0); 
	vec4 Data01 = texelFetch(Input, Pixel * 2 + ivec2(0,1), 0); 
	vec4 Data11 = texelFetch(Input, Pixel * 2 + ivec2(1,1), 0); 

	float MeanDepth = (Data00.w + Data10.w + Data01.w + Data11.w) / 4.0; 

	float StandardDeviation = sqrt((Deviation(Data00.w, MeanDepth) + Deviation(Data10.w, MeanDepth) + Deviation(Data01.w, MeanDepth)  +Deviation(Data11.w, MeanDepth)) / 4.0); 


	float Weight00 = GetWeight(MeanDepth, Data00.w,StandardDeviation); 
	float Weight10 = GetWeight(MeanDepth, Data10.w,StandardDeviation); 
	float Weight01 = GetWeight(MeanDepth, Data01.w,StandardDeviation); 
	float Weight11 = GetWeight(MeanDepth, Data11.w,StandardDeviation); 

	OutPut.xyz = vec3(0.0); 
	OutPut.xyz += Data00.xyz * Weight00; 
	OutPut.xyz += Data10.xyz * Weight10; 
	OutPut.xyz += Data01.xyz * Weight01; 
	OutPut.xyz += Data11.xyz * Weight11; 
	OutPut.xyz /= (Weight00 + Weight10 + Weight01 + Weight11); 
	OutPut.w = Data00.w; 
	//OutPut.xyz = vec3(MeanDepth); 
}