#pragma once

#include "Shader.h"
#include "FrameBuffer.h"


namespace iTrace {

	namespace Rendering {

		struct CausticBaker {
	
			Shader BakingShader; 

			unsigned int BakingTexture = 0, BakingTextureHighPr[2]; 

			void BakeCausticMap(std::string DirectoryAlbedo, std::string DirectoryNormal, std::string DirectoryCaustic); 

		};

	}

}