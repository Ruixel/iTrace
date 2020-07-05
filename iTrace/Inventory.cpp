#include "Inventory.h"
#include "FrameBuffer.h"
#include "ItemRenderer.h"
#include <iostream>

namespace iTrace {

	namespace Rendering {

		void ItemDrawer::Draw(Shader& ItemDrawShader)
		{
			ItemDrawShader.SetUniform("Scale", Size); 
			ItemDrawShader.SetUniform("Position", Position); 
			ItemDrawShader.SetUniform("Texture", TextureID);
			DrawPostProcessQuad(); 
		}

		void Inventory::PrepareInventory()
		{
			ItemDrawShader = Shader("Shaders/InventoryUIShader"); 
			ItemDrawShader.Bind(); 

			ItemDrawShader.SetUniform("BackDrop", 0); 
			ItemDrawShader.SetUniform("ItemTextures", 1); 

			ItemDrawShader.UnBind(); 

			ItemBG = LoadTextureGL("Resources/UI/InventorySlot.png"); 

			auto BlockRequests = GetBlockRequests();

			glGenTextures(1, &ItemTextures); 

			glBindTexture(GL_TEXTURE_2D_ARRAY, ItemTextures); 

			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, 256, 256, BlockRequests.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D_ARRAY,
				GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY,
				GL_TEXTURE_MAG_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);

			for (int idx = 0; idx < BlockRequests.size(); idx++) {

				auto& Data = BlockRequests[idx]; 

				std::string ItemIcon = "Items/" + Data.Title + ".png"; 

				sf::Image LoadingImage;
				LoadingImage.loadFromFile(ItemIcon); 

				glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
					0,
					0, 0, idx,
					256, 256, 1,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					LoadingImage.getPixelsPtr());

			}

			glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		}

		int Inventory::GetBlockID()
		{
			return ActiveSlot;
		}

		void Inventory::DrawInventory(Window& Window)
		{
			glDisable(GL_DEPTH_TEST); 
			glEnable(GL_BLEND); 
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

			float AspectCorrect = float(Window.GetResolution().y) / float(Window.GetResolution().x); 


			ItemDrawer Drawer; 

			Drawer.TextureID = ActiveSlot; 
			
			
			
			ItemBG.Bind(0); 

			glActiveTexture(GL_TEXTURE1); 
			glBindTexture(GL_TEXTURE_2D_ARRAY, ItemTextures);

			ItemDrawShader.Bind(); 

			for (int i = 0; i < 10; i++) {

				Drawer.Size = Vector2f(0.1 * AspectCorrect, 0.1);

				float x = float(i) - 4.5; 

				Drawer.Position = Vector2f(x * 0.2 * AspectCorrect, -0.8);




				Drawer.Draw(ItemDrawShader);

			}

			ItemDrawShader.UnBind(); 

			glDisable(GL_BLEND);

		}

	}

}