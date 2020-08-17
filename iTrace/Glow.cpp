#include "Glow.h"

namespace iTrace {

	namespace Rendering {

		float Aperture = 1.0/.3; 
		float ImageDistance = 12.0;
		float FocusPoint = 0.5; 

		void PostProcess::PreparePostProcess(Window& Window, Camera& Camera)
		{

			DoFPrepBuffer = MultiPassFrameBufferObject(Window.GetResolution() / 4, 1, { GL_RGBA16F }, false, true);

			GlowBuffer[0] = MultiPassFrameBufferObject(Window.GetResolution() / 8, 4, { GL_RGBA16F, GL_RGBA16F,GL_RGBA16F,GL_RGBA16F }, false);
			GlowBuffer[1] = MultiPassFrameBufferObject(Window.GetResolution() / 4, 5, { GL_RGBA16F, GL_RGBA16F,GL_RGBA16F,GL_RGBA16F,GL_RGB16F }, false);



			GlowPrepBuffer = MultiPassFrameBufferObject(Window.GetResolution() / 4, 4, { GL_RGBA16F, GL_RGBA16F,GL_RGBA16F,GL_RGBA16F }, false);



			SmartDownScale[0] = FrameBufferObject(Window.GetResolution() / 2, GL_RGBA16F, false, false); 
			SmartDownScale[1] = FrameBufferObject(Window.GetResolution() / 4, GL_RGBA16F, false, false);

			DoFBuffer = FrameBufferObject(Window.GetResolution() / 4, GL_RGBA16F, false, false);
			GlowShader = Shader("Shaders/Glow"); 
			DofShader = Shader("Shaders/DoF"); 
			GlowPrepShader = Shader("Shaders/BlurCombiner"); 
			DoFPrepShader = Shader("Shaders/DoFPrep"); 
			SmartDownScaler = Shader("Shaders/IntelligentDownScale"); 
			LensDirt = LoadTextureGL("Resources/Post Process/Lens Dirt.png"); 

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
			/*
			Combined.CombinedRefraction.BindImage(0, 0);

			SmartDownScaler.Bind(); 

			for (int i = 0; i < 2; i++) {

				SmartDownScale[i].Bind(); 

				DrawPostProcessQuad();

				SmartDownScale[i].UnBind(); 

				SmartDownScale[i].BindImage(0); 

			}

			SmartDownScaler.UnBind(); 

			Profiler::SetPerformance("Smart downscaling"); 

			*/
			
			Combined.CombinedRefraction.BindImage(1, 0); 

			for (int i = 0; i < 2; i++) {

				GlowPrepBuffer.Bind(); 

				GlowPrepShader.Bind(); 

				GlowPrepShader.SetUniform("First", i == 0); 
				GlowPrepShader.SetUniform("Mix", i == 0 ? 1.0f : 0.0f); 
				GlowPrepShader.SetUniform("Lod", i == 0 ? 3.0f : 2.0f); 

				DrawPostProcessQuad(); 

				GlowPrepShader.UnBind();

				GlowPrepBuffer.UnBind(); 

				GlowPrepBuffer.BindImage(0, 0); 
				GlowPrepBuffer.BindImage(1, 1);
				GlowPrepBuffer.BindImage(2, 2);
				GlowPrepBuffer.BindImage(3, 3);
				Combined.CombinedRefraction.BindImage(0, 4);
				//SmartDownScale[1].BindImage(4);

				GlowShader.Bind(); 

				GlowBuffer[i].Bind(); 

				GlowShader.SetUniform("PixelStep", i * 5 + 1);

				DrawPostProcessQuad(); 

				GlowBuffer[i].UnBind();

				GlowShader.UnBind();

				GlowBuffer[i].BindImage(0, 1); 
				GlowBuffer[i].BindImage(1, 2);
				GlowBuffer[i].BindImage(2, 3);
				GlowBuffer[i].BindImage(3, 4);

			}


			Profiler::SetPerformance("Glow"); 


			DoFPrepBuffer.Bind(); 

			DoFPrepShader.Bind(); 

			Combined.CombinedRefraction.BindImage(0, 0);
			GlowBuffer[1].BindImage(4,1);
			Combined.CombinedRefraction.BindImage(2, 2);
			LensDirt.Bind(3); 
			DoFPrepShader.SetUniform("Aperture", Aperture);
			DoFPrepShader.SetUniform("ImageDistance", ImageDistance);
			DoFPrepShader.SetUniform("FocusPoint", Combined.FocusPoint);

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
			DofShader.SetUniform("FocusPoint", Combined.FocusPoint);

			DrawPostProcessQuad();

			DofShader.UnBind();

			DoFBuffer.UnBind();

			Profiler::SetPerformance("DoF");

			

		}

		void PostProcess::ReloadPostProcess(Window& Window, Camera & Camera)
		{

			GlowShader.Reload("Shaders/Glow"); 
			DofShader.Reload("Shaders/DoF");
			GlowPrepShader.Reload("Shaders/BlurCombiner");
			DoFPrepShader.Reload("Shaders/DoFPrep");
			SmartDownScaler.Reload("Shaders/IntelligentDownScale"); 

			SetShaderUniforms(Window, Camera); 
		}

		void PostProcess::SetShaderUniforms(Window& Window, Camera & Camera)
		{
			
		
			GlowPrepShader.Bind(); 

			GlowPrepShader.SetUniform("RawInput", 0); 
			GlowPrepShader.SetUniform("GlowInput1", 1);
			GlowPrepShader.SetUniform("GlowInput2", 2);
			GlowPrepShader.SetUniform("GlowInput3", 3);
			GlowPrepShader.SetUniform("GlowInput4", 4);


			GlowPrepShader.UnBind(); 

			GlowShader.Bind(); 

			GlowShader.SetUniform("Input1", 0); 
			GlowShader.SetUniform("Input2", 1);
			GlowShader.SetUniform("Input3", 2);
			GlowShader.SetUniform("Input4", 3);
			GlowShader.SetUniform("RawInput", 4);

			GlowShader.SetUniform("InverseRes", 1.0f / Vector2f(Window.GetResolution()/8));

			GlowShader.UnBind(); 






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


			
			DoFPrepShader.Bind();

			DoFPrepShader.SetUniform("Lighting", 0);
			DoFPrepShader.SetUniform("Glow", 1);
			DoFPrepShader.SetUniform("Depth", 2);
			DoFPrepShader.SetUniform("LensDirt", 3);
			DoFPrepShader.SetUniform("znear", Camera.znear);
			DoFPrepShader.SetUniform("zfar", Camera.zfar);
			DoFPrepShader.SetUniform("MaxRadius", 50.0f);

			DoFPrepShader.UnBind();

			SmartDownScaler.Bind(); 

			SmartDownScaler.SetUniform("Input", 0); 

			SmartDownScaler.UnBind(); 

		}
		
	}

}