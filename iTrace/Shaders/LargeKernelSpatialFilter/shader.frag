#version 330

in vec2 TexCoord; 

layout(location = 0) out vec4 IndirectDiffuse;

uniform sampler2D InputPacked;
uniform sampler2D InputLighting; 
uniform sampler2D FrameCount; 

uniform int Radius; 
uniform int StepSize; 
uniform mat4 IncidentMatrix; 
uniform int SubFrame; 

ivec2 States[] = ivec2[](
	ivec2(1, 1),
	ivec2(0, 1),
	ivec2(0, 0),
	ivec2(1, 0));

float Kernel[4] = float[](0.106595,0.140367,0.165569,0.174938); 



float GetRoughness(inout vec3 Normal) {

	float L = length(Normal); 

	Normal /= L; 
	
	float Roughness = 1.0 - (L * 2.0 - 1.0); 
	return Roughness; 
}



void main() {

	
	ivec2 Pixel = ivec2(gl_FragCoord.xy); 
	
	ivec2 HighResPixel = Pixel * 2 + States[SubFrame];

	vec4 BasePacked = texelFetch(InputPacked, HighResPixel, 0); 

	float TotalWeight = Kernel[3] * Kernel[3]; 
	float TotalWeightAO = TotalWeight; 


	IndirectDiffuse =  texelFetch(InputLighting, Pixel, 0) * TotalWeight; 

	float BaseRoughness = GetRoughness(BasePacked.xyz); 
	
	float Frame = texelFetch(FrameCount, HighResPixel, 0).x;
	
	if(Frame > 7.0 && StepSize > 5) {
	
		IndirectDiffuse /= TotalWeight; 
		return; 
	}

	for(int x = -2; x <= 2; x++) {
		for(int y = -2; y <= 2; y++) {
			
			if(x == 0 && y == 0)  
				continue; 
				
			ivec2 NewPixel = Pixel + ivec2(x,y) * StepSize;
			ivec2 NewPixelHighRes = HighResPixel + ivec2(x,y) * StepSize * 2; 

			vec4 CurrentPackedData = texelFetch(InputPacked, NewPixelHighRes, 0); 

			float CurrentRoughness = GetRoughness(CurrentPackedData.xyz); 

			float Weight = pow(max(dot(CurrentPackedData.xyz, BasePacked.xyz), 0.0), 16.0); 

			Weight /= pow(1.0 + 3.0 * abs(CurrentPackedData.w - BasePacked.w), 4.0); 

			float KernelWeight = Kernel[3-abs(x)] * Kernel[3-abs(y)]; 

			 

			vec4 LightingSample = texelFetch(InputLighting, NewPixel, 0); 


			IndirectDiffuse.xyz += LightingSample.xyz * Weight * KernelWeight; 
			TotalWeight += Weight * KernelWeight; 

			//AO: 

			if((StepSize == 1 && Frame < 20.0) || (StepSize == 2 && Frame < 12.0) || Frame < 8.0) {

				float WeightAO = Weight * pow(Kernel[3-abs(x)] * Kernel[3-abs(y)], mix(1.0, 1.5, clamp(Frame/30.0,0.0,1.0))); 

				IndirectDiffuse.w += LightingSample.w * WeightAO; 
				TotalWeightAO += WeightAO; 
			}


		}
	}
	IndirectDiffuse.xyz /= TotalWeight; 
	IndirectDiffuse.w /= TotalWeightAO; 
}