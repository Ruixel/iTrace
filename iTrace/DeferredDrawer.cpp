#include "DeferredDrawer.h"
#include "Texture.h"
#include "ParallaxBaker.h"
#include "BooleanCommands.h"
#include "RainDropBaker.h"
#include "Weather.h"
#include "Profiler.h"

namespace iTrace {

	namespace Rendering {

		
		TextureGL TestStoneTexture; 

		void DeferredRenderer::PrepareDeferredRenderer(Window& Window)
		{

			DeferredManager = Shader("Shaders/Deferred"); 
			DeferredUnwrapper = Shader("Shaders/DeferredUnwrapper"); 
			TransparentDeferredManager = Shader("Shaders/DeferredTransparent"); 
			Deferred = MultiPassFrameBufferObjectPreviousData(Window.GetResolution(), 8, { GL_RGBA16F, GL_RGB32F,GL_RGBA16F, GL_RGBA16F, GL_R16F,GL_RGBA16F,GL_RGB16F,GL_RGB16F }, false);
			RawDeferred = FrameBufferObject(Window.GetResolution(), GL_RGBA16F); 
			DeferredRefractive = FrameBufferObject(Window.GetResolution(), GL_RGBA16F, false); 
			TestStoneTexture = LoadTextureGL("Materials/Stone/Albedo.png"); 

			Noise = LoadTextureGL("Textures/Noise.png",GL_RED); 

			RequestBoolean("parallax", false); 

			SetUniforms(Window); 


			glGenTextures(1, &RainDrop); 
			glBindTexture(GL_TEXTURE_2D_ARRAY, RainDrop); 
			glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, RAINDROP_TEX, RAINDROP_TEX, RAINDROP_FRAMES, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr); 

			for (int i = 0; i < RAINDROP_FRAMES; i++) {

				sf::Image Image; 
				Image.loadFromFile("Resources/BakeData/RainDrop_" + std::to_string(i) + ".png"); 

				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, RAINDROP_TEX, RAINDROP_TEX, 1, GL_RGBA, GL_UNSIGNED_BYTE, Image.getPixelsPtr());

			}

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glGenerateMipmap(GL_TEXTURE_2D_ARRAY); 

			glBindTexture(GL_TEXTURE_2D_ARRAY, 0); 


		}

		void DeferredRenderer::RenderDeferred(SkyRendering& Sky, Window& Window, Camera& Camera, WorldManager& World, Vector3f& SunDirection)
		{

			Profiler::FlushTime(); 

			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

			RawDeferred.Bind(); 

			DeferredManager.Bind();

			World.RenderWorld(Camera, DeferredManager);

			DeferredManager.UnBind();

			TransparentDeferredManager.Bind(); 

			TransparentDeferredManager.SetUniform("Time", Window.GetTimeOpened()); 

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_1D, Chunk::GetBlockDataTexture());

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_1D, Chunk::GetTextureExtensionData());

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(6));

			World.RenderWorldTransparent(Camera, DeferredManager); 

			TransparentDeferredManager.UnBind(); 

			RawDeferred.UnBind();

			glDisable(GL_DEPTH_TEST); 

			Deferred.Bind(); 

			Profiler::SetPerformance("Deferred polling");


			DeferredUnwrapper.Bind();

			glActiveTexture(GL_TEXTURE0); 
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(0)); 

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(3));

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(4));

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_1D, Chunk::GetBlockDataTexture());

			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_1D, Chunk::GetTextureExtensionData());

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_3D, World.LightContainer);

			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_1D, Chunk::GetBlockExtraDataTexture());

			glActiveTexture(GL_TEXTURE11);
			glBindTexture(GL_TEXTURE_2D, Noise.ID);

			glActiveTexture(GL_TEXTURE12);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(1));

			glActiveTexture(GL_TEXTURE13);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(2));

			glActiveTexture(GL_TEXTURE14);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(5));

			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D_ARRAY, RainDrop);

			Sky.ShadowMaps[4].BindDepthImage(18); 

			RawDeferred.BindImage(9); 
			RawDeferred.BindDepthImage(10); 

			DeferredUnwrapper.SetUniform("InverseProj", glm::inverse(Camera.Project)); 
			DeferredUnwrapper.SetUniform("InverseView", glm::inverse(Camera.View));
			DeferredUnwrapper.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));
			DeferredUnwrapper.SetUniform("CameraPosition", Camera.Position); 
			DeferredUnwrapper.SetUniform("Time", Window.GetTimeOpened());
			DeferredUnwrapper.SetUniform("SunDirection", SunDirection);
			DeferredUnwrapper.SetUniform("ParallaxDirections", BAKE_DIRECTIONS);
			DeferredUnwrapper.SetUniform("ParallaxResolution", BAKE_RESOLUTION);
			DeferredUnwrapper.SetUniform("DoParallax", GetBoolean("parallax"));
			DeferredUnwrapper.SetUniform("RainFrames", RAINDROP_FRAMES);
			DeferredUnwrapper.SetUniform("WetNess", GetGlobalWeatherManager().GetWeather().Wetness);
			DeferredUnwrapper.SetUniform("RainMatrix", Sky.ProjectionMatrices[4] * Sky.ViewMatrices[4]);
			DeferredUnwrapper.SetUniform("PositionBias", Vector2i(World.BiasX, World.BiasY)); 

			DrawPostProcessQuad(); 

			DeferredUnwrapper.UnBind(); 

			Deferred.UnBind(); 

			Profiler::SetPerformance("Deferred unwrapping"); 
			
		}

		void DeferredRenderer::SetUniforms(Window& Window)
		{

			DeferredUnwrapper.Bind();

			DeferredUnwrapper.SetUniform("DiffuseTextures", 0);
			DeferredUnwrapper.SetUniform("DisplacementTextures", 1);
			DeferredUnwrapper.SetUniform("EmissiveTextures", 2);
			DeferredUnwrapper.SetUniform("TextureData", 3);
			DeferredUnwrapper.SetUniform("LightData", 4);
			DeferredUnwrapper.SetUniform("TextureExData", 5);
			DeferredUnwrapper.SetUniform("BlockData", 7);
			DeferredUnwrapper.SetUniform("InTexCoord", 9);
			DeferredUnwrapper.SetUniform("InDepth", 10);
			DeferredUnwrapper.SetUniform("Noise", 11);
			DeferredUnwrapper.SetUniform("NormalTextures", 12);
			DeferredUnwrapper.SetUniform("RoughnessTextures", 13);
			DeferredUnwrapper.SetUniform("MetalnessTextures", 14);
			DeferredUnwrapper.SetUniform("RainDrop", 15);
			DeferredUnwrapper.SetUniform("ShadowMap", 18);

			DeferredUnwrapper.UnBind();

			TransparentDeferredManager.Bind(); 

			TransparentDeferredManager.SetUniform("TextureData", 0);
			TransparentDeferredManager.SetUniform("TextureExData", 1);
			TransparentDeferredManager.SetUniform("OpacityTextures", 2);

			TransparentDeferredManager.UnBind(); 

		}

		void DeferredRenderer::ReloadDeferred(Window& Window)
		{
			DeferredManager.Reload("Shaders/Deferred");
			DeferredUnwrapper.Reload("Shaders/DeferredUnwrapper");
			TransparentDeferredManager.Reload("Shaders/DeferredTransparent"); 
			SetUniforms(Window); 
		}

	}

}