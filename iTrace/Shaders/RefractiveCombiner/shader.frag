#version 430 core

in vec2 InTexCoord; 
layout(location = 0) out vec4 Lighting;
layout(location = 1) out vec3 Glow; 
layout(location = 2) out float DofDepth; //<- depth used for DoF 

layout(binding = 0, std140) buffer DepthMiddleBuffer {
	vec4 OcclusionDepth[]; 
};

uniform sampler2D CombinedLighting; 
uniform sampler2D CombinedGlow; 

uniform sampler2D Depth; 
uniform sampler2D RefractiveBlocks; 
uniform sampler2D PrimaryRefractionDepth; 
uniform sampler2D PrimaryRefractionColor; 
uniform sampler2D PrimaryRefractionNormal; 
uniform sampler2D PrimaryRefractionNormalLF; 
uniform sampler2D Volumetrics; 
uniform sampler2D WaterRefraction; 
uniform ivec2 PixelFocusPoint; 

uniform mat4 IdentityMatrix;
uniform mat4 InverseView; 
uniform mat4 InverseProj; 
uniform vec3 CameraPosition; 

const bool ChromaticAbberation = false; 
const bool HitLocationDepth = true; //<- if you want to use the surface location or the hit location depth for the DoF (refraction) 
const vec3 CAMultiplier = vec3(1.02, 1.0, 1.04); //<- multipliers for chromatic abberation 

uniform float znear; 
uniform float zfar; 

float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
} 


vec3 GetWorldPosition(float z, vec2 TexCoord) {

	vec4 Clip = vec4(TexCoord * 2.0 - 1.0, z * 2.0 -1.0, 1.0); 
	vec4 ViewSpace = InverseProj * Clip; 
	ViewSpace /= ViewSpace.w; 

	vec4 World = (InverseView * ViewSpace); 
	return World.xyz; 
}



//takes in origin + direction and returns analytical traversal for a ray within that block 

float AnalyticalTraversal(vec3 Direction, vec3 Origin) {


	vec3 BlockPosition = floor(Origin); //<- assumes one block is one unit! 
	vec3 BlockPositionCeil = BlockPosition + 1; 



	//the 2 y planes 

	float Y = (BlockPosition.y-Origin.y) / Direction.y; 

	if(Y < 0.0) 
		Y = (BlockPositionCeil.y-Origin.y ) / Direction.y; 

	

	//the 2 x planes 

	float X = (BlockPosition.x-Origin.x) / Direction.x; 
	if(X < 0.0) 
		X = ( BlockPositionCeil.x-Origin.x) / Direction.x; 
	//the 2 z planes 

	float Z = ( BlockPosition.z-Origin.z) / Direction.z; 
	if(Z < 0.0) 
		Z = (BlockPositionCeil.z-Origin.z) / Direction.z; 

	if(Y < 0.0) 
		Y = 10000.0; 

	if(X < 0.0) 
		X = 10000.0; 

	if(Z < 0.0) 
		Z = 10000.0; 

	float t = min(X,min(Y,Z)); 

	return t; 

}

void ManageRefraction(inout vec2 TC, inout vec3 Multiplier, float Depth) {
	
	float RefractiveDepth = texelFetch(PrimaryRefractionDepth, ivec2(gl_FragCoord), 0).x; 
	
	if(RefractiveDepth < Depth) {
		
		

		vec3 Normal = texture(PrimaryRefractionNormal, TC).xyz; 

		vec3 WorldPosition = GetWorldPosition(RefractiveDepth, TC); 
		
		vec3 Incident = normalize(WorldPosition - CameraPosition); 

		vec3 RayDir = refract(Incident, Normal, 1.0/1.33); 

		vec3 NewPosition = WorldPosition + RayDir * 0.2; 

		float t = AnalyticalTraversal(RayDir, WorldPosition - Normal * 0.01); 

		vec3 PrimaryColor = texture(PrimaryRefractionColor, TC).xyz;; 
		//PrimaryColor = mix(vec3(1.0), pow(PrimaryColor,vec3(5.0)),t/sqrt(3)); 
		



		vec4 Clip = IdentityMatrix * vec4(NewPosition, 1.0);
		Clip.xyz /= Clip.w; 


		TC = Clip.xy * 0.5 + 0.5; 

		Multiplier *= PrimaryColor * texture(RefractiveBlocks, TC).xyz;  



	}

}

vec2 TCFromWorldPos(vec3 WorldPos) {

	vec4 Clip = IdentityMatrix * vec4(WorldPos, 1.0); 
	Clip.xyz /= Clip.w; 

	return Clip.xy * 0.5 + 0.5; 

}


