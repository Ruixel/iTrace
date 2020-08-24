#version 330
#extension GL_ARB_bindless_texture : enable

in vec2 TexCoord; 
layout(location = 0) out vec4 IndirectDiffuse;
layout(location = 1) out vec4 Volumetrics;
layout(location = 2) out vec4 IndirectSpecular;
layout(location = 3) out vec4 Clouds;
layout(location = 4) out vec4 Direct;


uniform sampler2D FramesNormal[4]; 
uniform sampler2D FramesWorldPos[4]; 
uniform sampler2D FramesIndirectDiffuse[4]; 
uniform sampler2D FramesVolumetric[4]; 
uniform sampler2D FramesIndirectSpecular[4]; 
uniform sampler2D FramesClouds[4]; 
uniform sampler2D FramesDirect[4]; 
uniform sampler2D MotionVectorsSpecular[4]; 

uniform sampler2D MotionVectors[4]; 
uniform sampler2D WorldPos; 
uniform sampler2D Normal; 
uniform bool NewFiltering; 

uniform bool Upscale;
uniform int CurrentFrame; 
uniform ivec2 Resolution; 



bool IsBadMotionVectors(vec2 MotionVectors) {
//return true;



	return MotionVectors.x < -.3 || MotionVectors.y < -.3; 
}




ivec2 States[] = ivec2[](
ivec2(0,0),
ivec2(1,0),
ivec2(1,1),
ivec2(0,1)); 


float Luminance(vec3 Color) {
	return dot(Color, vec3(0.2126, 0.7152, 0.0722)); 
}

bool IsInSky(vec3 Normal) {
	float _L = length(Normal); 
	return _L <= 0.9 || _L >= 1.1; 
}

bool ConfirmGood(vec4 GTNormal, vec4 CurrentNormal) {
	if(!NewFiltering)
		return false; 

	return dot(normalize(GTNormal.xyz), normalize(CurrentNormal.xyz)) < 0.9 || abs(GTNormal.w - CurrentNormal.w) / min(GTNormal.w, CurrentNormal.w) > 0.5; 
}

bool ConstructMotionVector(int idx0, inout vec2 TC, inout vec2 SpecularTC) {
	
	vec2 MotionVector0 = texture(MotionVectors[idx0], TC).xy; 

	TC = TC + MotionVector0; 
	SpecularTC = SpecularTC + texture(MotionVectorsSpecular[idx0], SpecularTC).xy; 

	return IsBadMotionVectors(MotionVector0); 

}

bool ConstructMotionVector(int idx0, int idx1, inout vec2 TC, inout vec2 SpecularTC) {
	
	vec2 MotionVector0 = texture(MotionVectors[idx0], TC).xy; 

	TC = TC + texture(MotionVectors[idx0], TC).xy; 

	vec2 MotionVector1 = texture(MotionVectors[idx1], TC).xy; 

	TC = TC + MotionVector1; 

	SpecularTC = SpecularTC + texture(MotionVectorsSpecular[idx0], SpecularTC).xy; 
	SpecularTC = SpecularTC + texture(MotionVectorsSpecular[idx1], SpecularTC).xy; 

	return IsBadMotionVectors(MotionVector0) || IsBadMotionVectors(MotionVector1); 

}

bool ConstructMotionVector(int idx0, int idx1, int idx2, inout vec2 TC, inout vec2 SpecularTC) {
	
	vec2 MotionVector0 = texture(MotionVectors[idx0], TC).xy; 

	TC = TC + texture(MotionVectors[idx0], TC).xy; 

	vec2 MotionVector1 = texture(MotionVectors[idx1], TC).xy; 

	TC = TC + MotionVector1; 

	vec2 MotionVector2 = texture(MotionVectors[idx2], TC).xy; 

	TC = TC + MotionVector2; 

	SpecularTC = SpecularTC + texture(MotionVectorsSpecular[idx0], SpecularTC).xy; 
	SpecularTC = SpecularTC + texture(MotionVectorsSpecular[idx1], SpecularTC).xy; 
	SpecularTC = SpecularTC + texture(MotionVectorsSpecular[idx2], SpecularTC).xy; 

	return IsBadMotionVectors(MotionVector0) || IsBadMotionVectors(MotionVector1) || IsBadMotionVectors(MotionVector2); 

}


