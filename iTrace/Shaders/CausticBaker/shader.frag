#version 420

in vec2 TexCoord; 
in vec3 Normal; 
in vec3 Position; 

layout(location = 0) out vec4 c; 

layout(rgba8) uniform image2D Image; 
layout(rgba16f) uniform image2D ImageHighPr; 
uniform sampler2D CausticTexture; 
uniform sampler2D CausticNormalTexture; 
uniform sampler2D PreviousImage; 
uniform int mip; 

uniform float Distance; //the distance the light source has from the block
uniform float Distance2; 
uniform float Bias; //<- biases how bight the light source will be 
uniform float NoiseOffset; 

uniform int Frame; 
uniform ivec2 Resolution; 

float Rand_Seed = 0.0;

vec2 hash2() {
	return fract(sin(vec2(Rand_Seed += 0.1, Rand_Seed += 0.1)) * vec2(43758.5453123, 22578.1459123));
}

const vec3 BlockNormals[6] = vec3[](
				vec3(0.0, 0.0, 1.0),
				vec3(0.0, 0.0, -1.0),
				vec3(1.0, 0.0, 0.0),
				vec3(-1.0, 0.0, 0.0),
				vec3(0.0, 1.0, 0.0),
				vec3(0.0, -1.0, 0.0)
);

const vec3 BlockTangents[6] = vec3[](
	vec3(-1.0,0.0,0.0),
	vec3(1.0,0.0,0.0),
	vec3(0.0,0.0,1.0),
	vec3(0.0,0.0,-1.0),
	vec3(1.0,0.0,0.0),
	vec3(1.0,0.0,0.0)
); 

const vec3 BlockBiTangents[6] = vec3[](
	vec3(0.0,-1.0,0.0),
	vec3(0.0,-1.0,0.0),
	vec3(0.0,-1.0,0.0),
	vec3(0.0,-1.0,0.0),
	vec3(0.0,0.0,1.0),
	vec3(0.0,0.0,-1.0)
); 


float AnalyticalTraversal(vec3 Direction, vec3 Origin, out int Side) {

	
	vec3 BlockPosition = vec3(floor(Origin.xy), 0.0); //<- assumes one block is one unit! 
	vec3 BlockPositionCeil = vec3(BlockPosition.xy+1,1.0); 

	int _ySide = 0, _xSide = 0, _zSide = 0; 

	//the 2 y planes 

	float Y = (BlockPosition.y-Origin.y) / Direction.y; 

	if(Y < 0.0001) {
		Y = (BlockPositionCeil.y-Origin.y ) / Direction.y; 
		_ySide = 1; 
	}
	

	//the 2 x planes 

	float X = (BlockPosition.x-Origin.x) / Direction.x; 
	if(X < 0.0001) {
		X = ( BlockPositionCeil.x-Origin.x) / Direction.x; 
		_xSide = 1; 
	}
	//the 2 z planes 

	
	float Z = 1.0 / Direction.z; 

	if(Y < 0.0) 
		Y = 10000.0; 

	if(X < 0.0) 
		X = 10000.0; 

	if(Z < 0.0) 
		Z = 10000.0; 

	if(Z < min(Y,X)) {
		Side = _zSide; 
	}
	else if(X < Y) {
		Side = 2 + _xSide; 
	}
	else {
		Side = 4 + _ySide; 
	}

	float t = min(X,min(Y,Z)); 
	return t; 

}

vec3 ToTBNSpace(int Side, vec3 WorldPosition) {
	if(Side == 0) {
		return -WorldPosition.xyz; 
	}
	else if(Side == 1) {
		return WorldPosition.xyz * vec3(1.0,-1.0,1.0); 
	}
	else if(Side == 2) {
		return WorldPosition.zyx * vec3(1.0,-1.0,1.0); 
	}
	else if(Side == 3) {
		return WorldPosition.zyx * vec3(-1.0,-1.0,1.0); 
	}
	else {
		return WorldPosition.xzy; 
	}
}




