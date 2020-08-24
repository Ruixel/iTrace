#pragma once

#include "ParallaxBaker.h"


namespace iTrace {
	namespace Rendering {

		struct WaterBaker {

			Shader WaterBakingShader, WaterCausticBakeShader; 
			MultiPassFrameBufferObject WaterMap; 
			FrameBufferObject WaterParallaxMap; 
			FrameBufferObject WaterCausticMap; 
			void Bake(const std::string & BaseDir, const std::string & BaseParallaxDir, int ResolutionBase, unsigned int ResolutionParallax, unsigned int Frames); 
			void BakeCaustics(const std::string& BaseDir, int Resolution, int Frames); 

		};

	}
}