#version 330
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord; 

layout(location = 0) out vec2 Blocker; //y component stores depth, useful for upscaling later... 

//4 cascades 

uniform sampler2D DirectionalCascades[4]; 
uniform mat4 ShadowMatrix[4]; 

uniform sampler2D WorldPosition; 
uniform sampler2D Depth; 
const float MAXPENUM = 0.05; 


//A lot of the work done here is based on https://www.gamedev.net/tutorials/programming/graphics/contact-hardening-soft-shadows-made-fast-r4906/ 
//Which is a fantastic read if you wanna do direct shadows properly - so I definitely recommend reading that! 
//Albeit I do disagree with a few statements in it, but whatever. 

vec2 VogelDisk(int SampleIdx, int SampleCount, float SampleCountRoot, float Phi) {
	
	float r = sqrt(SampleIdx + 0.5) / SampleCountRoot; 
	float Theta = SampleIdx * 2.4 + Phi; 

	return vec2(cos(Theta) * r, sin(Theta) * r); 

}

float GradientNoise(vec2 ScreenPos) {

	return fract(52.9829189 * fract(dot(ScreenPos, vec2(0.06711056f, 0.00583715)))); 

}

uniform float znear; 
uniform float zfar; 

float LinearDepth(float z)
{
    return 2.0 * znear * zfar / (zfar + znear - (z * 2.0 - 1.0) * (zfar - znear));
} 

float AverageDepthToPenum(float NDCz, float AverageDepth) {
	float Penum = (NDCz - AverageDepth) / AverageDepth; 
	return clamp(100.0 * Penum * Penum, 0.0, 1.0); 
}

float Penumbra(int Cascade, float Noise, vec3 NDC, int Samples, float RootSamples) {

	float AverageBlockerDepth = 0.0;
	int BlockerCount = 0; 

	for(int i = 0; i < Samples; i++) {
	
		vec2 SampleUV = (NDC.xy) * 0.5 + 0.5 + MAXPENUM * VogelDisk(i, Samples, RootSamples, Noise * 2.4);  
		SampleUV = clamp(SampleUV, vec2(0.01), vec2(0.99)); 
		float Shadow = texelFetch(DirectionalCascades[Cascade], ivec2(SampleUV * 2048.0), 0).x; 

		if(Shadow < (NDC.z * 0.5 + 0.5)-0.00009) {
			AverageBlockerDepth += Shadow; 
			BlockerCount++; 
		} 


	}

	if(BlockerCount > 0) {
		return AverageDepthToPenum(NDC.z * 0.5 + 0.5, AverageBlockerDepth / float(BlockerCount)); 
	}

	return 0.0; 
	


}

void main() {
	
	vec3 WorldPos = texture(WorldPosition, TexCoord).xyz;
	Blocker.y = LinearDepth(texelFetch(Depth, ivec2(gl_FragCoord.xy) * 8, 0).x); 

	int Cascade = -1; 
	vec3 NDC; 

	float Edge = 0.9 - GradientNoise(gl_FragCoord.yx) * 0.1; 

	for(int i = 0; i < 4; i++) {
	
		vec4 Clip = ShadowMatrix[i] * vec4(WorldPos, 1); 
		NDC = Clip.xyz / Clip.w; 

		if((abs(NDC.x) < Edge && abs(NDC.y) < Edge) || (abs(NDC.x) < 1.0 && abs(NDC.y) < 1.0 && i == 3)) {
			Cascade = i; 
			break; 
		}


	}

	if(Cascade == -1) {
		Blocker.x = 0.0; 
		return; 
	}

	Blocker.x = Penumbra(Cascade, GradientNoise(gl_FragCoord.xy), NDC, 32, 5.65685424949); 
}
