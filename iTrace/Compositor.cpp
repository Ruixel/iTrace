#include "Indirect.h"
#include "Compositor.h"
#include "BooleanCommands.h"

namespace iTrace {

	namespace Rendering {

		const Matrix3f ACESInputMat = Matrix3f(
			0.59719, 0.07600, 0.02840,
			0.35458, 0.90834, 0.13383,
			0.04823, 0.01566, 0.83777
		);

		// ODT_SAT => XYZ => D60_2_D65 => sRGB
		const Matrix3f ACESOutputMat = Matrix3f(
			1.60475, -0.10208, -0.00327,
			-0.53108, 1.10813, -0.07276,
			-0.07367, -0.00605, 1.07602
		);

		float ODTtoRRT(float v) {

			float f1 = 0.4329510f * v - 0.0245786f;
			float f2 = 4.0 * (0.983729f * v - 1) * (0.238081f * v + 0.000090537f);
			float f3 = 0.4329510f * v;
			float f4 = 0.0245786f;

			return (-sqrt(f1 * f1 - f2) - f3 + f4) / (2.0 * (0.983729f * v - 1));

		}

		void Compositing::PrepareCompositing(Window & Window)
		{

			RequestBoolean("hdrpostprocess", true);

			CompositorShader = Shader("Shaders/Composite"); 
			ToneMapShader = Shader("Shaders/Tonemap"); 
			SetShaderUniforms(); 

			BlitFBO = FrameBufferObjectPreviousData(Window.GetResolution(), GL_RGB16F, false, true); 
			AverageColor = ShaderBuffer<Vector4f>::Create(1); 

			LensDirt = LoadTextureGL("Resources/Post Process/Lens Dirt.png"); 

		}

		float lum(Vector3f c) {
			return glm::dot(c, Vector3f(0.2126, 0.7152, 0.0722)); 
		}


		void Compositing::DoCompositing(Window& Window, Camera& Camera, DeferredRenderer& Deferred, LightCombiner& Combined, PostProcess& Glow)
		{

			
			BlitFBO.Bind(); 

			CompositorShader.Bind();

			CompositorShader.SetUniform("znear", Camera.znear); 
			CompositorShader.SetUniform("zfar", Camera.zfar);
			CompositorShader.SetUniform("DoGlow", GetBoolean("hdrpostprocess"));
			CompositorShader.SetUniform("FocusPoint", Vector2i(960,540));

			Combined.CombinedRefraction.BindImage(0,0); 
			Deferred.Deferred.BindDepthImage(1); 
			Glow.GlowBuffer[1].BindImage(0,2); 
			Glow.GlowBuffer[1].BindImage(1,3);
			Glow.GlowBuffer[1].BindImage(2, 4);
			Glow.GlowBuffer[1].BindImage(3, 5);

			Glow.DoFBuffer.BindImage(6);
			LensDirt.Bind(7); 
			BlitFBO.BindImagePrevious(8); 
			AverageColor.Bind(0); 

			DrawPostProcessQuad(); 

			CompositorShader.UnBind(); 

			BlitFBO.UnBind(); 

			auto AvgColor = AverageColor.GetData();

			float luminance = lum(glm::pow(Vector3f(*AvgColor), Vector3f(1.0/2.2))); 

			CurrentExposure = ODTtoRRT(0.55) / luminance;

			CurrentExposure = glm::clamp(CurrentExposure, 0.1f, 30.0f); 

			Exposure = glm::mix(Exposure, CurrentExposure, 0.33f*glm::min(Window.GetFrameTime(), 1.0f)); 

			AverageColor.UnMap();

			glBindFramebuffer(GL_FRAMEBUFFER, NULL);
			glViewport(0, 0, BlitFBO.Buffers[0].Resolution.x, BlitFBO.Buffers[0].Resolution.y);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

			BlitFBO.BindImage(0); 
			

			ToneMapShader.Bind(); 

			ToneMapShader.SetUniform("Exposure", Exposure);
			
			DrawPostProcessQuad(); 

			ToneMapShader.UnBind();

		}

		void Compositing::ReloadCompositing()
		{
			CompositorShader.Reload("Shaders/Composite");
			ToneMapShader.Reload("Shaders/Tonemap");
			SetShaderUniforms();
		}

		void Compositing::SetShaderUniforms()
		{
			CompositorShader.Bind();

			CompositorShader.SetUniform("CombinedLighting", 0);
			CompositorShader.SetUniform("Depth", 1);
			CompositorShader.SetUniform("Glow[0]", 2);
			CompositorShader.SetUniform("Glow[1]", 3);
			CompositorShader.SetUniform("Glow[2]", 4);
			CompositorShader.SetUniform("Glow[3]", 5);

			CompositorShader.SetUniform("DoF", 6);
			CompositorShader.SetUniform("LensDirt", 7);
			CompositorShader.SetUniform("PreviousComposite", 8);

			CompositorShader.UnBind();

			ToneMapShader.Bind(); 

			ToneMapShader.SetUniform("Composited", 0); 

			ToneMapShader.UnBind(); 

		}

	}


}