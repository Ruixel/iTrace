#include "CausticBaker.h"
#include "Chunk.h"


namespace iTrace {
	namespace Rendering {
		
		void CausticBaker::BakeCausticMap(std::string DirectoryAlbedo, std::string DirectoryNormal, std::string DirectoryCaustic)
		{

			float Distances[5] = { 0.1,1.0,2.0,3.0,4.0 }; 
			float Bias[5] = { 1.0,1.25,1.5,1.75,2.0 }; 
			float NoiseOffset[5] = { 0.01,0.02,0.03,0.04,0.05 }; 


			int Divisors[5] = { 1,2,4,8,16 }; 

			if (BakingTexture == 0) {
				glGenTextures(1, &BakingTexture); 
				glGenTextures(2, BakingTextureHighPr);

				BakingShader = Shader("Shaders/CausticBaker"); 

				BakingShader.Bind(); 

				BakingShader.SetUniform("Image", 0); 
				BakingShader.SetUniform("ImageHighPr", 1);

				BakingShader.SetUniform("CausticTexture", 2);
				BakingShader.SetUniform("CausticNormalTexture", 3);
				BakingShader.SetUniform("PreviousImage", 4);


				BakingShader.UnBind();

				glBindTexture(GL_TEXTURE_2D, BakingTexture); 
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_RES, TEXTURE_RES, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); 
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glGenerateMipmap(GL_TEXTURE_2D);

				glBindTexture(GL_TEXTURE_2D, 0); 

				for (int i = 0; i < 2; i++) {
					glBindTexture(GL_TEXTURE_2D, BakingTextureHighPr[i]);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, TEXTURE_RES, TEXTURE_RES, 0, GL_RGBA, GL_FLOAT, nullptr);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
					glGenerateMipmap(GL_TEXTURE_2D);

					glBindTexture(GL_TEXTURE_2D, 0);
				}

				glFinish(); 


			}

			auto Albedo = LoadTextureGL(DirectoryAlbedo); 
			auto Normal = LoadTextureGL(DirectoryNormal); 
			
			FrameBufferObject FBOTemp = FrameBufferObject(Vector2i(TEXTURE_RES), GL_RGBA8, false); 


			

			glDisable(GL_DEPTH_TEST); 
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);

			Albedo.Bind(2);
			Normal.Bind(3);

			for (int mip = 0; mip < 5; mip++) {

				BakingShader.Bind();

				int Divisor = Divisors[mip];

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, BakingTexture);
				glBindImageTexture(0, BakingTexture, mip, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

				BakingShader.SetUniform("Distance2", Distances[mip] * 10.0f);
				BakingShader.SetUniform("Distance", Distances[mip]);
				BakingShader.SetUniform("Resolution", Vector2i(TEXTURE_RES) / Divisor);
				BakingShader.SetUniform("NoiseOffset", NoiseOffset[mip]);

				BakingShader.SetUniform("Bias", Bias[mip]);
				BakingShader.SetUniform("mip", mip);

				for (int i = 0; i < 2048; i++) {

					FBOTemp.Bind();

					glViewport(0, 0, TEXTURE_RES / Divisor, TEXTURE_RES / Divisor); 

					BakingShader.SetUniform("Frame", i);

					glActiveTexture(GL_TEXTURE1);
					glBindTexture(GL_TEXTURE_2D, BakingTextureHighPr[i % 2]);
					glBindImageTexture(1, BakingTextureHighPr[i % 2], mip, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_2D, BakingTextureHighPr[!(i % 2)]);

					DrawPostProcessQuad();

					FBOTemp.UnBind();

				}

				BakingShader.UnBind();

				glFinish();

				auto Data = std::vector<sf::Uint8>((TEXTURE_RES * TEXTURE_RES) * 4 / (Divisor*Divisor));
				glBindTexture(GL_TEXTURE_2D, BakingTexture);
				glGetTexImage(GL_TEXTURE_2D, mip, GL_RGBA, GL_UNSIGNED_BYTE, Data.data());
				glBindTexture(GL_TEXTURE_2D, 0);

				glFinish();

				sf::Image SaveImage;
				SaveImage.create(TEXTURE_RES / Divisor, TEXTURE_RES / Divisor, Data.data());
				SaveImage.flipVertically();
				SaveImage.saveToFile(DirectoryCaustic + "_level" + std::to_string(mip) + ".png");

			}
			

		}
	}
}

