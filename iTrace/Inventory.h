#pragma once

#include "Shader.h"
#include "Window.h"
#include "Texture.h"

namespace iTrace {

	namespace Rendering {

		struct ItemDrawer {

			int TextureID; 
			int Amount; 
			Vector2f Size; 
			Vector2f Position; 

			void Draw(Shader& ItemDrawShader); 

		};

		struct Inventory {

			Shader ItemDrawShader; 
			int ActiveSlot = 0; 
			unsigned int ItemTextures; 
			TextureGL ItemBG; 
			

			void PrepareInventory(); 
			int GetBlockID(); 
			void DrawInventory(Window& Window); 


		};





	}

}