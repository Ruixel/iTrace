#version 330 

layout(location = 0) out vec3 LFNormal;
layout(location = 1) out vec3 WorldPos; 
layout(location = 2) out vec3 HFNormal; 
layout(location = 3) out vec3 Albedo; 

in vec3 Pos; 

uniform sampler2DArray WaterNormal; 
uniform sampler2DArray WaterParallax; 

uniform vec3 CameraPosition; 
uniform float Time; 


vec4 TextureInterp(sampler2DArray Sampler, vec3 TC) {

	float BaseTime = mod(TC.z, 100.); 

	int Coord1 = int(floor(BaseTime)); 
	int Coord2 = int(ceil(BaseTime))%100; 

	return mix(texture(Sampler, vec3(TC.xy, Coord1)), texture(Sampler,vec3(TC.xy, Coord2)), fract(BaseTime)); 

}

float SampleParallaxMap(vec4 Point) {
	
	Point.xyz = fract(Point.xyz); 
	
	int z = int(Point.y * 16); 
	
	int NextZ = (z+1) % 16; 
	
	float Interp = fract(Point.y*16); 
	

	vec2 PixelSize = 1.0 / vec2((128+2)*16, 128); 

	float AddonSize = 1.0 / 128.0; 

	float Multiply = float(128) / float((128+2)*16); 
	Point.z = 1.0 - Point.z; 

	vec2 PixelPoint = Point.xz; 
	PixelPoint.x = (PixelPoint.x + PixelSize.x) * Multiply + AddonSize * z; 

	vec2 PixelPoint2 = Point.xz; 
	PixelPoint2.x = (PixelPoint2.x + PixelSize.x) * Multiply + AddonSize * NextZ; 
	return mix(TextureInterp(WaterParallax, vec3(PixelPoint, Point.w)).x, TextureInterp(WaterParallax, vec3(PixelPoint2,Point.w)).x, Interp); 
}

float GetTraversal(vec3 TC, vec3 Direction, inout vec3 DirectionProjected) {
	DirectionProjected = Direction.xzy; 

	vec2 DirectionXZ = normalize(DirectionProjected.xy); 
	
	float Angle = atan(DirectionXZ.x, DirectionXZ.y); 

	return SampleParallaxMap(vec4(TC.x, (Angle-1.57079633) / 6.28318531,TC.y, TC.z)) * 0.25; //temporary! 
}


void main() {

	vec2 TC = Pos.xz * 0.25; 
	
	vec3 TC3D = vec3(TC, fract(Time/12.0)*100); 

	vec3 Incident = normalize(Pos-CameraPosition); 
	vec3 IncidentProjected ; 

	float Traversal = GetTraversal(TC3D, Incident, IncidentProjected); 

	TC3D.xy = (TC3D.xy + IncidentProjected.xy * Traversal); 
	TC3D.y = 1.0 - TC3D.y; 
	vec3 NormalSample = TextureInterp(WaterNormal, TC3D).xyz * vec3(2.0,2.0,1.0) - vec3(1.0,1.0,0.0); 

	float z = length(NormalSample.xy);  

	vec3 NormalSampleActual = normalize(vec3(NormalSample.x, z*6, NormalSample.y)); 

	LFNormal = NormalSampleActual.xyz; 
	//LFNormal.xy = fract(TC3D.xy); 
	HFNormal = LFNormal; //<- todo: LFNormal + HFNormal split 
	WorldPos = Pos; 
	//LFNormal.xyz = vec3(NormalSample.z); 
	Albedo = pow(vec3(0.3,0.7,1.0),vec3(NormalSample.z+1.0)) * pow(NormalSample.z,2.0); 
}