void main() {

	//first of all. what sub pixel are we? 
	ivec2 BasePixel = ivec2(gl_FragCoord.xy); 
	ivec2 HighResPixel = BasePixel * 2; 
	ivec2 SubPixel = BasePixel % 2; 
	ivec2 LowerRes = HighResPixel / 4; 

	 
	vec4 RawNormalFetch = texelFetch(Normal, BasePixel, 0); 



	vec3 Normal = RawNormalFetch.xyz; 
	vec3 WorldPos = texelFetch(WorldPos, HighResPixel, 0).xyz; 

	ivec2 Pixel = LowerRes; 

	vec2 TC = TexCoord; 
	vec2 SpecularTC = TexCoord; 

	bool DoSpatialUpscaling = false; 

	int State = 0; 

	if(SubPixel.x == States[1].x && SubPixel.y == States[1].y) {
		State = 1; 
	}
	else if(SubPixel.x == States[2].x && SubPixel.y == States[2].y) {
		State = 2; 
		
	}
	else if(SubPixel.x == States[3].x && SubPixel.y == States[3].y) {
		State = 3; 
	
	}
	else {
		
	}


	if(State == 0) {

		

		//step 1: calculate motion vectors

		if(CurrentFrame == 0) {
			DoSpatialUpscaling = ConstructMotionVector(0, 3, TC, SpecularTC); 
		}
		else if(CurrentFrame == 1) {
			DoSpatialUpscaling = ConstructMotionVector(1,0,3, TC, SpecularTC); 
		}
		else if(CurrentFrame == 3) {
			DoSpatialUpscaling = ConstructMotionVector(3, TC, SpecularTC); 
		}
	
		Pixel = ivec2(TC * Resolution); 
		ivec2 SpecularPixel = ivec2(SpecularTC * Resolution); 

		if(Pixel.x < 0 || Pixel.x >= Resolution.x || Pixel.y < 0 || Pixel.y >= Resolution.y) 
			DoSpatialUpscaling = true; 

		vec4 FetchedNormal = texelFetch(FramesNormal[2], Pixel, 0); 

		DoSpatialUpscaling = DoSpatialUpscaling || ConfirmGood(RawNormalFetch, FetchedNormal); 


		//step 2: refine (to do) 
		if(!DoSpatialUpscaling) {
		


			IndirectDiffuse = texelFetch(FramesIndirectDiffuse[2], Pixel, 0); 
			Volumetrics = texelFetch(FramesVolumetric[2], Pixel, 0); 
			IndirectSpecular.xyz = texelFetch(FramesIndirectSpecular[2], SpecularPixel,0).xyz; 
			IndirectSpecular.w = texelFetch(FramesIndirectSpecular[2], Pixel, 0).w; 
			Clouds = texelFetch(FramesClouds[2], Pixel, 0); 
			Direct = texelFetch(FramesDirect[2], Pixel, 0); 

		}
	}
	else if(State == 1) {

		if(CurrentFrame == 0) {
			DoSpatialUpscaling = ConstructMotionVector(0, TC, SpecularTC); 
		}
		else if(CurrentFrame == 1) {
			DoSpatialUpscaling = ConstructMotionVector(1,0,TC,SpecularTC); 
		}
		else if(CurrentFrame == 2) {
			DoSpatialUpscaling = ConstructMotionVector(2,1,0, TC, SpecularTC); 
		}

		Pixel = ivec2(TC * Resolution); 
		ivec2 SpecularPixel = ivec2(SpecularTC * Resolution); 


		if(Pixel.x < 0 || Pixel.x >= Resolution.x || Pixel.y < 0 || Pixel.y >= Resolution.y) 
			DoSpatialUpscaling = true; 

		vec4 FetchedNormal = texelFetch(FramesNormal[3], Pixel, 0); 

		DoSpatialUpscaling = DoSpatialUpscaling || ConfirmGood(RawNormalFetch, FetchedNormal);

		if(!DoSpatialUpscaling) {



			IndirectDiffuse = texelFetch(FramesIndirectDiffuse[3], Pixel, 0); 
			Volumetrics = texelFetch(FramesVolumetric[3], Pixel, 0); 
			IndirectSpecular.xyz = texelFetch(FramesIndirectSpecular[3], SpecularPixel,0).xyz; 
			IndirectSpecular.w = texelFetch(FramesIndirectSpecular[3], Pixel, 0).w; 
			Clouds = texelFetch(FramesClouds[3], Pixel, 0); 
			Direct = texelFetch(FramesDirect[3], Pixel, 0); 

		}
	}
	else if(State == 2) {
		
		if(CurrentFrame == 1) {
			DoSpatialUpscaling = ConstructMotionVector(1, TC, SpecularTC); 
		}
		else if(CurrentFrame == 2) {
			DoSpatialUpscaling = ConstructMotionVector(2,1, TC, SpecularTC); 
		}
		else if(CurrentFrame == 3) {
			DoSpatialUpscaling = ConstructMotionVector(3,2,1, TC, SpecularTC); 
		}

		Pixel = ivec2(TC * Resolution); 
		ivec2 SpecularPixel = ivec2(SpecularTC * Resolution); 

		if(Pixel.x < 0 || Pixel.x >= Resolution.x || Pixel.y < 0 || Pixel.y >= Resolution.y) 
			DoSpatialUpscaling = true; 

		vec4 FetchedNormal = texelFetch(FramesNormal[0], Pixel, 0); 

		DoSpatialUpscaling = DoSpatialUpscaling || ConfirmGood(RawNormalFetch, FetchedNormal);

		if(!DoSpatialUpscaling) {



			IndirectDiffuse = texelFetch(FramesIndirectDiffuse[0], Pixel, 0); 
			Volumetrics = texelFetch(FramesVolumetric[0], Pixel, 0); 
			IndirectSpecular.xyz = texelFetch(FramesIndirectSpecular[0], SpecularPixel,0).xyz; 
			IndirectSpecular.w = texelFetch(FramesIndirectSpecular[0], Pixel, 0).w; 
			Clouds = texelFetch(FramesClouds[0], Pixel, 0); 
			Direct = texelFetch(FramesDirect[0], Pixel, 0); 


		}
	}
	else if(State == 3) {

		if(CurrentFrame == 0) {
			DoSpatialUpscaling = ConstructMotionVector(0,3,2,TC,SpecularTC); 
		}
		else if(CurrentFrame == 2) {
			DoSpatialUpscaling = ConstructMotionVector(2,TC,SpecularTC); 
		}
		else if(CurrentFrame == 3) {
			DoSpatialUpscaling = ConstructMotionVector(3,2, TC, SpecularTC);
		}


		Pixel = ivec2(TC * Resolution); 
		ivec2 SpecularPixel = ivec2(SpecularTC * Resolution); 

		if(Pixel.x < 0 || Pixel.x >= Resolution.x || Pixel.y < 0 || Pixel.y >= Resolution.y) 
			DoSpatialUpscaling = true; 

		vec4 FetchedNormal = texelFetch(FramesNormal[1], Pixel, 0); 

		DoSpatialUpscaling = DoSpatialUpscaling || ConfirmGood(RawNormalFetch, FetchedNormal);

		if(!DoSpatialUpscaling) {


			IndirectDiffuse = texelFetch(FramesIndirectDiffuse[1], Pixel, 0); 
			Volumetrics = texelFetch(FramesVolumetric[1], Pixel, 0); 
			IndirectSpecular.xyz = texelFetch(FramesIndirectSpecular[1], SpecularPixel,0).xyz; 
			IndirectSpecular.w = texelFetch(FramesIndirectSpecular[1], Pixel, 0).w; 
			Clouds = texelFetch(FramesClouds[1], Pixel, 0); 
			Direct = texelFetch(FramesDirect[1], Pixel, 0); 


		}
	}

	

	if(DoSpatialUpscaling || Upscale) {


		//find the correct pixel! 

		
		bool Sky = IsInSky(Normal); 

		float BestWeight = -1.0; 

		ivec2 BestPixel = LowerRes; 

		for(int x = -1; x <= 1; x++) {
			for(int y = -1; y <= 1; y++) {
				
				ivec2 CurrentPixel = LowerRes + ivec2(x,y); 

				vec4 NormalData = texelFetch(FramesNormal[CurrentFrame], LowerRes + ivec2(x,y), 0); 
				vec3 CurrentWorldPos = texelFetch(FramesWorldPos[CurrentFrame], LowerRes + ivec2(x,y),0).xyz;  

				float CurrentWeight = (1.0-dot(NormalData.xyz, Normal)) + distance(CurrentWorldPos, WorldPos);

				bool IsSkyHere = IsInSky(NormalData.xyz); 

				if((CurrentWeight < BestWeight || BestWeight < 0.0) && Sky == IsSkyHere) {
				
					BestPixel = CurrentPixel; 
					BestWeight = CurrentWeight; 

				}

			}
		}
		IndirectDiffuse = texelFetch(FramesIndirectDiffuse[CurrentFrame], BestPixel, 0); 
		Volumetrics = texelFetch(FramesVolumetric[CurrentFrame], BestPixel , 0); 
		IndirectSpecular = texelFetch(FramesIndirectSpecular[CurrentFrame], BestPixel, 0); 
		Clouds = texelFetch(FramesClouds[CurrentFrame], BestPixel, 0); 
		Direct = texelFetch(FramesDirect[CurrentFrame], BestPixel, 0); 
	}

}