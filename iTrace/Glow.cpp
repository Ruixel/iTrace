#include "Glow.h"

namespace iTrace {

	namespace Rendering {

		float Aperture = 1.0/.3; 
		float ImageDistance = 12.0;
		float FocusPoint = 0.5; 

		void PostProcess::PreparePostProcess(Window& Window, Camera& Camera)
		{

			PrepBuffer = MultiPassFrameBufferObject(Window.GetResolution() / 4, 1, { GL_RGB16F }, false, true); 
			DoFPrepBuffer = MultiPassFrameBufferObject(Window.GetResolution() / 4, 1, { GL_RGBA16F }, false, true);

			for (int i = 0; i < 2; i++) {
				for(int j = 0; j < 3; j++)
					GlowBuffer[j][i] = FrameBufferObject(Window.GetResolution() / Divisors[j], GL_RGB16F, false, false);
			}
			DoFBuffer = FrameBufferObject(Window.GetResolution() / 4, GL_RGBA16F, false, false);
			PrepShader = Shader("Shaders/PrepShader");
			BlurShader[0] = Shader("Shaders/BlurHorizontal");
			BlurShader[1] = Shader("Shaders/BlurVertical");
			DofShader = Shader("Shaders/DoF"); 
			GlowPrepShader = Shader("Shaders/BlurCombiner"); 
			DoFPrepShader = Shader("Shaders/DoFPrep"); 

			SetShaderUniforms(Window, Camera); 

		}

		void PostProcess::RenderPostProcess(Window& Window, SkyRendering& Sky, LightManager& Indirect, DeferredRenderer& Deferred, LightCombiner& Combined)
		{

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::I)) {
				Aperture += 1.0 * Window.GetFrameTime(); 
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::K)) {
				Aperture -= 1.0 * Window.GetFrameTime();
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::U)) {
				FocusPoint += 1.0 * Window.GetFrameTime();
			}
			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::J)) {
				FocusPoint -= 1.0 * Window.GetFrameTime();
			}

			PrepBuffer.Bind();

			PrepShader.Bind();

			Deferred.Deferred.BindImage(0, 0); 
			Deferred.Deferred.BindImage(2, 1);
			Sky.SkyIncident.BindImage(1, 2); 
			Combined.CombinedLighting.BindImage(0,3); 
			Indirect.TemporallyFiltered.BindImage(1, 4); 
			Combined.CombinedLighting.BindImage(1, 5);
			Indirect.TemporallyFiltered.BindImage(3, 6);


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

					GlowPrepShader.SetUniform("Mix", j == 2 ? 0.75f : 0.5f);
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

			Profiler::SetPerformance("Glow"); 


			DoFPrepBuffer.Bind(); 

			DoFPrepShader.Bind(); 

			Combined.CombinedLighting.BindImage(0, 0);
			GlowBuffer[2][1].BindImage(1);
			Deferred.RawDeferred.BindDepthImage(2);
			DoFPrepShader.SetUniform("Aperture", Aperture);
			DoFPrepShader.SetUniform("ImageDistance", ImageDistance);
			DoFPrepShader.SetUniform("FocusPoint", FocusPoint);

			DrawPostProcessQuad(); 

			DoFPrepShader.UnBind();

			DoFPrepBuffer.UnBind(); 

			//Dof: 

			DoFPrepBuffer.BindImage(0,0); 
			Deferred.RawDeferred.BindDepthImage(1); 

			DoFBuffer.Bind();

			DofShader.Bind();

			DofShader.SetUniform("Aperture", Aperture); 
			DofShader.SetUniform("ImageDistance", ImageDistance); 
			DofShader.SetUniform("FocusPoint", FocusPoint);

			DrawPostProcessQuad();

			DofShader.UnBind();

			DoFBuffer.UnBind();

			Profiler::SetPerformance("DoF");

			

		}

		void PostProcess::ReloadPostProcess(Window& Window, Camera & Camera)
		{

			PrepShader.Reload("Shaders/PrepShader");
			BlurShader[0].Reload("Shaders/BlurHorizontal");
			BlurShader[1].Reload("Shaders/BlurVertical");
			DofShader.Reload("Shaders/DoF");
			GlowPrepShader.Reload("Shaders/BlurCombiner");
			DoFPrepShader.Reload("Shaders/DoFPrep");

			SetShaderUniforms(Window, Camera); 
		}

		void PostProcess::SetShaderUniforms(Window& Window, Camera & Camera)
		{
			PrepShader.Bind();

			PrepShader.SetUniform("Normal", 0);
			PrepShader.SetUniform("Albedo", 1);
			PrepShader.SetUniform("SkyReigh", 2);
			PrepShader.SetUniform("Lighting", 3);
			PrepShader.SetUniform("Volumetrics", 4);
			PrepShader.SetUniform("Glow", 5);
			PrepShader.SetUniform("Clouds", 6);

			PrepShader.UnBind();

			for (int x = 0; x < 2; x++) {

				BlurShader[x].Bind();
				BlurShader[x].SetUniform("InverseRes", 1.0f / Vector2f(Window.GetResolution() / 4));
				BlurShader[x].SetUniform("Input", 0);
				BlurShader[x].UnBind();

			}

			DofShader.Bind();
			DofShader.SetUniform("InverseRes", 1.0f / Vector2f(Window.GetResolution() / 4));
			DofShader.SetUniform("Input", 0);
			DofShader.SetUniform("Depth", 1);
			DofShader.SetUniform("znear", Camera.znear);
			DofShader.SetUniform("zfar", Camera.zfar);
			DofShader.SetUniform("MaxRadius", 50.0f);
			DofShader.SetUniform("ApertureDistance", 2.5f);
			DofShader.SetUniform("ApertureDistance", 2.5f);

			DofShader.UnBind();


			GlowPrepShader.Bind();


			GlowPrepShader.SetUniform("GlowInput", 0);
			GlowPrepShader.SetUniform("RawInput", 1);


			GlowPrepShader.UnBind();

			DoFPrepShader.Bind();

			DoFPrepShader.SetUniform("Lighting", 0);
			DoFPrepShader.SetUniform("Glow", 1);
			DoFPrepShader.SetUniform("Depth", 2);
			DoFPrepShader.SetUniform("znear", Camera.znear);
			DoFPrepShader.SetUniform("zfar", Camera.zfar);
			DoFPrepShader.SetUniform("MaxRadius", 50.0f);

			DoFPrepShader.UnBind();
		}
		
	}

}