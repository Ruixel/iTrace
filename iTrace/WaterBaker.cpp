#include "WaterBaker.h"
#include <fstream>


namespace iTrace {
	namespace Rendering {
		void WaterBaker::Bake(const std::string& BaseDir, const std::string& BaseParallaxDir, int ResolutionBase, unsigned int ResolutionParallax, unsigned int Frames)
		{
			
			auto FileExists = [](const std::string& FilePath) {
				return (std::ifstream(FilePath)).good(); 
			}; 

			if (FileExists(BaseDir + "_" + std::to_string(Frames - 1) + ".png") && FileExists(BaseParallaxDir + "_" + std::to_string(Frames - 1) + ".png"))
				return; //<- water is already baked! 

			ParallaxBaker Baker; 
			Baker.PrepareParallaxBaker(); 
			
			WaterBakingShader = Shader("Shaders/WaterBaker"); 
			WaterBakingShader.Bind(); 

			WaterBakingShader.SetUniform("TIME", (int)Frames); 

			WaterBakingShader.UnBind();

			WaterMap = MultiPassFrameBufferObject(Vector2i(ResolutionBase), 2, { GL_RGBA8, GL_R8 }, false, false, true); 
			WaterParallaxMap = FrameBufferObject(Vector2i((ResolutionParallax + 2) * 16, ResolutionParallax), GL_R8, false); 

			unsigned char* Pixels = new unsigned char[ResolutionBase* ResolutionBase * 4];
			
			for (int Frame = 0; Frame < Frames; Frame++) {

				std::string OutPutDir = BaseDir + "_" + std::to_string(Frame) + ".png"; 
				std::string OutPutDirParallax = BaseParallaxDir + "_" + std::to_string(Frame) + ".png";

				WaterMap.Bind(); 

				WaterBakingShader.Bind(); 

				WaterBakingShader.SetUniform("Frame", Frame); 

				DrawPostProcessQuad(); 

				WaterBakingShader.UnBind(); 

				WaterMap.UnBind(); 

				glBindTexture(GL_TEXTURE_2D, WaterMap.ColorBuffers[0]);
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);
				glBindTexture(GL_TEXTURE_2D, 0);

				glFinish();

				sf::Image* OutPutImage = new sf::Image();
				OutPutImage->create(ResolutionBase, ResolutionBase, Pixels);
				OutPutImage->saveToFile(OutPutDir);

				delete OutPutImage; 

				Baker.BakeParallaxMapCustom(OutPutDirParallax, 1.0, ResolutionParallax, 16, WaterParallaxMap, WaterMap.ColorBuffers[1]); 

				glFinish(); 

			}

			delete[] Pixels; 

		}
		void WaterBaker::BakeCaustics(const std::string& BaseDir, int Resolution, int Frames)
		{

			auto FileExists = [](const std::string& FilePath) {
				return (std::ifstream(FilePath)).good();
			};

			if (FileExists(BaseDir + "_" + std::to_string(Frames - 1) + ".png") )
				return; //<- water is already baked! 

			WaterCausticMap = FrameBufferObject(Vector2i(Resolution), GL_R8, false); 
			WaterCausticBakeShader = Shader("Shaders/WaterCausticsBaker"); 

			

			unsigned char* Pixels = new unsigned char[Resolution * Resolution * 4];


			for (int Frame = 0; Frame < Frames; Frame++) {
					
				std::string OutPutDir = BaseDir + "_" + std::to_string(Frame) + ".png";

				WaterCausticMap.Bind(); 

				WaterCausticBakeShader.Bind();

				WaterCausticBakeShader.SetUniform("Divisor", 0.31231273f * 0.375f);

				WaterCausticBakeShader.SetUniform("Time", float(Frame) / float(Frames)); 

				DrawPostProcessQuad(); 

				WaterCausticBakeShader.UnBind();

				WaterCausticMap.UnBind(); 



				glBindTexture(GL_TEXTURE_2D, WaterCausticMap.ColorBuffer);
				glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);
				glBindTexture(GL_TEXTURE_2D, 0);

				glFinish();

				sf::Image* OutPutImage = new sf::Image();
				OutPutImage->create(Resolution, Resolution, Pixels);
				OutPutImage->saveToFile(OutPutDir);

				delete OutPutImage;

			}


			


		}
	}
}
