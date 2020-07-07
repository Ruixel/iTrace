#include "Indirect.h"
#include "External/BlueNoiseData.h"
#include "BooleanCommands.h"
#include "ParallaxBaker.h"
#include "Weather.h"

namespace iTrace {

	namespace Rendering {



		void LightManager::PrepareIndirectLightingHandler(Window& Window)
		{
			RequestBoolean("raytracing", true); 
			RequestBoolean("volumetrics", true);
			RequestBoolean("spatial", true);
			RequestBoolean("temporal", true);

			
			for (int x = 0; x < 4; x++) {
				RawPathTrace[x] = MultiPassFrameBufferObject(Window.GetResolution() / 4, 5, { GL_RGBA16F, GL_RGBA16F, GL_RGB32F,GL_RGBA16F, GL_R16F }, false);
				MotionVectors[x] = FrameBufferObject(Window.GetResolution() / 2, GL_RGB16F, false); 
				SpatialyFiltered[x * 2] = MultiPassFrameBufferObject(Window.GetResolution() / 4, 3, { GL_RGBA16F,GL_RGBA16F,GL_RGBA16F }, false);
				SpatialyFiltered[x * 2 + 1] = MultiPassFrameBufferObject(Window.GetResolution() / 4, 3, { GL_RGBA16F,GL_RGBA16F,GL_RGBA16F }, false);
				VolumetricFBO[x] = FrameBufferObject(Window.GetResolution() / 4, GL_RGBA16F, false); 
				Clouds[x] = MultiPassFrameBufferObject(Window.GetResolution() / 4, 2, { GL_RGBA16F, GL_R32F }, false);

			}

			DirectBlockerBuffer = FrameBufferObject(Window.GetResolution() / 8, GL_RG16F, false);
			TemporalFrameCount = FrameBufferObjectPreviousData(Window.GetResolution() / 2, GL_R16F, false);
			TemporalyUpscaled = MultiPassFrameBufferObject(Window.GetResolution() / 2, 4, { GL_RGBA16F,GL_RGBA16F,GL_RGBA16F,GL_RGBA16F }, false);
			PackedSpatialData = FrameBufferObject(Window.GetResolution() / 2, GL_RGBA16F, false);
			TemporallyFiltered = MultiPassFrameBufferObjectPreviousData(Window.GetResolution() / 2, 4, { GL_RGBA16F,GL_RGBA16F,GL_RGBA16F,GL_RGBA16F }, false);
			SpatialyUpscaled = MultiPassFrameBufferObject(Window.GetResolution(), 2, { GL_RGBA16F,GL_RGBA16F }, false);
			ProjectedClouds = FrameBufferObjectPreviousData(Vector2i(512), GL_RGBA16F, false); 

			IndirectLightShader = Shader("Shaders/RawPathTracing");
			TemporalUpscaler = Shader("Shaders/TemporalUpscaler");
			SpatialFilter = Shader("Shaders/SpatialFilter"); 
			SpatialUpscaler = Shader("Shaders/SpatialUpscaler"); 
			TemporalFilter = Shader("Shaders/TemporalFilter"); 
			RTMotionVectorCalculator = Shader("Shaders/RTMotionVectors"); 
			SpatialPacker = Shader("Shaders/SpatialPacker"); 
			FrameCount = Shader("Shaders/TemporalFrameCounter");
			Volumetrics = Shader("Shaders/Volumetrics"); 
			DirectBlocker = Shader("Shaders/DirectBlocker"); 
			CloudRenderer = Shader("Shaders/Clouds"); 
			CloudProjection = Shader("Shaders/CloudProjection"); 

			SetShaderUniforms(Window); 
			

			glGenTextures(1, &SobolTexture);
			glBindTexture(GL_TEXTURE_2D, SobolTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 256, 256, 0, GL_RED, GL_UNSIGNED_BYTE, sobol_256spp_256d);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glBindTexture(GL_TEXTURE_2D, 0);

			glGenTextures(1, &RankingTexture);
			glBindTexture(GL_TEXTURE_2D, RankingTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 256, 512, 0, GL_RED, GL_UNSIGNED_BYTE, rankingTile);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glBindTexture(GL_TEXTURE_2D, 0);

			glGenTextures(1, &ScramblingTexture);
			glBindTexture(GL_TEXTURE_2D, ScramblingTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 256, 512, 0, GL_RED, GL_UNSIGNED_BYTE, scramblingTile);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glBindTexture(GL_TEXTURE_2D, 0);

			WindNoise = LoadTextureGL("Textures/Wind.jpg"); 
			SimplifiedBlueNoise = LoadTextureGL("Textures/Noise.png");
			WeatherMap = LoadTextureGL("Textures/CloudsWeather.PNG"); 

			CloudNoise = LoadTextureGL3D("Textures/CloudsWorley.png", Vector3i(128)); 

		}

