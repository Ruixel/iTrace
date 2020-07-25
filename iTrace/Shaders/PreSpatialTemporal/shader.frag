#version 420

in vec2 TexCoord; 

layout(location = 0) out vec4 TemporallyFilteredDiffuse; 
layout(location = 1) out vec4 TemporallyFilteredSpecular;
layout(location = 2) out vec4 TemporallyFilteredDetail; 
layout(location = 3) out vec4 LSVGFDiffuse; //<- L-SVGF tricked diffuse SVGF(Diffuse / Detail) * Detail

uniform sampler2D CurrentDiffuse; 
uniform sampler2D CurrentSpecular; 
uniform sampler2D CurrentDetail; 

uniform sampler2D PreviousDiffuse; 
uniform sampler2D PreviousSpecular;
uniform sampler2D PreviousDetail; 

uniform sampler2D SpatialDenoiseData; 

uniform sampler2D MotionVectors; 
uniform sampler2D FrameCount; 

uniform int SubFrame; 


ivec2 States[] = ivec2[](
	ivec2(1, 1),
	ivec2(0, 1),
	ivec2(0, 0),
	ivec2(1, 0));

float GetRoughness(inout vec3 Normal) {

	float L = length(Normal); 

	Normal /= L; 
	
	float Roughness = 1.0 - (L * 2.0 - 1.0); 
	return Roughness; 
}

float Kernel[4] = float[](0.60933016268,0.80238141513,0.904644388297,1.0); 

void main() {

	vec2 MotionVectors = texture(MotionVectors, TexCoord).xy; 
	
	float FrameCount = max(texture(FrameCount, TexCoord + MotionVectors).x - 1.0,0.0); 
	
	if(MotionVectors.x < -.5 || MotionVectors.y < -.5) {
		FrameCount = 0; 
	}

	//if required, spatially filter the detail using a 3x3 bilateral filter 
	
	ivec2 Pixel = ivec2(gl_FragCoord.xy); 

	TemporallyFilteredDetail = texelFetch(CurrentDetail, Pixel, 0); 

	if(FrameCount < 4.0) {
		
		ivec2 HighResPixel = Pixel * 2 + States[SubFrame];

		vec4 BasePacked = texelFetch(SpatialDenoiseData, HighResPixel, 0); 

		float BaseRoughness = GetRoughness(BasePacked.xyz); 

		float TotalWeight = 1.0; 

		for(int x = -1; x <= 1; x++) {
			for(int y = -1; y <= 1; y++) {
				
				ivec2 PixelOffet = Pixel + ivec2(x,y); 
				ivec2 PixelOffsetHighRes = HighResPixel + ivec2(x,y)*2; 

				vec4 CurrentPacked = texelFetch(SpatialDenoiseData, PixelOffsetHighRes, 0); 
				vec4 CurrentDetail = texelFetch(CurrentDetail, PixelOffet, 0); 

				float CurrentRoughness = GetRoughness(CurrentPacked.xyz); 

				//basic spatial filter -> 

				float NormalWeight = pow(max(dot(CurrentPacked.xyz, BasePacked.xyz), 0.0), 16.0); 

				float Weight = NormalWeight;  

				Weight /= pow(1.0 + 3.0 * abs(CurrentPacked.w - BasePacked.w), 4.0); 

				TemporallyFilteredDetail += CurrentDetail * Weight; 
				TotalWeight += Weight; 


			}
		}

		TemporallyFilteredDetail /= TotalWeight; 

	}

	//temporally filter the signals -> 

	float MixFactor = min(FrameCount / (FrameCount+1.0),0.95); //<- should be enough to guide the spatial filter without impossible ghosting 

	TemporallyFilteredDiffuse = mix(texture(CurrentDiffuse, TexCoord), texture(PreviousDiffuse, TexCoord+MotionVectors), MixFactor); 
	TemporallyFilteredSpecular = mix(texture(CurrentSpecular, TexCoord), texture(PreviousSpecular, TexCoord+MotionVectors), MixFactor); 
	TemporallyFilteredDetail = mix(TemporallyFilteredDetail, texture(PreviousDetail, TexCoord+MotionVectors), MixFactor); 

	//-> convert detail + raw diffuse into L-SVGF friendly signal 

	LSVGFDiffuse = TemporallyFilteredDiffuse; 
	//LSVGFDiffuse.xyz = LSVGFDiffuse.xyz / (TemporallyFilteredDetail.xyz * TemporallyFilteredDetail.w); 

}