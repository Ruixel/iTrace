#version 330 

layout(location = 0) out vec4 Lighting; 

in vec2 TexCoord; 

uniform sampler2D Input; 
uniform sampler2D Depth; 
uniform vec2 InverseRes; 
const int Samples = 12;

uniform float znear; 
uniform float zfar; 

uniform float MaxRadius; 
const float ApertureDistance = 40.0; 

uniform float Aperture; 
uniform float ImageDistance; 
uniform float FocusPoint; 



float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
}

float LinearDepth(vec2 TC) {
	return LinearDepth(texture(Depth, TC).x); 
}

//https://en.wikipedia.org/wiki/Depth_of_field
//https://developer.nvidia.com/sites/all/modules/custom/gpugems/books/GPUGems/gpugems_ch23.html
//http://fileadmin.cs.lth.se/cs/Education/EDAN35/lectures/12DOF.pdf

float GetDofRadius(float CurrentDepth, float FocusDepth) {


	//1 / P + 1 / I = 1 / F

	float InverseImageD = 1.0 / ImageDistance; 
	float InverseFocus = 1.0 / FocusDepth; 

	float FocalLength = 1.0 / (InverseImageD + InverseFocus); 


	CurrentDepth = max(CurrentDepth, 0.01); 
	FocusDepth = max(FocusDepth, 0.01); 

	return min(abs(Aperture * (FocalLength * (CurrentDepth - FocusDepth)) / (CurrentDepth * (FocusDepth - FocalLength))), MaxRadius); 

}



float GetRadius() {
	

	float DepthMiddle = LinearDepth(texture(Depth, vec2(.5)).x); 
	float DepthCurrent = LinearDepth(texture(Depth, TexCoord).x); 

	//look at the difference 

	float Difference = abs(DepthMiddle-DepthCurrent); 

	return mix(0.0,MaxRadius,min(Difference,ApertureDistance)/ApertureDistance);  


}

float Hash(vec2 seed) {
	
	seed = fract(seed * vec2(5.3987, 5.4421));
    seed += dot(seed.yx, seed.xy + vec2(21.5351, 14.3137));
	return fract(seed.x * seed.y * 95.4307);

}


vec3 Blur(vec2 Direction, float Radius, float Hash, float BaseDepth) {
	
	vec2 Start = TexCoord - 0.5 * Radius * Direction; 
	vec2 End = TexCoord + 0.5 * Radius * Direction; 
	
	vec2 Size = vec2(textureSize(Input, 0).xy); 

	vec2 AspectCorrect = vec2(Size.x/Size.x, 1.0); 




	vec2 Step = ((End-Start) / float(Samples)) * AspectCorrect; 

	vec2 SamplePosition = Start + Hash * Step; 
	
	vec3 Result = vec3(0.); 

	int SuccessFulSamples = 0; 

	for(int i=0;i<Samples;i++) {
		
		SuccessFulSamples ++; 

		Result += texture(Input, SamplePosition).xyz; 

		SamplePosition += Step; 

	}



	return SuccessFulSamples == 0 ? texture(Input, TexCoord).xyz : Result / vec3(SuccessFulSamples); 

}

float seed;

vec2 hash2White() {
	return fract(sin(vec2(seed += 0.1, seed += 0.1)) * vec2(43758.5453123, 22578.1459123));
	}vec2 Kernels[30] = vec2[30](
vec2(0.0625,0),
vec2(0.257919,0.176448),
vec2(0.131361,0.337862),
vec2(-0.0921431,0.394999),
vec2(-0.327485,0.300003),
vec2(-0.474508,0.0676617),
vec2(-0.459069,-0.226504),
vec2(-0.265963,-0.472752),
vec2(0.0499406,-0.569048),
vec2(0.379882,-0.462599),
vec2(0.599794,-0.174604),
vec2(0.61739,0.202347),
vec2(0.409938,0.534691),
vec2(0.0376871,0.695925),
vec2(-0.373478,0.614816),
vec2(-0.675182,0.305508),
vec2(-0.750517,-0.132756),
vec2(-0.559119,-0.547683),
vec2(-0.156106,-0.787305),
vec2(0.323357,-0.755836),
vec2(0.709687,-0.451445),
vec2(0.859202,0.0287391),
vec2(0.707557,0.519613),
vec2(0.296475,0.845165),
vec2(-0.237045,0.881797),
vec2(-0.706533,0.605069),
vec2(-0.941473,0.102269),
vec2(-0.849325,-0.454972),
vec2(-0.451564,-0.869457),
vec2(0.120156,-0.988399));

//partially based on http://tuxedolabs.blogspot.com/2018/05/bokeh-depth-of-field-in-single-pass.html
vec3 DepthOfFieldNew(float Radius, vec2 Hash, float CenterDepth) {
	
	
	vec3 Result = vec3(0.0); 

	vec2 TexelSize = 1.0 / vec2(textureSize(Input, 0)); 

	float Samples = 0.0; 


	for(int i = 0; i < 30; i++) {
		
		vec2 Coord = TexCoord + Kernels[i] * TexelSize * Radius * 0.125; 

		vec4 Input = texture(Input, Coord); 

		float CurrentRadius = GetDofRadius(Input.w, FocusPoint); 


		

		float Weight = 1.0 / (1.0 + abs(CurrentRadius-Radius)/min(CurrentRadius, Radius));  

		if(Samples > 0.0) {
			Result += mix(Result/Samples,Input.xyz,Weight); 
			Samples += 1.0; 
		}
		else {
			Result += Input.xyz * Weight; 
			Samples += Weight; 
		}

	}

	return Result / Samples; 

}


void main() {

	seed = (TexCoord.x * TexCoord.y) * 500.0 * 20.0;

	ivec2 HighResPixel = ivec2(gl_FragCoord.xy) * 4; 

	float BaseDepth = texture(Input, TexCoord).w; 

	//float FocusPoint = LinearDepth(texelFetch(Depth, ivec2(960,540),0).x); 

//	float FocusPercentage =  1.-pow(1.-clamp(min(abs(FocusPoint-LinearDepth(BaseDepth)),ApertureDistance)/ApertureDistance,0.0,1.0),8.); 

 
	//float Radius = mix(0.,20.0,clamp(FocusPercentage,0.0,1.0)); 
	
	float Radius = GetDofRadius(BaseDepth, FocusPoint); 

//	float HashSample = Hash(TexCoord); 

vec2 HashSample = hash2White(); 
	
	Lighting = vec4(DepthOfFieldNew(Radius, HashSample, BaseDepth),Radius / 2.0); 
	//Lighting.w = 0.0; 
}
