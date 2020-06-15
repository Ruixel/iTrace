#include "ParallaxBaker.h"
#include "Texture.h"


namespace iTrace {

	namespace Rendering {


		void ParallaxBaker::PrepareParallaxBaker()
		{


			BakeFBO = FrameBufferObject(Vector2i((BAKE_RESOLUTION+2) * BAKE_DIRECTIONS, BAKE_RESOLUTION), GL_R8, false);
			BakeShader = Shader("Shaders/ParallaxBakeShader"); 
			SetShaderUniforms(); 

		}

		void ParallaxBaker::BakeParallaxMap(const std::string& HeightMap, const std::string& OutPutParallaxMap, float Strength)
		{

			auto HeightMapTexture = LoadTextureGL(HeightMap); 

			BakeShader.Bind(); 
			
			BakeFBO.Bind(); 

			BakeShader.SetUniform("Strength", Strength); 

			HeightMapTexture.Bind(0); 
			
			DrawPostProcessQuad(); 

			BakeFBO.UnBind(); 

			BakeShader.UnBind(); 

			unsigned char* Pixels = new unsigned char[(BAKE_RESOLUTION+2) * BAKE_DIRECTIONS * BAKE_RESOLUTION * 4]; 

			glBindTexture(GL_TEXTURE_2D, BakeFBO.ColorBuffer);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);
			glBindTexture(GL_TEXTURE_2D, 0);

			glFinish();

			sf::Image* OutPutImage = new sf::Image();
			OutPutImage->create((BAKE_RESOLUTION+2) * BAKE_DIRECTIONS, BAKE_RESOLUTION, Pixels);
			OutPutImage->saveToFile(OutPutParallaxMap);

			delete[] Pixels; 

		}

		void ParallaxBaker::ReloadParallaxBaker()
		{
			BakeShader.Reload("Shaders/ParallaxBakeShader"); 
			SetShaderUniforms(); 

		}

		void ParallaxBaker::SetShaderUniforms()
		{

			BakeShader.Bind(); 

			BakeShader.SetUniform("TextureSize", BAKE_RESOLUTION); 
			BakeShader.SetUniform("RayCount", BAKE_DIRECTIONS); 
			BakeShader.SetUniform("DepthTexture", 0); 

			BakeShader.UnBind(); 

		}

	}

}