void main() {

	vec3 Multiplier = vec3(1.0); 

	float DepthSample = texelFetch(Depth, ivec2(gl_FragCoord), 0).x; 
	float RefractiveDepth = texelFetch(PrimaryRefractionDepth, ivec2(gl_FragCoord), 0).x; 
	
	vec2 TC = InTexCoord; 
	vec3 ColorSample = vec3(1.0); 

	TC = texture(WaterRefraction, InTexCoord).xy; 

	if(RefractiveDepth < DepthSample) {
		
		ColorSample = pow(texture(PrimaryRefractionColor, TC).xyz,vec3(2.2)); 

		vec3 Normal = texture(PrimaryRefractionNormal, TC).xyz; 
		vec3 LFNormal = texture(PrimaryRefractionNormalLF, TC).xyz; 

		vec3 WorldPosition = GetWorldPosition(RefractiveDepth, TC); 
		
		vec3 Incident = normalize(WorldPosition - CameraPosition); 


		if(ChromaticAbberation) { //<- temporarily removed 
			vec3 RayDirR = refract(Incident, Normal, 1.0/(1.3*CAMultiplier.r)); 
			vec3 RayDirG = refract(Incident, Normal, 1.0/(1.3*CAMultiplier.g)); 
			vec3 RayDirB = refract(Incident, Normal, 1.0/(1.3*CAMultiplier.b)); 


			//todo: add option for these traversals to be the same, improving performance
			//with only a small cost in accuracy 
			vec3 Traversals = vec3(
				AnalyticalTraversal(RayDirR, WorldPosition - Normal * 0.01),
				AnalyticalTraversal(RayDirG, WorldPosition - Normal * 0.01), 
				AnalyticalTraversal(RayDirB, WorldPosition - Normal * 0.01)); 

			vec2 TCR = TCFromWorldPos(WorldPosition + RayDirR * Traversals.r); 
			vec2 TCG = TCFromWorldPos(WorldPosition + RayDirG * Traversals.g); 
			vec2 TCB = TCFromWorldPos(WorldPosition + RayDirB * Traversals.b); 

			vec4 LightingFetchR = texture(CombinedLighting, TCR); 
			vec4 LightingFetchG = texture(CombinedLighting, TCG); 
			vec4 LightingFetchB = texture(CombinedLighting, TCB); 

			Glow = vec3(
				texture(CombinedGlow, TCR).r, 
				texture(CombinedGlow, TCG).g, 
				texture(CombinedGlow, TCB).b); 

			Lighting = vec4(
				LightingFetchR.r,
				LightingFetchG.g,
				LightingFetchB.b,
				LightingFetchR.w); 

			ivec2 Pixel = ivec2(TCG * textureSize(Depth, 0)); 

			if(HitLocationDepth)
				DofDepth = texelFetch(Depth, Pixel,0).x; 
			else 
				DofDepth = RefractiveDepth; 

			//Lighting = vec3(
		
		}
		else {
			vec3 RayDir = refract(Incident, Normal, 1.0/1.1); 
			
			float Traversal = AnalyticalTraversal(RayDir, (WorldPosition - LFNormal * 0.04) + RayDir * 0.001); 
	
			TC = TCFromWorldPos(WorldPosition + RayDir * Traversal); 
			float DepthFetchHere = texelFetch(Depth, ivec2(TC * textureSize(Depth, 0).xy),0).x; 

			if(DepthFetchHere < RefractiveDepth) 
				TC = InTexCoord; 


			Glow = texture(CombinedGlow, TC).xyz;
			Lighting = texture(CombinedLighting, TC); 

			ivec2 Pixel = ivec2(TC * textureSize(Depth, 0)); 

			if(HitLocationDepth)
				DofDepth = texelFetch(Depth, Pixel,0).x; 
			else 
				DofDepth = RefractiveDepth; 

			//Lighting.xyz = vec3(Traversal); 

			ColorSample = pow(ColorSample / 2.0, vec3((Traversal+.5)*1.0)) * 2.0; 
			ColorSample = clamp(ColorSample, vec3(0.0), vec3(1.0));  
			
		}

		ColorSample *= texture(RefractiveBlocks, TC).xyz; 
		//Lighting.xyz = ColorSample; 
		Lighting.xyz *= ColorSample; 

		Glow.xyz *= ColorSample; 



	}
	else {
	
		Lighting = texture(CombinedLighting, TC); 
		Glow = texture(CombinedGlow, TC).xyz; 
		DofDepth = DepthSample; 

	}
	vec4 Volumetrics = texture(Volumetrics, InTexCoord); 
	Lighting.xyz += Volumetrics.xyz * ColorSample; 
	gl_FragDepth = min(DofDepth,0.9999999); 

	ivec2 Pixel = ivec2(gl_FragCoord); 

	if(Pixel.x == PixelFocusPoint.x && Pixel.y == PixelFocusPoint.y) {
		
		OcclusionDepth[0] = vec4(DofDepth); 

	}
	Lighting.w = LinearDepth(DofDepth); 

	Glow = vec3(0.0); 
	

}