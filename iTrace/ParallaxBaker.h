#pragma once

#include "Shader.h"
#include "FrameBuffer.h"
	
const int BAKE_RESOLUTION = 256; 
const int BAKE_DIRECTIONS = 32; 


namespace iTrace {

	namespace Rendering {

		struct ParallaxBaker {

			Shader BakeShader; 
			FrameBufferObject BakeFBO; 
			
			void PrepareParallaxBaker(); 
			void BakeParallaxMap(const std::string& HeightMap, const std::string& OutPutParallaxMap, float Strength); 
			void ReloadParallaxBaker(); 

		private: 

			void SetShaderUniforms(); 

		};	


	}

}