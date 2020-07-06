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
vec2 Kernels[120] = vec2[120](
vec2(0.02,0),
vec2(0.181575,0.12422),
vec2(0.0863111,0.221993),
vec2(-0.0579242,0.248309),
vec2(-0.199579,0.182831),
vec2(-0.282584,0.0402946),
vec2(-0.268546,-0.1325),
vec2(-0.153378,-0.27263),
vec2(0.0284658,-0.324353),
vec2(0.214432,-0.261123),
vec2(0.335785,-0.0977491),
vec2(0.343198,0.112482),
vec2(0.226483,0.295407),
vec2(0.0207097,0.382422),
vec2(-0.204259,0.336248),
vec2(-0.367703,0.166379),
vec2(-0.407184,-0.0720249),
vec2(-0.302308,-0.296124),
vec2(-0.0841434,-0.424369),
vec2(0.173803,-0.40626),
vec2(0.380473,-0.242026),
vec2(0.459542,0.0153711),
vec2(0.377612,0.277309),
vec2(0.157906,0.450144),
vec2(-0.126018,0.468779),
vec2(-0.374955,0.321109),
vec2(-0.49883,0.0541862),
vec2(-0.449328,-0.240699),
vec2(-0.238558,-0.459329),
vec2(0.0633937,-0.521473),
vec2(0.35179,-0.400318),
vec2(0.523653,-0.133622),
vec2(0.514592,0.187928),
vec2(0.322849,0.451601),
vec2(0.0116202,0.562221),
vec2(-0.311753,0.476537),
vec2(-0.532865,0.219959),
vec2(-0.57078,-0.120769),
vec2(-0.407632,-0.426917),
vec2(-0.0964785,-0.589203),
vec2(0.255896,-0.546836),
vec2(0.52567,-0.310205),
vec2(0.615549,0.0412247),
vec2(0.489855,0.385595),
vec2(0.188497,0.600959),
vec2(-0.185601,0.608503),
vec2(-0.501675,0.401353),
vec2(-0.64688,0.048455),
vec2(-0.566591,-0.328351),
vec2(-0.284865,-0.59643),
vec2(0.102586,-0.659082),
vec2(0.460916,-0.490414),
vec2(0.663122,-0.145786),
vec2(0.635081,0.256304),
vec2(0.382677,0.574989),
vec2(-0.00889368,0.696425),
vec2(-0.403869,0.574465),
vec2(-0.663028,0.248087),
vec2(-0.692788,-0.170966),
vec2(-0.478986,-0.536459),
vec2(-0.0931379,-0.71873),
vec2(0.331454,-0.650704),
vec2(0.645781,-0.352526),
vec2(0.737447,0.0742206),
vec2(0.570864,0.481124),
vec2(0.20093,0.724584),
vec2(-0.245011,0.716514),
vec2(-0.611019,0.456187),
vec2(-0.767119,0.0317238),
vec2(-0.655465,-0.409721),
vec2(-0.311712,-0.712999),
vec2(0.146275,-0.769521),
vec2(0.558845,-0.556127),
vec2(0.780235,-0.144374),
vec2(0.730087,0.323432),
vec2(0.422584,0.683435),
vec2(-0.0373454,0.807645),
vec2(-0.489827,0.649446),
vec2(-0.775634,0.261009),
vec2(-0.792241,-0.223858),
vec2(-0.530584,-0.635816),
vec2(-0.0793691,-0.82916),
vec2(0.404988,-0.733358),
vec2(0.752597,-0.378746),
vec2(0.839708,0.112981),
vec2(0.632758,0.570537),
vec2(0.201198,0.832729),
vec2(-0.305788,0.805254),
vec2(-0.710866,0.494602),
vec2(-0.870595,0.0068724),
vec2(-0.72623,-0.48846),
vec2(-0.325275,-0.817448),
vec2(0.194094,-0.86277),
vec2(0.650655,-0.605569),
vec2(0.883388,-0.133088),
vec2(0.808273,0.390898),
vec2(0.448601,0.782868),
vec2(-0.0721387,0.903848),
vec2(-0.572656,0.708682),
vec2(-0.876988,0.262823),
vec2(-0.876376,-0.279588),
vec2(-0.568116,-0.729017),
vec2(-0.0575292,-0.926786),
vec2(0.478022,-0.801096),
vec2(0.85075,-0.393068),
vec2(0.928307,0.156658),
vec2(0.680774,0.6564),
vec2(0.192103,0.930284),
vec2(-0.368346,0.880154),
vec2(-0.804496,0.520724),
vec2(-0.962175,-0.0245789),
vec2(-0.783613,-0.565997),
vec2(-0.328567,-0.91349),
vec2(0.245647,-0.943448),
vec2(0.738541,-0.642659),
vec2(0.976474,-0.11388),
vec2(0.873826,0.459255),
vec2(0.463789,0.876015),
vec2(-0.1123,0.988892),
vec2(-0.653678,0.755804));

//partially based on http://tuxedolabs.blogspot.com/2018/05/bokeh-depth-of-field-in-single-pass.html
vec3 DepthOfFieldNew(float Radius, vec2 Hash, float CenterDepth) {
	
	
	vec3 Result = vec3(0.0); 

	vec2 TexelSize = 1.0 / vec2(textureSize(Input, 0)); 

	float Samples = 0.0; 


	for(int i = 0; i < 120; i++) {
		
		vec2 Coord = TexCoord + Kernels[i] * TexelSize * Radius * 0.5; 

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
