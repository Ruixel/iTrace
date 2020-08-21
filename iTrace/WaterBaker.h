#pragma once

#include "ParallaxBaker.h"


namespace iTrace {
	namespace Rendering {

		struct WaterBaker {

			Shader WaterBakingShader; 
			MultiPassFrameBufferObject WaterMap; 
			FrameBufferObject WaterParallaxMap; 

			void Bake(const std::string & BaseDir, const std::string & BaseParallaxDir, int ResolutionBase, unsigned int ResolutionParallax, unsigned int Frames); 

		};

	}
}