#version 330

in vec2 TexCoord; 
in vec3 Normal; 
in vec3 Position; 
layout(location = 0) out vec4 Lighting;

uniform int BlockType; 
uniform sampler2DArray DiffuseTextures; 
uniform sampler2DArray DisplacementTextures; 
uniform sampler1D TextureData;
uniform sampler1D TextureExData; 

uniform vec3 CameraPosition; 

int GetTextureIdx(int Type, int Side) {
	return int(texelFetch(TextureData, Type*2+(Side/3), 0)[Side%3]*255.0+0.5);
}

vec3 Fresnel(vec3 Incident, vec3 Normal, vec3 Specular, float Roughness) {
	return Specular + (max(vec3(1-Roughness) - Specular,vec3(Specular))) * pow(max(1.0 - abs(dot(Incident,Normal)),0.0), 5.0);
}

uniform int ParallaxDirections; 
uniform int ParallaxResolution; 


vec3 ToTBNSpace(int Side, vec3 WorldPosition) {


	if(Side == 1) {
		return -WorldPosition.xyz; 
	}
	else if(Side == 0) {
		return WorldPosition.xyz * vec3(1.0,-1.0,1.0); 
	}
	else if(Side == 3) {
		return WorldPosition.zyx * vec3(1.0,-1.0,1.0); 
	}
	else if(Side == 2) {
		return WorldPosition.zyx * vec3(-1.0,-1.0,1.0); 
	}
	else {
		return WorldPosition.zxy; 
	}


}


float SampleParallaxMap(vec3 Point, uint Texture) {
	
	Point = fract(Point); 
	
	int z = int(Point.y * ParallaxDirections); 
	
	int NextZ = (z+1) % ParallaxDirections; 
	
	float Interp = fract(Point.y*ParallaxDirections); 
	

	vec2 PixelSize = 1.0 / vec2((ParallaxResolution+2)*ParallaxDirections, ParallaxResolution); 

	float AddonSize = 1.0 / ParallaxDirections; 

	float Multiply = float(ParallaxResolution) / float((ParallaxResolution+2)*ParallaxDirections); 
	Point.z = 1.0 - Point.z; 

	vec2 PixelPoint = Point.xz; 
	PixelPoint.x = (PixelPoint.x + PixelSize.x) * Multiply + AddonSize * z; 

	vec2 PixelPoint2 = Point.xz; 
	PixelPoint2.x = (PixelPoint2.x + PixelSize.x) * Multiply + AddonSize * NextZ; 
	return mix(textureLod(DisplacementTextures, vec3(PixelPoint, Texture), 0.0).x, textureLod(DisplacementTextures, vec3(PixelPoint2,Texture),0.0).x, Interp); 
}

float GetTraversal(vec2 TC, vec3 Direction, uint Side, uint Type, inout vec3 DirectionProjected) {
	DirectionProjected = ToTBNSpace(int(Side),Direction); 

	vec2 DirectionXZ = normalize(DirectionProjected.xy); 
	
	float Angle = atan(DirectionXZ.x, DirectionXZ.y); 

	return SampleParallaxMap(vec3(TC.x, (Angle-1.57079633) / 6.28318531,TC.y), Type) * 0.5; //temporary! 
}



void main() {

	vec3 Incident = normalize(Position - CameraPosition); 

	vec3 Nabs = abs(Normal); 

	int Side = 0;
	
	vec2 ATC = TexCoord; 

	if(Nabs.x > max(Nabs.y, Nabs.z)) {

		ATC = ATC.yx; 

		ATC.y = 1.0 - ATC.y; 

		Side = 2 + int(max(-Nabs.x,0)+.5); 
	}
	else if(Nabs.z > Nabs.y) {

		ATC.y = 1.0 - ATC.y; 

		Side = int(max(-Nabs.z,0)+.5); 
	}
	else {

		ATC = ATC.yx; 
		ATC.x = 1.0 - ATC.x; 

		Side = 4 + int(max(-Nabs.y,0)+.5); 
	}

	
	

	int TextureIdx = GetTextureIdx(BlockType, Side); 

	ivec2 TextureExData = ivec2(texelFetch(TextureExData, TextureIdx,0).xy * 255); 

	if(TextureExData.x != 0) {
		vec3 IncidentProjected; 

		float Traversal = GetTraversal(ATC.xy, Incident, uint(Side), uint(TextureExData.x)-1u,IncidentProjected) ;


		ATC = (ATC.xy + IncidentProjected.xy * Traversal).xy; 


	}
	vec3 DiffuseColor = pow(texture(DiffuseTextures, vec3(ATC, TextureIdx)).xyz, vec3(2.2)); 



	vec3 ReflectionVector = reflect(Incident, Normal); 

	vec3 LightDir = normalize(vec3(0.0,2.0,0.0)); 
	vec3 LightDir2 = normalize(vec3(2.0,0.0,0.0)); 
	vec3 LightDir3 = normalize(vec3(0.0,0.0,2.0)); 


	Lighting.xyz = DiffuseColor * (max(dot(Normal,LightDir),0.0) + max(dot(Normal,LightDir2),0.0) + max(dot(Normal,LightDir3),0.0)); 
	Lighting.xyz = pow(Lighting.xyz, vec3(0.4545454545)); 
	Lighting.w = 1.0; 
	

}