		void LightManager::RenderIndirectLighting(Window& Window, Camera& Camera, DeferredRenderer& Deferred, WorldManager& World, SkyRendering& Sky)
		{


			
			
			Profiler::FlushTime(); 
			DoRawPathTrace(Window, Camera, Deferred, World, Sky); 
			Profiler::SetPerformance("Raw path tracing"); 

			Profiler::FlushTime();
			FindDirectBlocker(Window, Camera, Deferred, Sky);
			Profiler::SetPerformance("Direct blocker finder");
	
			RenderClouds(Window, Camera, Deferred, Sky);
			Profiler::SetPerformance("Cloud Rendering");

			GenerateMotionVectors(Window, Camera, Deferred);
			Profiler::SetPerformance("Motion vectors");

			DoVolumetricLighting(Window, Camera, Deferred, World, Sky);
			Profiler::SetPerformance("Volumetric lighting");

			SpatialyFilter(Window, Camera, Deferred);
			Profiler::SetPerformance("Spatial filter");

			

			TemporalyUpscale(Window, Camera, Deferred);
			Profiler::SetPerformance("Temporal upscale");

			TemporalyFilter(Window, Camera, Deferred);
			Profiler::SetPerformance("Temporal filtering");

			SpatialyUpscale(Window, Camera, Deferred); 
			Profiler::SetPerformance("Spatial upscaling");

		}

		void LightManager::FindDirectBlocker(Window& Window, Camera& Camera, DeferredRenderer& Deferred, SkyRendering& Sky)
		{

			DirectBlocker.Bind(); 

			DirectBlockerBuffer.Bind(); 

			for (int i = 0; i < 4; i++) {

				DirectBlocker.SetUniform("ShadowMatrix[" + std::to_string(i) + "]", Sky.ProjectionMatricesRaw[i] * Sky.ViewMatrices[i]);
				Sky.ShadowMaps[i].BindDepthImage(i+19);

			}

			DirectBlocker.SetUniform("znear", Camera.znear);
			DirectBlocker.SetUniform("zfar", Camera.zfar);

			Deferred.Deferred.BindImage(1, 4);
			Deferred.RawDeferred.BindDepthImage(5);

			DrawPostProcessQuad(); 

			DirectBlockerBuffer.UnBind();

			DirectBlocker.UnBind(); 

		}

