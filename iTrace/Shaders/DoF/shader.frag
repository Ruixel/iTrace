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
	}
vec2 Kernels[51] = vec2[51](
vec2(0.00390625,-0),
vec2(0.0603367,-0.128936),
vec2(-0.128061,-0.15346),
vec2(-0.227301,0.0599167),
vec2(-0.048118,0.264502),
vec2(0.250687,0.178076),
vec2(0.280579,-0.158967),
vec2(-0.0290974,-0.373316),
vec2(-0.34729,-0.130682),
vec2(-0.291286,0.284268),
vec2(0.138152,0.396194),
vec2(0.435724,0.0445877),
vec2(0.24203,-0.404635),
vec2(-0.263341,-0.389367),
vec2(-0.517893,0.0822949),
vec2(-0.13934,0.485495),
vec2(0.406607,0.354163),
vec2(0.498269,-0.22081),
vec2(0.0111189,-0.557759),
vec2(-0.531133,-0.261174),
vec2(-0.453163,0.363994),
vec2(0.151844,0.621488),
vec2(0.598553,0.119764),
vec2(0.38213,-0.519114),
vec2(-0.309577,-0.567813),
vec2(-0.654743,0.0406989),
vec2(-0.253446,0.643877),
vec2(0.463911,0.489348),
vec2(0.699053,-0.232933),
vec2(0.0806435,-0.695484),
vec2(-0.625404,-0.385607),
vec2(-0.614048,0.40336),
vec2(0.106442,0.733428),
vec2(0.746403,0.225141),
vec2(0.504134,-0.563291),
vec2(-0.322943,-0.756055),
vec2(-0.77925,-0.0260326),
vec2(-0.369307,0.72618),
vec2(0.500768,0.640928),
vec2(0.796456,-0.183267),
vec2(0.179765,-0.839643),
vec2(-0.661411,-0.500714),
vec2(-0.79504,0.419711),
vec2(0.0413755,0.850904),
vec2(0.821128,0.336082),
vec2(0.65061,-0.60033),
vec2(-0.268957,-0.845017),
vec2(-0.923605,-0.119745),
vec2(-0.48111,0.757234),
vec2(0.521294,0.817409),
vec2(0.910716,-0.119648)
);

//partially based on http://tuxedolabs.blogspot.com/2018/05/bokeh-depth-of-field-in-single-pass.html
vec3 DepthOfFieldNew(float Radius, vec2 Hash, float CenterDepth) {
	
	
	vec3 Result = vec3(0.0); 

	vec2 TexelSize = 1.0 / vec2(textureSize(Input, 0)); 

	float Samples = 0.0; 


	for(int i = 0; i < 51; i++) {
		
		vec2 Coord = TexCoord + Kernels[i] * TexelSize * 1.25* Radius*0.0625; 

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
