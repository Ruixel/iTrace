#include "LightCombiner.h"
#include "BooleanCommands.h"

namespace iTrace {

	namespace Rendering {




		void LightCombiner::PrepareLightCombiner(Window& Window)
		{

			CombinedLighting = MultiPassFrameBufferObject(Window.GetResolution(), 2, { GL_RGB16F,GL_RGB16F }, false);
			CombinedRefraction = MultiPassFrameBufferObject(Window.GetResolution(), 3, { GL_RGBA16F, GL_RGB16F, GL_R32F }, false, true); 
			LightCombinerShader = Shader("Shaders/LightCombiner"); 
			RefractiveCombiner = Shader("Shaders/RefractiveCombiner"); 

			FocusSSBO = ShaderBuffer<Vector4f>::Create(1); 

			RequestBoolean("noalbedo", false); 

			SetUniforms(Window); 
		}

		float LinearZ(float z, Camera & Cam) {

			return 2.0 * Cam.znear * Cam.zfar / (Cam.zfar + Cam.znear - (z * 2. - 1.) * (Cam.zfar - Cam.znear));

		}

		void LightCombiner::CombineLighting(Window& Window, Camera & Camera, LightManager& Indirect, DeferredRenderer& Deferred, SkyRendering& Sky, ParticleSystem& Particles)
		{

			CombinedLighting.Bind(); 

			LightCombinerShader.Bind(); 

			Indirect.SpatialyUpscaled.BindImage(0, 0);
			Indirect.SpatialyUpscaled.BindImage(1, 1);
			
			//Indirect.RawPathTrace[Window.GetFrameCount() % 4].BindImage(0, 0); 

			Indirect.TemporallyFiltered.BindImage(1, 12); 
			Deferred.Deferred.BindImage(4, 13);

			Deferred.Deferred.BindImage(2, 2);
			Deferred.Deferred.BindImage(0, 3);
			Deferred.Deferred.BindImage(3, 4);

			Sky.SkyIncident.BindImage(0, 5);
			Deferred.Deferred.BindImage(1, 6);

			for (int i = 0; i < 5; i++) {
				Sky.ShadowMaps[i].BindDepthImage(i + 7);

				std::string Title = "ShadowMatrices[" + std::to_string(i) + "]";

				LightCombinerShader.SetUniform(Title, Sky.ProjectionMatrices[i] * Sky.ViewMatrices[i]);

			}

			Deferred.Deferred.BindImage(7, 14);
			Indirect.PreUpscaled.BindImage(0, 15); 
			//Indirect.ProjectedClouds.BindImage(15); 

			Particles.ParticleSystemFBO.BindImage(16); 
			Particles.ParticleSystemFBO.BindDepthImage(17); 
			Deferred.RawDeferred.BindDepthImage(18); 

			glActiveTexture(GL_TEXTURE19); 
			glBindTexture(GL_TEXTURE_CUBE_MAP, Sky.SkyCube.Texture[0]); 
			
			Deferred.DeferredRefractive.BindImage(20);

			Indirect.SpatialyUpscaled.BindImage(2, 21);

			Deferred.PrimaryDeferredRefractive.BindDepthImage(22);
			Deferred.PrimaryDeferredRefractive.BindImage(0,23);
			Deferred.PrimaryDeferredRefractive.BindImage(1,24);
			Sky.SkyIncident.BindImage(1, 25);
			Deferred.RawWaterDeferred.BindDepthImage(26);
			glActiveTexture(GL_TEXTURE27);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Deferred.WaterCaustics);
			

			Indirect.PreUpscaled.BindImage(2, 28);