		void LightManager::DoRawPathTrace(Window& Window, Camera& Camera, DeferredRenderer& Deferred, WorldManager& World, SkyRendering& Sky)
		{

			RawPathTrace[Window.GetFrameCount()%4].Bind(); 

			IndirectLightShader.Bind();

			Deferred.Deferred.BindImage(3, 0);
			Deferred.Deferred.BindImage(1, 1);
			Deferred.RawDeferred.BindDepthImage(18);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_3D, World.Chunk->ChunkTexID);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, SobolTexture);

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, RankingTexture);

			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, ScramblingTexture);

			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(0));

			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_1D, Chunk::GetBlockDataTexture());

			glActiveTexture(GL_TEXTURE8);
			glBindTexture(GL_TEXTURE_CUBE_MAP, Sky.SkyCube.Texture[0]);

			glActiveTexture(GL_TEXTURE14);
			glBindTexture(GL_TEXTURE_3D, World.Chunk->ChunkLightTexID);
			
			glActiveTexture(GL_TEXTURE15);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(4));

			glActiveTexture(GL_TEXTURE16);
			glBindTexture(GL_TEXTURE_1D, Chunk::GetTextureExtensionData());

			glActiveTexture(GL_TEXTURE17);
			glBindTexture(GL_TEXTURE_1D, Chunk::GetBlockExtraDataTexture());

			glActiveTexture(GL_TEXTURE10);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Sky.HemiSphericalShadowMap);

			glActiveTexture(GL_TEXTURE23);
			glBindTexture(GL_TEXTURE_CUBE_MAP, Sky.SkyCube.Texture[1]);

			for (int i = 0; i < 48; i++)
			{
				IndirectLightShader.SetUniform("HemisphericalMatrices[" + std::to_string(i) + "]", Sky.HemiProjectionMatrix * Sky.HemiViewMatrices[i]);

			}
			for (int i = 0; i < 48; i++)
			{
				IndirectLightShader.SetUniform("HemisphericalDirections[" + std::to_string(i) + "]", Sky.HemiDirections[i]);

			}

			for (int i = 0; i < 4; i++) {

				IndirectLightShader.SetUniform("DirectionMatrices[" + std::to_string(i) + "]", Sky.ProjectionMatricesRaw[i] * Sky.ViewMatrices[i]); 
				Sky.ShadowMaps[i].BindDepthImage(i + 19); 

			}

			DirectBlockerBuffer.BindImage(24); 

			IndirectLightShader.SetUniform("CameraPosition", Camera.Position);
			IndirectLightShader.SetUniform("CameraMatrix", Camera.Project * Camera.View);

			IndirectLightShader.SetUniform("FrameCount", Window.GetFrameCount() % 1024);
			IndirectLightShader.SetUniform("Time", Window.GetTimeOpened());

			IndirectLightShader.SetUniform("State", Window.GetFrameCount() % 4);

			IndirectLightShader.SetUniform("UseWhiteNoise", sf::Keyboard::isKeyPressed(sf::Keyboard::V));
			IndirectLightShader.SetUniform("DoRayTracing", GetBoolean("raytracing"));
			IndirectLightShader.SetUniform("znear", Camera.znear);
			IndirectLightShader.SetUniform("zfar", Camera.zfar);
			IndirectLightShader.SetUniform("LightDirection", Sky.Orientation);
			IndirectLightShader.SetUniform("SunColor", Sky.SunColor);

			glActiveTexture(GL_TEXTURE25);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(3));

			Deferred.Deferred.BindImage(5, 26);
			Deferred.Deferred.BindImage(6, 27);
			Deferred.Deferred.BindImage(0, 28);

			glActiveTexture(GL_TEXTURE29);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(1));

			ProjectedClouds.BindImage(30); 

			DrawPostProcessQuad();

			IndirectLightShader.UnBind();

			RawPathTrace[Window.GetFrameCount() % 4].UnBind(Window);


		}

		void LightManager::SpatialyFilter(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{

			int Addon = (Window.GetFrameCount() % 4) * 2; 



			PackedSpatialData.Bind();

			SpatialPacker.Bind();

			Deferred.Deferred.BindImage(3, 0);
			Deferred.RawDeferred.BindDepthImage(1);

			SpatialPacker.SetUniform("znear", Camera.znear);
			SpatialPacker.SetUniform("zfar", Camera.zfar);

			DrawPostProcessQuad();

			SpatialPacker.UnBind();

			PackedSpatialData.UnBind();

			SpatialFilter.Bind();

			SpatialFilter.SetUniform("Radius", 1);
			SpatialFilter.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));
			SpatialFilter.SetUniform("SubFrame", Window.GetFrameCount()%4);
			SpatialFilter.SetUniform("DoSpatial", GetBoolean("spatial"));
			SpatialFilter.SetUniform("NewFiltering", GetBoolean("newfiltering"));

			PackedSpatialData.BindImage(0);

			RawPathTrace[Window.GetFrameCount() % 4].BindImage(0, 1); 
			TemporalFrameCount.BindImage(3);
			VolumetricFBO[Window.GetFrameCount() % 4].BindImage(4); 
			RawPathTrace[Window.GetFrameCount() % 4].BindImage(3, 5);
			MotionVectors[Window.GetFrameCount() % 4].BindImage(6);
			RawPathTrace[Window.GetFrameCount() % 4].BindImage(4, 7);


			for (int x = 0; x < 3; x++) {

				SpatialFilter.SetUniform("StepSize", StepSizes[x]);
				SpatialFilter.SetUniform("Final", x == 2);

				SpatialyFiltered[Addon + x % 2].Bind();

				DrawPostProcessQuad();

				SpatialyFiltered[Addon + x % 2].UnBind();

				SpatialyFiltered[Addon + x % 2].BindImage(0, 1);
				SpatialyFiltered[Addon + x % 2].BindImage(1, 4);
				SpatialyFiltered[Addon + x % 2].BindImage(2, 5);


			}


			SpatialFilter.UnBind();

		}

		void LightManager::TemporalyUpscale(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{

			

			TemporalyUpscaled.Bind(); 

			TemporalUpscaler.Bind(); 

			for (int x = 0; x < 4; x++) {
				RawPathTrace[x].BindImage(1, x);
				RawPathTrace[x].BindImage(2, x + 4);
				
				SpatialyFiltered[x * 2].BindImage(0, x + 8); 
				SpatialyFiltered[x * 2].BindImage(1, x + 12);
				SpatialyFiltered[x * 2].BindImage(2, x + 22);
				Clouds[x].BindImage(0, x + 26);

				MotionVectors[x].BindImage(x + 16); 

			}

			Deferred.Deferred.BindImage(1, 20); 
			PackedSpatialData.BindImage(21); 


			//TemporalUpscaler.SetUniform("Upscale", sf::Keyboard::isKeyPressed(sf::Keyboard::U)); 
			TemporalUpscaler.SetUniform("CurrentFrame", Window.GetFrameCount()%4);
			TemporalUpscaler.SetUniform("NewFiltering", GetBoolean("newfiltering"));

			DrawPostProcessQuad(); 

			TemporalUpscaler.UnBind(); 

			TemporalyUpscaled.UnBind(Window); 



		}

		void LightManager::TemporalyFilter(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{

			TemporallyFiltered.Bind();

			TemporalFilter.Bind();


			TemporalyUpscaled.BindImage(0, 0);
			TemporallyFiltered.BindImagePrevious(0, 1);
			TemporalyUpscaled.BindImage(1, 2);
			TemporallyFiltered.BindImagePrevious(1, 3);

			MotionVectors[Window.GetFrameCount() % 4].BindImage(4);
			TemporalFrameCount.BindImage(5);
			Deferred.Deferred.BindImage(3, 6); 

			TemporalyUpscaled.BindImage(2, 7);
			TemporallyFiltered.BindImagePrevious(2, 8);


			TemporalyUpscaled.BindImage(3, 9);
			TemporallyFiltered.BindImagePrevious(3, 10);
			
			TemporalFilter.SetUniform("DoTemporal", GetBoolean("temporal")); 
			TemporalFilter.SetUniform("NewFiltering", GetBoolean("newfiltering"));

			DrawPostProcessQuad();

			TemporalFilter.UnBind();

			TemporallyFiltered.UnBind(Window);


		}

		void LightManager::DoVolumetricLighting(Window& Window, Camera& Camera, DeferredRenderer& Deferred, WorldManager& World, SkyRendering& Sky)
		{

			auto Weather = GetGlobalWeatherManager().GetWeather();

			Volumetrics.Bind(); 

			VolumetricFBO[Window.GetFrameCount() % 4].Bind(); 

			Volumetrics.SetUniform("CameraPosition", Camera.Position); 
			Volumetrics.SetUniform("Frame", Window.GetFrameCount()%512); 
			Volumetrics.SetUniform("Time", Window.GetTimeOpened());
			Volumetrics.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));
			Volumetrics.SetUniform("DoVolumetrics", GetBoolean("volumetrics"));

			Deferred.Deferred.BindImage(1, 0); 
			Deferred.Deferred.BindImage(0, 7);

			WindNoise.Bind(1); 

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_3D, World.Chunk->ChunkLightTexID);

			SimplifiedBlueNoise.Bind(3); 

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Sky.HemiSphericalShadowMap);

			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_CUBE_MAP, Sky.SkyCube.Texture[1]);

			WindNoise.Bind(6); 

			for (int i = 0; i < 48; i++)
			{
				Volumetrics.SetUniform("HemisphericalMatrices[" + std::to_string(i) + "]", Sky.HemiProjectionMatrix * Sky.HemiViewMatrices[i]);

			}
			for (int i = 0; i < 48; i++)
			{
				Volumetrics.SetUniform("HemisphericalDirections[" + std::to_string(i) + "]", Sky.HemiDirections[i]);

			}

			for (int i = 0; i < 4; i++) {

				Volumetrics.SetUniform("DirectionMatrices[" + std::to_string(i) + "]", Sky.ProjectionMatrices[i] * Sky.ViewMatrices[i]);
				Sky.ShadowMaps[i].BindDepthImage(i + 8);

			}

			ProjectedClouds.BindImage(12);
			Clouds[Window.GetFrameCount()%4].BindImage(1,13);


			Volumetrics.SetUniform("LightDirection", Sky.Orientation);
			Volumetrics.SetUniform("SunColor", Sky.SunColor);

			Volumetrics.SetUniform("ScatteringMultiplier", Weather.VolumetricsScatteringMultiplier);
			Volumetrics.SetUniform("AbsorptionMultiplier", Weather.VolumetricsAbsorptionMultiplier);

			DrawPostProcessQuad(); 

			VolumetricFBO[Window.GetFrameCount() % 4].UnBind();

			Volumetrics.UnBind(); 


		}

		void LightManager::GenerateMotionVectors(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{

			MotionVectors[Window.GetFrameCount() % 4].Bind();

			RTMotionVectorCalculator.Bind();

			Deferred.Deferred.BindImage(1, 0);
			Deferred.Deferred.BindImage(0, 1);
			Deferred.Deferred.BindImagePrevious(0, 2);
			Deferred.Deferred.BindImagePrevious(1, 3);
			Deferred.Deferred.BindImage(7, 4);
			Deferred.Deferred.BindImagePrevious(7, 5);
			Clouds[Window.GetFrameCount() % 4].BindImage(1, 6); 

			RTMotionVectorCalculator.SetUniform("CameraPosition", Camera.Position);
			RTMotionVectorCalculator.SetUniform("MotionMatrix", Camera.PrevProject * Camera.PrevView);
			RTMotionVectorCalculator.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));
			RTMotionVectorCalculator.SetUniform("NewFiltering", GetBoolean("newfiltering"));

			DrawPostProcessQuad();

			RTMotionVectorCalculator.UnBind();

			MotionVectors[Window.GetFrameCount() % 4].UnBind();

			TemporalFrameCount.Bind(); 

			FrameCount.Bind(); 

			MotionVectors[Window.GetFrameCount() % 4].BindImage(0);

			TemporalFrameCount.BindImagePrevious(1); 

			DrawPostProcessQuad(); 

			FrameCount.UnBind();

			TemporalFrameCount.UnBind(Window);


		}

		void LightManager::RenderClouds(Window& Window, Camera& Camera, DeferredRenderer& Deferred, SkyRendering& Sky)
		{
		
			auto Weather = GetGlobalWeatherManager().GetWeather();




			ProjectedClouds.Bind(); 

			CloudProjection.Bind(); 

			ProjectedClouds.BindImagePrevious(0); 

			CloudProjection.SetUniform("LightDirection", Sky.Orientation);
			CloudProjection.SetUniform("SunColor", Sky.SunColor);
			CloudProjection.SetUniform("AmbientColor", Sky.SkyColor);

			CloudProjection.SetUniform("Time", Window.GetTimeOpened());
			CloudProjection.SetUniform("SubFrame", Window.GetFrameCount() % 4);
			CloudProjection.SetUniform("Frame", Window.GetFrameCount());

			CloudProjection.SetUniform("ScatteringMultiplier", Weather.CloudScatteringMultiplier); 
			CloudProjection.SetUniform("AbsorptionMultiplier", Weather.CloudAbsorbtionMultiplier);

			CloudProjection.SetUniform("GlobalPower", Weather.CloudGlobalPower); 
			CloudProjection.SetUniform("DetailPower", Weather.CloudDetailPower);
			CloudProjection.SetUniform("NoisePower", Weather.CloudNoisePower);

			SimplifiedBlueNoise.Bind(2);

			CloudNoise.Bind(3);

			WeatherMap.Bind(4);

			DrawPostProcessQuad(); 

			

			CloudProjection.UnBind(); 

			ProjectedClouds.UnBind(Window); 



			Clouds[Window.GetFrameCount() % 4].Bind(); 
			
			CloudRenderer.Bind(); 

			ProjectedClouds.BindImage(0);

			SimplifiedBlueNoise.Bind(2); 

			CloudNoise.Bind(3); 

			WeatherMap.Bind(4); 

			CloudRenderer.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));
			
			CloudRenderer.SetUniform("LightDirection", Sky.Orientation);
			CloudRenderer.SetUniform("SunColor", Sky.SunColor);
			CloudRenderer.SetUniform("AmbientColor", Sky.SkyColor);

			CloudRenderer.SetUniform("Time", Window.GetTimeOpened());
			CloudRenderer.SetUniform("SubFrame", Window.GetFrameCount()%4);
			CloudRenderer.SetUniform("Frame", Window.GetFrameCount());
			CloudRenderer.SetUniform("TextureSize", Window.GetResolution()/2);
			CloudRenderer.SetUniform("CameraPosition", Camera.Position);

			CloudRenderer.SetUniform("ScatteringMultiplier", Weather.CloudScatteringMultiplier);
			CloudRenderer.SetUniform("AbsorptionMultiplier", Weather.CloudAbsorbtionMultiplier);

			CloudRenderer.SetUniform("GlobalPower", Weather.CloudGlobalPower);
			CloudRenderer.SetUniform("DetailPower", Weather.CloudDetailPower);
			CloudRenderer.SetUniform("NoisePower", Weather.CloudNoisePower);

			DrawPostProcessQuad(); 

			CloudRenderer.UnBind();

			Clouds[Window.GetFrameCount() % 4].UnBind();

		}

		void LightManager::ReloadIndirect(Window& Window)
		{
			IndirectLightShader.Reload("Shaders/RawPathTracing");
			TemporalUpscaler.Reload("Shaders/TemporalUpscaler");
			SpatialFilter.Reload("Shaders/SpatialFilter");
			SpatialUpscaler.Reload("Shaders/SpatialUpscaler");
			TemporalFilter.Reload("Shaders/TemporalFilter");
			RTMotionVectorCalculator.Reload("Shaders/RTMotionVectors");
			SpatialPacker.Reload("Shaders/SpatialPacker");
			FrameCount.Reload("Shaders/TemporalFrameCounter");
			Volumetrics.Reload("Shaders/Volumetrics"); 
			DirectBlocker.Reload("Shaders/DirectBlocker"); 
			CloudRenderer.Reload("Shaders/Clouds"); 
			CloudProjection.Reload("Shaders/CloudProjection"); 

			SetShaderUniforms(Window); 

		}

		void LightManager::SetShaderUniforms(Window& Window)
		{
			IndirectLightShader.Bind();

			IndirectLightShader.SetUniform("Normals", 0);
			IndirectLightShader.SetUniform("WorldPosition", 1);
			IndirectLightShader.SetUniform("Voxels", 2);
			IndirectLightShader.SetUniform("Sobol", 3);
			IndirectLightShader.SetUniform("Ranking", 4);
			IndirectLightShader.SetUniform("Scrambling", 5);
			IndirectLightShader.SetUniform("DiffuseTextures", 6);
			IndirectLightShader.SetUniform("TextureData", 7);
			IndirectLightShader.SetUniform("Sky", 8);
			IndirectLightShader.SetUniform("LightingData", 14);
			IndirectLightShader.SetUniform("EmissiveTextures", 15);
			IndirectLightShader.SetUniform("TextureExData", 16);
			IndirectLightShader.SetUniform("BlockData", 17);
			IndirectLightShader.SetUniform("HemisphericalShadowMap", 10);
			IndirectLightShader.SetUniform("Depth", 18);
			IndirectLightShader.SetUniform("DirectionalCascades[0]", 19);
			IndirectLightShader.SetUniform("DirectionalCascades[1]", 20);
			IndirectLightShader.SetUniform("DirectionalCascades[2]", 21);
			IndirectLightShader.SetUniform("DirectionalCascades[3]", 22);
			IndirectLightShader.SetUniform("SkyNoMie", 23);
			IndirectLightShader.SetUniform("BlockerData", 24);
			IndirectLightShader.SetUniform("DisplacementTextures", 25);
			IndirectLightShader.SetUniform("ParallaxDirections", BAKE_DIRECTIONS);
			IndirectLightShader.SetUniform("ParallaxResolution", BAKE_RESOLUTION);
			IndirectLightShader.SetUniform("ParallaxData", 26);
			IndirectLightShader.SetUniform("TCData", 27);
			IndirectLightShader.SetUniform("LowFrequencyNormal", 28);
			IndirectLightShader.SetUniform("NormalTextures", 29);
			IndirectLightShader.SetUniform("ProjectedClouds", 30);

			IndirectLightShader.UnBind();

			TemporalUpscaler.Bind();

			TemporalUpscaler.SetUniform("FramesNormal[0]", 0);
			TemporalUpscaler.SetUniform("FramesNormal[1]", 1);
			TemporalUpscaler.SetUniform("FramesNormal[2]", 2);
			TemporalUpscaler.SetUniform("FramesNormal[3]", 3);

			TemporalUpscaler.SetUniform("FramesWorldPos[0]", 4);
			TemporalUpscaler.SetUniform("FramesWorldPos[1]", 5);
			TemporalUpscaler.SetUniform("FramesWorldPos[2]", 6);
			TemporalUpscaler.SetUniform("FramesWorldPos[3]", 7);

			TemporalUpscaler.SetUniform("FramesIndirectDiffuse[0]", 8);
			TemporalUpscaler.SetUniform("FramesIndirectDiffuse[1]", 9);
			TemporalUpscaler.SetUniform("FramesIndirectDiffuse[2]", 10);
			TemporalUpscaler.SetUniform("FramesIndirectDiffuse[3]", 11);

			TemporalUpscaler.SetUniform("FramesVolumetric[0]", 12);
			TemporalUpscaler.SetUniform("FramesVolumetric[1]", 13);
			TemporalUpscaler.SetUniform("FramesVolumetric[2]", 14);
			TemporalUpscaler.SetUniform("FramesVolumetric[3]", 15);

			TemporalUpscaler.SetUniform("MotionVectors[0]", 16);
			TemporalUpscaler.SetUniform("MotionVectors[1]", 17);
			TemporalUpscaler.SetUniform("MotionVectors[2]", 18);
			TemporalUpscaler.SetUniform("MotionVectors[3]", 19);

			TemporalUpscaler.SetUniform("WorldPos", 20);
			TemporalUpscaler.SetUniform("Normal", 21);

			TemporalUpscaler.SetUniform("FramesIndirectSpecular[0]", 22);
			TemporalUpscaler.SetUniform("FramesIndirectSpecular[1]", 23);
			TemporalUpscaler.SetUniform("FramesIndirectSpecular[2]", 24);
			TemporalUpscaler.SetUniform("FramesIndirectSpecular[3]", 25);

			TemporalUpscaler.SetUniform("FramesClouds[0]", 26);
			TemporalUpscaler.SetUniform("FramesClouds[1]", 27);
			TemporalUpscaler.SetUniform("FramesClouds[2]", 28);
			TemporalUpscaler.SetUniform("FramesClouds[3]", 29);

			TemporalUpscaler.SetUniform("Resolution", Window.GetResolution() / 4);

			TemporalUpscaler.UnBind();

			RTMotionVectorCalculator.Bind();

			RTMotionVectorCalculator.SetUniform("WorldPosPrevious", 0);
			RTMotionVectorCalculator.SetUniform("Normal", 1);
			RTMotionVectorCalculator.SetUniform("NormalPrevious", 2);
			RTMotionVectorCalculator.SetUniform("PreviousWorldPos", 3);
			RTMotionVectorCalculator.SetUniform("CurrentLighting", 4);
			RTMotionVectorCalculator.SetUniform("PreviousLighting", 5);
			RTMotionVectorCalculator.SetUniform("CloudDepth", 6);

			RTMotionVectorCalculator.SetUniform("Resolution", Vector2f(Window.GetResolution() / 2));

			RTMotionVectorCalculator.UnBind();

			SpatialPacker.Bind();

			SpatialPacker.SetUniform("InputNormal", 0);
			SpatialPacker.SetUniform("InputDepth", 1);

			SpatialPacker.UnBind();

			SpatialFilter.Bind();

			SpatialFilter.SetUniform("InputPacked", 0);
			SpatialFilter.SetUniform("InputLighting", 1);
			SpatialFilter.SetUniform("InputSH", 2);
			SpatialFilter.SetUniform("FrameCount", 3);
			SpatialFilter.SetUniform("InputVolumetric", 4);
			SpatialFilter.SetUniform("InputSpecular", 5);
			SpatialFilter.SetUniform("MotionVectors", 6);
			SpatialFilter.SetUniform("Direct", 7);


			SpatialFilter.UnBind();

			SpatialUpscaler.Bind();

			SpatialUpscaler.SetUniform("Depth", 0);
			SpatialUpscaler.SetUniform("Normal", 1);
			SpatialUpscaler.SetUniform("Lighting", 2);
			SpatialUpscaler.SetUniform("PackedGeometryData", 3);
			SpatialUpscaler.SetUniform("Specular", 5);
			SpatialUpscaler.SetUniform("Direct", 6);

			SpatialUpscaler.UnBind();

			TemporalFilter.Bind();

			TemporalFilter.SetUniform("UpscaledDiffuse", 0);
			TemporalFilter.SetUniform("PreviousDiffuse", 1);
			TemporalFilter.SetUniform("UpscaledVolumetrics", 2);
			TemporalFilter.SetUniform("PreviousVolumetrics", 3);
			TemporalFilter.SetUniform("MotionVectors", 4);
			TemporalFilter.SetUniform("FrameCount", 5);
			TemporalFilter.SetUniform("NormalRoughness", 6);

			TemporalFilter.SetUniform("UpscaledSpecular", 7);
			TemporalFilter.SetUniform("PreviousSpecular", 8);

			TemporalFilter.SetUniform("UpscaledClouds", 9);
			TemporalFilter.SetUniform("PreviousClouds", 10);

			TemporalFilter.UnBind();

			

			FrameCount.Bind();

			FrameCount.SetUniform("MotionVectors", 0); 
			FrameCount.SetUniform("PreviousFrame", 1); 
			

			FrameCount.UnBind();


			Volumetrics.Bind(); 

			Volumetrics.SetUniform("WorldPosition", 0); 
			Volumetrics.SetUniform("WindNoise", 1);
			Volumetrics.SetUniform("ChunkLighting", 2);
			Volumetrics.SetUniform("BasicBlueNoise", 3);
			Volumetrics.SetUniform("HemisphericalShadowMap", 4);
			Volumetrics.SetUniform("Sky", 5);
			Volumetrics.SetUniform("Wind", 6);
			Volumetrics.SetUniform("Normal", 7);
			Volumetrics.SetUniform("DirectionalCascades[0]", 8);
			Volumetrics.SetUniform("DirectionalCascades[1]", 9);
			Volumetrics.SetUniform("DirectionalCascades[2]", 10);
			Volumetrics.SetUniform("DirectionalCascades[3]", 11);
			Volumetrics.SetUniform("ProjectedClouds", 12); 
			Volumetrics.SetUniform("CloudDepth", 13);

			Volumetrics.UnBind(); 

			DirectBlocker.Bind(); 

			DirectBlocker.SetUniform("DirectionalCascades[0]", 19); 
			DirectBlocker.SetUniform("DirectionalCascades[1]", 20);
			DirectBlocker.SetUniform("DirectionalCascades[2]", 21);
			DirectBlocker.SetUniform("DirectionalCascades[3]", 22);
			DirectBlocker.SetUniform("WorldPosition", 4);
			DirectBlocker.SetUniform("Depth", 5);

			DirectBlocker.UnBind(); 
			
			CloudRenderer.Bind(); 

			CloudRenderer.SetUniform("ProjectedClouds", 0); 
			CloudRenderer.SetUniform("BasicBlueNoise", 2);
			CloudRenderer.SetUniform("CloudNoise", 3);
			CloudRenderer.SetUniform("WeatherMap", 4);

			CloudRenderer.SetUniform("TextureSize", Window.GetResolution() / 2);

			CloudRenderer.UnBind();

			CloudProjection.Bind();

			CloudProjection.SetUniform("PreviousCloudResult", 0);
			CloudProjection.SetUniform("BasicBlueNoise", 2);
			CloudProjection.SetUniform("CloudNoise", 3);
			CloudProjection.SetUniform("WeatherMap", 4);

			CloudProjection.UnBind();


		}

		void LightManager::SpatialyUpscale(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{

			SpatialyUpscaled.Bind(); 

			SpatialUpscaler.Bind(); 

			SpatialUpscaler.SetUniform("znear", Camera.znear); 
			SpatialUpscaler.SetUniform("zfar", Camera.zfar);
			SpatialUpscaler.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));
			SpatialUpscaler.SetUniform("SpatialUpscaling", GetBoolean("spatialupscale"));

			Deferred.Deferred.BindDepthImage(0); 
			Deferred.Deferred.BindImage(3, 1);
			TemporallyFiltered.BindImage(0, 2); 
			PackedSpatialData.BindImage(3);
			TemporallyFiltered.BindImage(1, 4);
			TemporallyFiltered.BindImage(2, 5);
			TemporalyUpscaled.BindImage(2, 6);


			DrawPostProcessQuad(); 

			SpatialUpscaler.UnBind();

			SpatialyUpscaled.UnBind();

		}

	}
}