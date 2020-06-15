#include "ItemRenderer.h"
#include "Chunk.h"
#include <fstream>
#include "ParallaxBaker.h"

namespace iTrace {

	namespace Rendering {


		std::vector<ItemIconRequest> Requests; 

		void AddItemIconRequest(ItemRequestType Type, int ID, std::string Title)
		{
			Requests.push_back({ Type, ID, Title }); 
		}

		void RenderItemRequests()
		{

			auto FileExists = [](const std::string FileName) {
				std::ifstream f(FileName.c_str());
				return f.good();
			};

			auto ToLower = [](const std::string& Input) {

				std::string New = Input;

				for (auto& Char : New) {

					Char = tolower(Char);

				}

				return New;

			};

			//Create temporary view and projection matrices 
			//and framebuffer 
			//first however, count requests that are necessary 
			
			std::vector<ItemIconRequest> RequiredRequests; 

			for (auto& Request : Requests) {

				std::string FullTitle = "Items/" + Request.Title + ".png"; 

				if (!FileExists(ToLower(FullTitle))) {
					RequiredRequests.push_back(Request); 
				}

			}

			//does any new icons actually need to be created? 
			//if not, then just return 
			if (RequiredRequests.size() == 0)
				return;


			Matrix4f CubeDrawerView = glm::lookAt(Vector3f(2.0), Vector3f(0.0), Vector3f(0.0,1.0,0.0)); 
			Matrix4f CubeDrawerProject = glm::perspective(glm::radians(60.0), 1.0, 0.01, 10.0);

			FrameBufferObject IconFBO = FrameBufferObject(Vector2i(ITEM_ICON_RESOLUTION), GL_RGBA8, true, false); 

			Shader CubeItemDrawer = Shader("Shaders/CubeItemDrawer"); 

			CubeItemDrawer.Bind(); 

			CubeItemDrawer.SetUniform("Matrix", CubeDrawerProject * CubeDrawerView);
			CubeItemDrawer.SetUniform("DiffuseTextures", 0); 
			CubeItemDrawer.SetUniform("DisplacementTextures", 1);
			CubeItemDrawer.SetUniform("TextureData", 2);
			CubeItemDrawer.SetUniform("TextureExData", 3);
			CubeItemDrawer.SetUniform("ParallaxDirections", BAKE_DIRECTIONS);
			CubeItemDrawer.SetUniform("ParallaxResolution", BAKE_RESOLUTION);

			CubeItemDrawer.SetUniform("CameraPosition", Vector3f(2.0));

			CubeItemDrawer.UnBind(); 


			glClearColor(0.0, 0.0, 0.0, 0.0); 

			//we really don't need to care about performance here. this will only be done a few times at most 

			glEnable(GL_DEPTH_TEST); 

			for (auto& Request : RequiredRequests) {

				std::string FullTitle = "Items/" + Request.Title + ".png";

				//TODO: maybe utilize polymorphism here if the amount of item types becomes much larger than 1 or 2
				
				IconFBO.Bind(); 

				if (Request.Type == ItemRequestType::Block) {

					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(0));

					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(1));

					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_1D, Chunk::GetBlockDataTexture());

					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_1D, Chunk::GetTextureExtensionData());

					CubeItemDrawer.Bind(); 

					CubeItemDrawer.SetUniform("BlockType", Request.ID); 

					DrawPostProcessCube(); 

					CubeItemDrawer.UnBind(); 

					 

				}

				IconFBO.UnBind(); 

				glFinish();

				auto Data = std::vector<sf::Uint8>((ITEM_ICON_RESOLUTION * ITEM_ICON_RESOLUTION) * 4);
				glBindTexture(GL_TEXTURE_2D, IconFBO.ColorBuffer);
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data.data());
				glBindTexture(GL_TEXTURE_2D, 0);

				auto SuperSampledData = std::vector<sf::Uint8>((ITEM_ICON_RESOLUTION * ITEM_ICON_RESOLUTION) / 16);

				for (int x = 0; x < ITEM_ICON_RESOLUTION / 8; x++) {

					for (int y = 0; y < ITEM_ICON_RESOLUTION / 8; y++) {

						Vector4f AverageData = Vector4f(0.0); 

						for (int SubPixelX = 0; SubPixelX < 8; SubPixelX++) {
							for (int SubPixelY = 0; SubPixelY < 8; SubPixelY++) {

								Vector4f CurrentPixel = Vector4f(0.0); 

								for (int c = 0; c < 4; c++) {
									CurrentPixel[c] = float(Data[((x * 8 + SubPixelX) * ITEM_ICON_RESOLUTION + (y * 8 + SubPixelY))*4+c]) / 255.f;
								}
								
								AverageData += CurrentPixel; 

							}
						}

						AverageData /= 64.f; 

						for (int c = 0; c < 4; c++) {
							
							SuperSampledData[(x * (ITEM_ICON_RESOLUTION / 8) + y) * 4 + c] = sf::Uint8(AverageData[c] * 255); 

						}


					}

				}

				sf::Image SaveImage; 
				SaveImage.create(ITEM_ICON_RESOLUTION / 8, ITEM_ICON_RESOLUTION / 8, SuperSampledData.data());
				SaveImage.flipVertically(); 
				SaveImage.saveToFile(FullTitle); 
				
			}

		}

	}

}