			LightCombinerShader.SetUniform("LightDirection", Sky.Orientation);
			LightCombinerShader.SetUniform("SunColor", Sky.SunColor);
			LightCombinerShader.SetUniform("CameraPosition", Camera.Position);
			LightCombinerShader.SetUniform("UseSpec", sf::Keyboard::isKeyPressed(sf::Keyboard::J));
			LightCombinerShader.SetUniform("NoAlbedo", GetBoolean("noalbedo"));
			LightCombinerShader.SetUniform("InverseProj", glm::inverse(Camera.Project));
			LightCombinerShader.SetUniform("InverseView", glm::inverse(Camera.View));
			LightCombinerShader.SetUniform("IdentityMatrix", Camera.Project * Camera.View); 
			LightCombinerShader.SetUniform("Time", Window.GetTimeOpened());

			DrawPostProcessQuad();

			LightCombinerShader.UnBind(); 

			CombinedLighting.UnBind(); 

			RefractiveCombiner.Bind(); 

			CombinedRefraction.Bind(); 

			CombinedLighting.BindImage(0, 0); 
			CombinedLighting.BindImage(1, 1);
			Deferred.RawDeferred.BindDepthImage(2);

			Deferred.PrimaryDeferredRefractive.BindDepthImage(3);
			Deferred.PrimaryDeferredRefractive.BindImage(0, 4);
			Deferred.PrimaryDeferredRefractive.BindImage(1, 5);
			Indirect.TemporallyFiltered.BindImage(1, 6);
			Deferred.DeferredRefractive.BindImage(7);
			Deferred.PrimaryDeferredRefractive.BindImage(2, 8);
			Indirect.RefractedWater.BindImage(0, 9); 
			Deferred.RawWaterDeferred.BindImage(3, 10);
			Deferred.RawWaterDeferred.BindDepthImage(11);
			Deferred.RawWaterDeferred.BindImage(1, 12);
			Deferred.RawWaterDeferred.BindImage(2, 13);
			Indirect.SpatialyUpscaled.BindImage(1, 18);
			Indirect.RefractedWater.BindImage(2, 19);
			Indirect.ErrorMaskBlur[1].BindImage(20); 

			

			FocusSSBO.Bind(0);
			
			

			RefractiveCombiner.SetUniform("InverseProj", glm::inverse(Camera.Project));
			RefractiveCombiner.SetUniform("InverseView", glm::inverse(Camera.View));
			RefractiveCombiner.SetUniform("IdentityMatrix", Camera.Project * Camera.View);
			RefractiveCombiner.SetUniform("CameraPosition", Camera.Position);
			RefractiveCombiner.SetUniform("PixelFocusPoint", Window.GetResolution() / 2); 
			RefractiveCombiner.SetUniform("znear", Camera.znear); 
			RefractiveCombiner.SetUniform("zfar", Camera.zfar);
			RefractiveCombiner.SetUniform("LightDirection", Sky.Orientation);
			RefractiveCombiner.SetUniform("SunColor", Sky.SunColor);

			for (int i = 0; i < 4; i++) {
				Sky.ShadowMaps[i].BindDepthImage(i + 14);

				std::string Title = "ShadowMatrices[" + std::to_string(i) + "]";

				RefractiveCombiner.SetUniform(Title, Sky.ProjectionMatrices[i] * Sky.ViewMatrices[i]);

			}


			DrawPostProcessQuad(); 

			RefractiveCombiner.UnBind();

			CombinedRefraction.UnBind();

			auto FocusPointBuffer = FocusSSBO.GetData(); 

			

			CurrentFocusPoint = LinearZ(FocusPointBuffer[0].x, Camera);
			CurrentFocusPoint = glm::min(CurrentFocusPoint, 20.0f); 

			float vec = (CurrentFocusPoint - ActualFocusPoint); 

			vec = glm::sign(vec) * 8.0 * sqrt(glm::min(glm::abs(vec), 1.0f)); 


			PreviousFocusPoint = ActualFocusPoint; 
			ActualFocusPoint = ActualFocusPoint + vec * glm::min(Window.GetFrameTime(),1.0f);

			FocusPoint = (PreviousFocusPoint + ActualFocusPoint) / 2.0f; 


			FocusSSBO.UnMap(); 

		}

