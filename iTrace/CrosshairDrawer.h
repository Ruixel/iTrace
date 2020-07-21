#pragma once

#include "Texture.h"
#include "Shader.h"

namespace iTrace {

	namespace Rendering {


		struct CrosshairDrawer {

			Shader UIShader; //todo: write a UI wrapper, and have the shader stored there! 
			TextureGL CrosshairTexture; 

			void PrepareCrosshairDrawer(Window & Window); 
			void DrawCrosshair(); 

		};



	}

}