void main() {

	ivec2 Pixel = ivec2(gl_FragCoord); 

	Rand_Seed = (TexCoord.x * TexCoord.y) * 500 + Frame * 1000; 

	vec2 hash_ = hash2() * 2.0 - 1.0; 

	vec2 TexelSize = 1.0 / Resolution; 
	
	vec2 SuperSampledTexCoord = TexCoord + TexelSize * hash_; 
	
	vec2 hash = hash2() * 2.0 - 1.0; 
	hash *= NoiseOffset * 0.25;
	vec3 Origin = vec3(SuperSampledTexCoord, 0.0); 
	vec3 OriginGlass = vec3(fract(SuperSampledTexCoord - hash), Distance); 
	vec3 OriginFinalLight = vec3(OriginGlass.xy, Distance + Distance2 + 1.0); 

	vec3 DirectionPrimary =	normalize(OriginGlass - Origin); 

	float FirstTraversal = Distance / DirectionPrimary.z; 

	vec3 FirstPosition = Origin + DirectionPrimary * FirstTraversal; 
	
	vec3 FirstAlbedo = pow(texture(CausticTexture, FirstPosition.xy).xyz,vec3(2.2)); 
	vec3 FirstNormal = (texture(CausticNormalTexture, fract(FirstPosition.xy)).xyz * 2.0 - 1.0); 

	FirstNormal.xy = sign(FirstNormal.xy) * max(abs(FirstNormal.xy) - vec2(1.0/64.0),vec2(0.0)); 

	FirstNormal = normalize(FirstNormal); 
	//FirstNormal = vec3(0.0,0.0,-1.0); 



	vec3 NewDirection = refract(DirectionPrimary, FirstNormal, 1.0 / 1.3); 

	//todo: add internal refraction equation 
	int Side = 0; 
	float InternalT = AnalyticalTraversal(NewDirection, FirstPosition, Side); 

	vec3 SecondPosition = FirstPosition + NewDirection * InternalT; 

	vec2 TC = fract(ToTBNSpace(Side, SecondPosition).xy); 

	vec3 SecondAlbedo = pow(texture(CausticTexture, TC).xyz,vec3(2.2)); 

	vec3 BaseNormal = BlockNormals[Side]; 


	vec3 SecondaryNormalSample = texture(CausticNormalTexture, TC).xyz * 2 - 1;
	SecondaryNormalSample.xy = sign(SecondaryNormalSample.xy) * max(abs(SecondaryNormalSample.xy) - vec2(1.0/64.0),vec2(0.0)); 
	SecondaryNormalSample = normalize(SecondaryNormalSample); 

	vec3 SecondNormal = SecondaryNormalSample * mat3(BlockTangents[Side], BlockBiTangents[Side], BaseNormal); 
	SecondNormal *= vec3(1.0,1.0,-1.0); 
	

	vec3 FinalDirection = refract(NewDirection, SecondNormal, 1.1); 

	FinalDirection.z = clamp(FinalDirection.z,0.01,1.0); 
	
	float FinalTraversal = (OriginFinalLight.z - FirstPosition.z) / FinalDirection.z; 

	vec3 FinalPoint = FirstPosition + FinalDirection * FinalTraversal; 

	
	vec3 c = (FirstAlbedo + SecondAlbedo) / 2.0; 

	float Dist = distance(FinalPoint.xy, OriginFinalLight.xy) * 2.0; 

	c /= 0.5 * (1.0 + Dist * Dist * 9.0); 

	vec3 ColorMix = mix(c, texelFetch(PreviousImage, ivec2(gl_FragCoord),mip).xyz, float(Frame)/float(Frame+1.0)); 



	imageStore(Image, ivec2(gl_FragCoord), vec4(ColorMix, 1.0)); 
	imageStore(ImageHighPr, ivec2(gl_FragCoord), vec4(ColorMix, 1.0)); 


}