		void LightCombiner::SetUniforms(Window& Window)
		{
			LightCombinerShader.Bind();

			LightCombinerShader.SetUniform("Indirect", 0);
			LightCombinerShader.SetUniform("IndirectSpecular", 1);
			LightCombinerShader.SetUniform("Albedo", 2);
			LightCombinerShader.SetUniform("Normal", 3);
			LightCombinerShader.SetUniform("HighfreqNormal", 4);

			LightCombinerShader.SetUniform("Sky", 5);
			LightCombinerShader.SetUniform("WorldPos", 6);

			LightCombinerShader.SetUniform("ShadowMaps[0]", 7);
			LightCombinerShader.SetUniform("ShadowMaps[1]", 8);
			LightCombinerShader.SetUniform("ShadowMaps[2]", 9);
			LightCombinerShader.SetUniform("ShadowMaps[3]", 10);
			LightCombinerShader.SetUniform("ShadowMaps[4]", 11);

			LightCombinerShader.SetUniform("Volumetrics", 12);
			LightCombinerShader.SetUniform("DirectMultiplier", 13);
			LightCombinerShader.SetUniform("SimpleLight", 14);
			LightCombinerShader.SetUniform("Clouds", 15);

			LightCombinerShader.SetUniform("Particles", 16);
			LightCombinerShader.SetUniform("ParticleDepth", 17);
			LightCombinerShader.SetUniform("Depth", 18);

			LightCombinerShader.SetUniform("SkyCube", 19);

			LightCombinerShader.SetUniform("RefractiveBlocks", 20);
			LightCombinerShader.SetUniform("DirectShadow", 21);

			LightCombinerShader.SetUniform("PrimaryRefractionDepth", 22);
			LightCombinerShader.SetUniform("PrimaryRefractionColor", 23);
			LightCombinerShader.SetUniform("PrimaryRefractionNormal", 24);
			LightCombinerShader.SetUniform("SkyReigh", 25);
			LightCombinerShader.SetUniform("WaterDepth", 26);
			LightCombinerShader.SetUniform("WaterCaustics", 27);
			LightCombinerShader.SetUniform("Detail", 28);

			
			LightCombinerShader.UnBind();

			RefractiveCombiner.Bind(); 

			RefractiveCombiner.SetUniform("CombinedLighting",0); 
			RefractiveCombiner.SetUniform("CombinedGlow", 1);

			RefractiveCombiner.SetUniform("Depth", 2);

			RefractiveCombiner.SetUniform("PrimaryRefractionDepth", 3);
			RefractiveCombiner.SetUniform("PrimaryRefractionColor", 4);
			RefractiveCombiner.SetUniform("PrimaryRefractionNormal", 5);
			RefractiveCombiner.SetUniform("Volumetrics", 6);
			RefractiveCombiner.SetUniform("RefractiveBlocks", 7);
			RefractiveCombiner.SetUniform("PrimaryRefractionNormalLF", 8);
			RefractiveCombiner.SetUniform("WaterRefraction", 9);
			RefractiveCombiner.SetUniform("WaterAlbedo", 10);
			RefractiveCombiner.SetUniform("WaterDepth", 11);
			RefractiveCombiner.SetUniform("WaterPosition", 12);
			RefractiveCombiner.SetUniform("WaterNormal", 13);

			RefractiveCombiner.SetUniform("ShadowMaps[0]", 14);
			RefractiveCombiner.SetUniform("ShadowMaps[1]", 15);
			RefractiveCombiner.SetUniform("ShadowMaps[2]", 16);
			RefractiveCombiner.SetUniform("ShadowMaps[3]", 17);

			RefractiveCombiner.SetUniform("IndirectSpecular", 18);
			RefractiveCombiner.SetUniform("WaterReflection", 19);
			RefractiveCombiner.SetUniform("WaterError", 20);


			RefractiveCombiner.UnBind(); 

			
		}

		void LightCombiner::ReloadLightCombiner(Window& Window)
		{
			LightCombinerShader.Reload("Shaders/LightCombiner");
			RefractiveCombiner.Reload("Shaders/RefractiveCombiner");
			SetUniforms(Window);
		}

	}

}