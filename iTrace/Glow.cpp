#include "Glow.h"

namespace iTrace {

	namespace Rendering {





		void GlowRenderer::PrepareGlow(Window& Window, Camera& Camera)
		{

			PrepBuffer = MultiPassFrameBufferObject(Window.GetResolution() / 2, 2, { GL_RGB16F, GL_RGB16F }, false, true); 

			for (int i = 0; i < 2; i++) {
				for(int j = 0; j < 3; j++)
					GlowBuffer[j][i] = FrameBufferObject(Window.GetResolution() / Divisors[j], GL_RGB16F, false, false);
				DoFBuffer[i] = FrameBufferObject(Window.GetResolution() / 4, GL_RGB16F, false, false); 
			}
			PrepShader = Shader("Shaders/PrepShader");
			BlurShader[0] = Shader("Shaders/BlurHorizontal");
			BlurShader[1] = Shader("Shaders/BlurVertical");
			DofShader[0] = Shader("Shaders/DofHorizontal"); 
			DofShader[1] = Shader("Shaders/DofVertical"); 
			GlowPrepShader = Shader("Shaders/BlurCombiner"); 

			PrepShader.Bind();

			PrepShader.SetUniform("Normal", 0);
			PrepShader.SetUniform("Albedo", 1);
			PrepShader.SetUniform("SkyReigh", 2);
			PrepShader.SetUniform("Lighting", 3);
			PrepShader.SetUniform("Volumetrics", 4);

			PrepShader.UnBind();

			for (int x = 0; x < 2; x++) {

				BlurShader[x].Bind();
				BlurShader[x].SetUniform("InverseRes", 1.0f / Vector2f(Window.GetResolution() / 4));
				BlurShader[x].SetUniform("Input", 0);
				BlurShader[x].UnBind();

			}

			for (int x = 0; x < 2; x++) {

				DofShader[x].Bind();
				DofShader[x].SetUniform("InverseRes", 1.0f / Vector2f(Window.GetResolution() / 4));
				DofShader[x].SetUniform("Input", 0);
				DofShader[x].SetUniform("Depth", 1);
				DofShader[x].SetUniform("znear", Camera.znear);
				DofShader[x].SetUniform("zfar", Camera.zfar);
				DofShader[x].UnBind();

			}

			GlowPrepShader.Bind(); 


			GlowPrepShader.SetUniform("GlowInput", 0);
			GlowPrepShader.SetUniform("RawInput", 1); 


			GlowPrepShader.UnBind(); 

		}

		void GlowRenderer::RenderGlow(Window& Window, SkyRendering& Sky, IndirectLightingHandler& Indirect, DeferredRenderer& Deferred, LightCombiner& Combined)
		{
			PrepBuffer.Bind();

			PrepShader.Bind();

			Deferred.Deferred.BindImage(0, 0); 
			Deferred.Deferred.BindImage(2, 1);
			Sky.SkyIncident.BindImage(1, 2); 
			Combined.CombinedLighting.BindImage(3); 
			Indirect.TemporallyFiltered.BindImage(1, 4); 

			DrawPostProcessQuad(); 

			PrepShader.UnBind();

			PrepBuffer.UnBind();

			//Glow: 

			PrepBuffer.BindImage(0, 0);
			PrepBuffer.BindImage(0, 1);

			float Lods[2] = { 1.0, 0.0 }; 

			for (int j = 0; j < 3; j++) {


				if (j != 0) {

					GlowBuffer[j][1].Bind(); 

					GlowPrepShader.Bind(); 

					GlowPrepShader.SetUniform("Mix", j == 2 ? 0.5f : 0.33f);
					GlowPrepShader.SetUniform("Lod", Lods[j-1]);
					GlowPrepShader.SetUniform("LensFlare", j == 2);

					DrawPostProcessQuad(); 

					GlowPrepShader.UnBind(); 

					GlowBuffer[j][1].UnBind();
					GlowBuffer[j][1].BindImage(0);


				}


				for (int x = 0; x < 2; x++) {

					GlowBuffer[j][x].Bind();

					BlurShader[x].Bind();

					BlurShader[x].SetUniform("InverseRes", 1.0f / Vector2f(Window.GetResolution() / Divisors[j]));
					BlurShader[x].SetUniform("j", j);

					DrawPostProcessQuad();

					BlurShader[x].UnBind();

					GlowBuffer[j][x].UnBind();

					GlowBuffer[j][x].BindImage(0);

				}


			}

			/*
			//Dof: 

			PrepBuffer.BindImage(1, 0);
			Deferred.Deferred.BindDepthImage(1); 

			for (int x = 0; x < 2; x++) {

				DoFBuffer[x].Bind();

				DofShader[x].Bind();

				DrawPostProcessQuad();

				DofShader[x].UnBind();

				DoFBuffer[x].UnBind();

				DoFBuffer[x].BindImage(0);

			}
*/
		}
		
	}

}

