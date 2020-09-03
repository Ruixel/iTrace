#include "Indirect.h"
#include "External/BlueNoiseData.h"
#include "BooleanCommands.h"
#include "ParallaxBaker.h"
#include "Weather.h"

namespace iTrace {

	namespace Rendering {

//#define CHECKERBOARDINJECTCODE "#define CheckerBoarding"
//#define DO_CHECKERBOARD

		void LightManager::PrepareIndirectLightingHandler(Window& Window)
		{
			

			std::cout << "Injection code: " << Chunk::GetInjectionCode() << '\n'; 

			
			RequestBoolean("raytracing", true); 
			RequestBoolean("volumetrics", false);
			RequestBoolean("spatial", true);
			RequestBoolean("temporal", true);

			Vector2i CheckerBoardedResolution = Vector2i(Window.GetResolution()) / Vector2i(8, 4); 
			
			auto GenerateTexture = [](unsigned int & Texture, Vector2i Resolution, int InternalFormat, int Format) {
				glGenTextures(1, &Texture); 

				glBindTexture(GL_TEXTURE_2D, Texture); 
				glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Resolution.x, Resolution.y, 0, Format, GL_FLOAT, nullptr); 
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glBindTexture(GL_TEXTURE_2D, 0);



			}; 

			GenerateTexture(SpecularDetailTexture, CheckerBoardedResolution, GL_RGBA16F, GL_RGBA); 
			GenerateTexture(SpecularDirectionTexture, CheckerBoardedResolution, GL_RGBA16F, GL_RGBA);


			for (int x = 0; x < 4; x++) {
				RawPathTrace[x] = MultiPassFrameBufferObject(CheckerBoardedResolution, 8, { GL_RGBA16F, GL_RGBA16F, GL_RGB32F,GL_RGBA16F, GL_RGB16F,GL_RGBA16F, GL_R16F,GL_RGBA16F,GL_RGBA16F,GL_RGBA16F }, false);
				MotionVectors[x] = MultiPassFrameBufferObject(Window.GetResolution() / 2, 3, { GL_RGB16F,GL_RGB16F, GL_RG16F }, false);
				SpatialyFiltered[x] = MultiPassFrameBufferObject(CheckerBoardedResolution, 3, { GL_RGBA16F,GL_RGBA16F,GL_RGBA16F }, false);
				VolumetricFBO[x] = FrameBufferObject(CheckerBoardedResolution, GL_RGBA16F, false);
				Checkerboarder[x] = MultiPassFrameBufferObject(Window.GetResolution() / 4, 7, { GL_RGBA16F, GL_RGBA16F,GL_RGBA16F,GL_RGBA16F,GL_RGBA16F,GL_R32F,GL_RGBA16F }, false);

			}

			for (int i = 0; i < 2; i++) {
				SpatialyFilteredTemporary[i] = MultiPassFrameBufferObject(CheckerBoardedResolution, 3, { GL_RGBA16F, GL_RGBA16F, GL_R16F });
				ErrorMaskBlur[i] = FrameBufferObject(Window.GetResolution() / 4, GL_R8, false); 
			}

			Clouds = MultiPassFrameBufferObject(Window.GetResolution() / 8, 2, { GL_RGBA16F, GL_R32F }, false);
			RefractedWater = MultiPassFrameBufferObject(Window.GetResolution() / 4, 4, { GL_RG32F, GL_RGBA16F, GL_RG32F, GL_R8 }, false);
			DirectBlockerBuffer = FrameBufferObject(Window.GetResolution() / 8, GL_RG16F, false);
			TemporalFrameCount = MultiPassFrameBufferObjectPreviousData(Window.GetResolution() / 2, 2, { GL_R16F,GL_R16F }, false);
			TemporalyUpscaled = MultiPassFrameBufferObject(Window.GetResolution() / 2, 5, { GL_RGBA16F,GL_RGBA16F,GL_RGBA16F,GL_RGBA16F,GL_RGBA16F }, false);
			PackedSpatialData = MultiPassFrameBufferObjectPreviousData(Window.GetResolution() / 2, 3, { GL_RGBA16F, GL_RGBA16F, GL_RGB32F }, false);
			TemporallyFiltered = MultiPassFrameBufferObjectPreviousData(Window.GetResolution() / 2, 5, { GL_RGBA16F,GL_RGBA16F,GL_RGBA16F,GL_RGBA16F,GL_RGBA16F }, false);
			SpatialyUpscaled = MultiPassFrameBufferObject(Window.GetResolution(), 3, { GL_RGBA16F,GL_RGBA16F, GL_RGBA16F }, false);
			ProjectedClouds = FrameBufferObjectPreviousData(Vector2i(256), GL_RGBA16F, false); 
			PreSpatialTemporal = FrameBufferObjectPreviousData(Window.GetResolution() / 4, GL_RGBA16F, false);
			PackedWaterData = FrameBufferObject(Window.GetResolution() / 2, GL_R16F, false); 
			PreUpscaled = MultiPassFrameBufferObjectPreviousData(Window.GetResolution() / 2, 4, { GL_RGBA16F, GL_RGBA16F, GL_RGBA16F, GL_R16F }, false); 
			CheckerUpscaledDetail = FrameBufferObject(Window.GetResolution() / 4, GL_RGBA16F, false);
			SpecularMotionVectorData = FrameBufferObjectPreviousData(Window.GetResolution() / 4, GL_RGBA16F, false);

			IndirectLightShader = Shader("Shaders/RawPathTracing", false, Chunk::GetInjectionCode());
			TemporalUpscaler = Shader("Shaders/TemporalUpscaler");
			SpatialFilter = Shader("Shaders/SpatialFilter"); 
			SpatialFilterFinal = Shader("Shaders/SpatialFilter", false, "#define SPATIAL_FINAL"); 
			SpatialUpscaler = Shader("Shaders/SpatialUpscaler"); 
			TemporalFilter = Shader("Shaders/TemporalFilter"); 
			RTMotionVectorCalculator = Shader("Shaders/RTMotionVectors"); 
			SpatialPacker = Shader("Shaders/SpatialPacker"); 
			FrameCount = Shader("Shaders/TemporalFrameCounter");
			Volumetrics = Shader("Shaders/Volumetrics"); 
			DirectBlocker = Shader("Shaders/DirectBlocker"); 
			CloudRenderer = Shader("Shaders/Clouds"); 
			CloudProjection = Shader("Shaders/CloudProjection"); 
			CheckerboardUpscaler = Shader("Shaders/CheckerBoardHandler"); 
			PreSpatialTemporalFilter = Shader("Shaders/PreSpatialTemporal"); 
			WaterRefraction = Shader("Shaders/WaterRefraction"); 
			WaterDepthPacker = Shader("Shaders/WaterDepthPacker"); 
			WaterErrorBlur = Shader("Shaders/ErrorMaskBlur"); 
			PreUpscaler = Shader("Shaders/PreUpscaler"); 
			CheckerUpscaler = Shader("Shaders/CheckerUpscaler"); 
			SpecularMotionVectorResolver = Shader("Shaders/SpecularMotionVectorResolver"); 

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
			CloudShape = LoadTextureGL3D("Textures/CloudsShape.png", Vector3i(32)); 
			Turbulence = LoadTextureGL("Textures/Turbulence.png"); 
			Cirrus = LoadTextureGL("Textures/Cirrus.png");

		}

		void LightManager::RenderIndirectLighting(Window& Window, Camera& Camera, DeferredRenderer& Deferred, WorldManager& World, SkyRendering& Sky)
		{


			Profiler::FlushTime();
			PackSpatialData(Window, Camera, Deferred);
			Profiler::SetPerformance("Pack spatial data");
			
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

			DoPreUpscaling(Window, Camera, Deferred); 
			Profiler::SetPerformance("Pre upscaling"); 

			DoVolumetricLighting(Window, Camera, Deferred, World, Sky);
			Profiler::SetPerformance("Volumetric lighting");

			SpatialyFilter(Window, Camera, Deferred);
			Profiler::SetPerformance("Spatial filter");

			CheckerboardUpscale(Window, Camera, Deferred); 
			Profiler::SetPerformance("Checkerboarding"); 

			TemporalyUpscale(Window, Camera, Deferred);
			Profiler::SetPerformance("Temporal upscale");

			TemporalyFilter(Window, Camera, Deferred);
			Profiler::SetPerformance("Temporal filtering");

			SpatialyUpscale(Window, Camera, Deferred); 
			Profiler::SetPerformance("Spatial upscaling");

			DoWaterRayTrace(Window, Camera, Deferred); 
			Profiler::SetPerformance("Water refraction"); 

		}

		void LightManager::PackSpatialData(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{
			PackedSpatialData.Bind();

			SpatialPacker.Bind();

			Deferred.Deferred.BindImage(0, 0);
			Deferred.RawDeferred.BindDepthImage(1);
			Deferred.Deferred.BindImage(3, 2);
			Deferred.RawWaterDeferred.BindDepthImage(3);
			Deferred.RawWaterDeferred.BindImage(2, 4);
			Deferred.RawWaterDeferred.BindImage(1, 5);
			Deferred.Deferred.BindImage(1, 6);


			SpatialPacker.SetUniform("znear", Camera.znear);
			SpatialPacker.SetUniform("zfar", Camera.zfar);

			DrawPostProcessQuad();

			SpatialPacker.UnBind();

			PackedSpatialData.UnBind();
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

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_3D, World.ChunkContainer);

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
			glBindTexture(GL_TEXTURE_3D, World.LightContainer);
			
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
			IndirectLightShader.SetUniform("SkyColor", Sky.SkyColor);

			IndirectLightShader.SetUniform("CheckerStep", (Window.GetFrameCount() / 4) % 2);

			IndirectLightShader.SetUniform("PositionBias", Vector2i(World.BiasX, World.BiasY));

			glActiveTexture(GL_TEXTURE25);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(3));

			Deferred.Deferred.BindImage(5, 26);
			Deferred.Deferred.BindImage(6, 39);
			Deferred.Deferred.BindImage(0, 40);

			glActiveTexture(GL_TEXTURE29);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(1));

			ProjectedClouds.BindImage(30); 

			for (int i = 0; i < 4; i++) {
				Sky.RefractiveShadowMaps[i].BindImage(i + 31);
			}

			Deferred.Deferred.BindImage(2, 35);

			Deferred.RawWaterDeferred.BindImage(1, 36); 
			Deferred.RawWaterDeferred.BindImage(2, 37);
			Deferred.RawWaterDeferred.BindDepthImage(38);

			glBindImageTexture(0, SpecularDetailTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			glBindImageTexture(1, SpecularDirectionTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

			DrawPostProcessQuad();

			IndirectLightShader.UnBind();

			RawPathTrace[Window.GetFrameCount() % 4].UnBind(Window);


		}

		void LightManager::SpatialyFilter(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{

			
			


			SpatialFilter.Bind();

			SpatialFilter.SetUniform("Radius", 1);
			SpatialFilter.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));
			SpatialFilter.SetUniform("SubFrame", Window.GetFrameCount()%4);
			SpatialFilter.SetUniform("DoSpatial", GetBoolean("spatial"));
			SpatialFilter.SetUniform("NewFiltering", GetBoolean("newfiltering"));
			SpatialFilter.SetUniform("CheckerStep", (Window.GetFrameCount() / 4) % 2);

			PackedSpatialData.BindImage(0,0);
			RawPathTrace[Window.GetFrameCount() % 4].BindImage(7, 0);

			RawPathTrace[Window.GetFrameCount() % 4].BindImage(0, 1); 
			TemporalFrameCount.BindImage(0,2);
			RawPathTrace[Window.GetFrameCount() % 4].BindImage(3, 3);
			MotionVectors[Window.GetFrameCount() % 4].BindImage(0,4);
			RawPathTrace[Window.GetFrameCount() % 4].BindImage(4, 5);
			PreUpscaled.BindImage(2, 6); 
			RawPathTrace[Window.GetFrameCount() % 4].BindImage(6, 7); 
			Deferred.RawDeferred.BindDepthImage(8);
			Deferred.RawWaterDeferred.BindDepthImage(9);


			for (int x = 0; x < 2; x++) {
				SpatialFilter.SetUniform("StepSize", StepSizes[x]);
				SpatialFilter.SetUniform("Final", x == 2);

				SpatialyFilteredTemporary[x].Bind();

				DrawPostProcessQuad();

				SpatialyFilteredTemporary[x].UnBind();

				SpatialyFilteredTemporary[x].BindImage(0, 1);
				SpatialyFilteredTemporary[x].BindImage(1, 3);
				SpatialyFilteredTemporary[x].BindImage(2, 7);
			}


			SpatialFilter.UnBind();

			SpatialFilterFinal.Bind();


			SpatialFilterFinal.SetUniform("StepSize", StepSizes[2]);
			SpatialFilterFinal.SetUniform("Radius", 1);
			SpatialFilterFinal.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));
			SpatialFilterFinal.SetUniform("SubFrame", Window.GetFrameCount() % 4);
			SpatialFilterFinal.SetUniform("DoSpatial", GetBoolean("spatial"));
			SpatialFilterFinal.SetUniform("NewFiltering", GetBoolean("newfiltering"));
			SpatialFilterFinal.SetUniform("CheckerStep", (Window.GetFrameCount() / 4) % 2);

			SpatialyFiltered[Window.GetFrameCount() % 4].Bind();

			DrawPostProcessQuad(); 

			SpatialyFiltered[Window.GetFrameCount() % 4].UnBind();

			SpatialFilterFinal.UnBind();

		}

		void LightManager::TemporalyUpscale(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{

			

			TemporalyUpscaled.Bind(); 

			TemporalUpscaler.Bind(); 

			for (int x = 0; x < 4; x++) {
				RawPathTrace[x].BindImage(1, x);
				Checkerboarder[x].BindImage(6, x); 
				RawPathTrace[x].BindImage(2, x + 4);
				
				SpatialyFiltered[x].BindImage(0, x + 8); 
				Checkerboarder[x].BindImage(2, x + 8); 
				VolumetricFBO[x].BindImage(x + 12); 
				Checkerboarder[x].BindImage(1, x + 12);

				SpatialyFiltered[x].BindImage(1, x + 22);

				Checkerboarder[x].BindImage(3, x + 22);

				Clouds.BindImage(0, x + 26);
				Checkerboarder[x].BindImage(0, x + 26); 
				SpatialyFiltered[x].BindImage(2, x + 30); 
				Checkerboarder[x].BindImage(4, x + 30);

				MotionVectors[x].BindImage(0,x + 16); 
				MotionVectors[x].BindImage(1,x + 34);
			}

			Deferred.Deferred.BindImage(1, 20); 
			PackedSpatialData.BindImage(0,21); 


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

			MotionVectors[Window.GetFrameCount() % 4].BindImage(0,4);
			TemporalFrameCount.BindImage(0,5);
			Deferred.Deferred.BindImage(3, 6); 

			TemporalyUpscaled.BindImage(2, 7);
			TemporallyFiltered.BindImagePrevious(2, 8);


			TemporalyUpscaled.BindImage(3, 9);
			TemporallyFiltered.BindImagePrevious(3, 10);

			TemporalyUpscaled.BindImage(4, 11);
			TemporallyFiltered.BindImagePrevious(4, 12);

			Deferred.RawDeferred.BindDepthImage(13); 
			Deferred.RawWaterDeferred.BindDepthImage(14); 

			MotionVectors[Window.GetFrameCount() % 4].BindImage(1, 15);
			TemporalFrameCount.BindImage(1, 16);

			
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
			Volumetrics.SetUniform("CheckerStep", (Window.GetFrameCount() / 4) % 2);

			Deferred.Deferred.BindImage(1, 0); 
			Deferred.Deferred.BindImage(0, 7);

			WindNoise.Bind(1); 

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_3D, World.LightContainer);

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
				Sky.RefractiveShadowMaps[i].BindImage(i + 14);
				Sky.RefractiveShadowMapsDepth[i].BindDepthImage(i + 18);

			}

			ProjectedClouds.BindImage(12);
			Clouds.BindImage(1,13);
			Deferred.PrimaryDeferredRefractive.BindDepthImage(22); 

			Volumetrics.SetUniform("LightDirection", Sky.Orientation);
			Volumetrics.SetUniform("SunColor", Sky.SunColor);

			Volumetrics.SetUniform("ScatteringMultiplier", Weather.VolumetricsScatteringMultiplier);
			Volumetrics.SetUniform("AbsorptionMultiplier", Weather.VolumetricsAbsorptionMultiplier);

			Volumetrics.SetUniform("zNear", Camera.znear); 
			Volumetrics.SetUniform("zFar", Camera.zfar);

			Volumetrics.SetUniform("PositionBias", Vector2i(World.BiasX, World.BiasY));


			DrawPostProcessQuad(); 

			VolumetricFBO[Window.GetFrameCount() % 4].UnBind();

			Volumetrics.UnBind(); 


		}

		Vector3f Cam = Vector3f(-1.0); 

		void LightManager::GenerateMotionVectors(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{
			
			SpecularMotionVectorData.Bind(); 

			SpecularMotionVectorResolver.Bind(); 

			SpecularMotionVectorResolver.SetUniform("CheckerStep", (Window.GetFrameCount() / 4) % 2);

			glActiveTexture(GL_TEXTURE0); 
			glBindTexture(GL_TEXTURE_2D, SpecularDirectionTexture);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, SpecularDetailTexture);

			DrawPostProcessQuad();

			SpecularMotionVectorResolver.UnBind(); 

			SpecularMotionVectorData.UnBind(); 
			

			MotionVectors[Window.GetFrameCount() % 4].Bind();

			RTMotionVectorCalculator.Bind();

			Deferred.Deferred.BindImage(1, 0);
			Deferred.Deferred.BindImage(0, 1);
			Deferred.Deferred.BindImagePrevious(0, 2);
			Deferred.Deferred.BindImagePrevious(1, 3);
			Deferred.Deferred.BindImage(7, 4);
			Deferred.Deferred.BindImagePrevious(7, 5);
			Clouds.BindImage(1, 6); 
			Checkerboarder[Window.GetFrameCount() % 4].BindImage(5, 6); 

			Deferred.RawWaterDeferred.BindImage(1, 7); 
			Deferred.RawWaterDeferred.BindImage(1, 8);
			Deferred.RawWaterDeferred.BindImage(2, 9);
			Deferred.RawWaterDeferred.BindImage(2, 10);
			Deferred.RawDeferred.BindDepthImage(11); 
			Deferred.RawWaterDeferred.BindDepthImage(12);

			SpecularMotionVectorData.BindImage(13); 
			PackedSpatialData.BindImage(2, 14); 

			SpecularMotionVectorData.BindImagePrevious(15);
			PackedSpatialData.BindImagePrevious(2, 16);
			

			RTMotionVectorCalculator.SetUniform("CameraPosition", Camera.Position);
			RTMotionVectorCalculator.SetUniform("PreviousCameraPosition", Cam);

			RTMotionVectorCalculator.SetUniform("MotionMatrix", Camera.PrevProject * Camera.PrevView);
			RTMotionVectorCalculator.SetUniform("ProjectionMatrix", Camera.PrevProject);
			RTMotionVectorCalculator.SetUniform("ViewMatrix", Camera.PrevView);

			RTMotionVectorCalculator.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));
			RTMotionVectorCalculator.SetUniform("NewFiltering", GetBoolean("newfiltering"));

			DrawPostProcessQuad();

			RTMotionVectorCalculator.UnBind();

			MotionVectors[Window.GetFrameCount() % 4].UnBind();

			TemporalFrameCount.Bind(); 

			FrameCount.Bind(); 

			MotionVectors[Window.GetFrameCount() % 4].BindImage(0,0);

			TemporalFrameCount.BindImagePrevious(0,1); 
			TemporalFrameCount.BindImagePrevious(1, 2);
			MotionVectors[Window.GetFrameCount() % 4].BindImage(1, 3);


			DrawPostProcessQuad(); 

			FrameCount.UnBind();

			TemporalFrameCount.UnBind(Window);

			Cam = Camera.Position; 

		}

		void LightManager::CheckerboardUpscale(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{

			Checkerboarder[Window.GetFrameCount()%4].Bind(); 

			CheckerboardUpscaler.Bind(); 

			CheckerboardUpscaler.SetUniform("CheckerStep", (Window.GetFrameCount()/4)%2);
			CheckerboardUpscaler.SetUniform("State", Window.GetFrameCount() % 4); 
			CheckerboardUpscaler.SetUniform("znear", Camera.znear);
			CheckerboardUpscaler.SetUniform("zfar", Camera.zfar);

			Clouds.BindImage(0, 0);
			VolumetricFBO[Window.GetFrameCount() % 4].BindImage(1); 
			SpatialyFiltered[Window.GetFrameCount() % 4].BindImage(0,2); 
			SpatialyFiltered[Window.GetFrameCount() % 4].BindImage(1,3);
			SpatialyFiltered[Window.GetFrameCount() % 4].BindImage(2,4);
			//RawPathTrace[Window.GetFrameCount() % 4].BindImage(4, 4); 
			Clouds.BindImage(1, 5);
			PackedSpatialData.BindImage(0,6);
			RawPathTrace[Window.GetFrameCount() % 4].BindImage(7, 6); 
			Deferred.RawDeferred.BindDepthImage(7); 
			Deferred.Deferred.BindImage(0, 8); 
			PackedSpatialData.BindImage(1, 9);


			DrawPostProcessQuad(); 

			CheckerboardUpscaler.UnBind();

			Checkerboarder[Window.GetFrameCount() % 4].UnBind(); 
		}

		void LightManager::DoWaterRayTrace(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{

			PackedWaterData.Bind(); 

			WaterDepthPacker.Bind(); 

			WaterDepthPacker.SetUniform("znear", Camera.znear); 
			WaterDepthPacker.SetUniform("zfar", Camera.zfar);

			Deferred.RawDeferred.BindDepthImage(0); 
			Deferred.PrimaryDeferredRefractive.BindDepthImage(1); 

			DrawPostProcessQuad(); 

			WaterDepthPacker.Bind();

			PackedWaterData.UnBind(); 
			
			RefractedWater.Bind(); 

			WaterRefraction.Bind(); 

			Deferred.RawWaterDeferred.BindImage(0, 0); 
			Deferred.RawWaterDeferred.BindImage(1, 1);
			Deferred.RawWaterDeferred.BindDepthImage(2);
			PackedWaterData.BindImage(3); 

			WaterRefraction.SetUniform("ViewMatrix", Camera.View); 
			WaterRefraction.SetUniform("ProjectionMatrix", Camera.Project);
			WaterRefraction.SetUniform("CameraPosition", Camera.Position);

			WaterRefraction.SetUniform("znear", Camera.znear);
			WaterRefraction.SetUniform("zfar", Camera.zfar);

			DrawPostProcessQuad(); 

			WaterRefraction.UnBind();

			RefractedWater.UnBind(); 


			RefractedWater.BindImage(3, 0); 

			WaterErrorBlur.Bind(); 

			WaterErrorBlur.SetUniform("Resolution", Window.GetResolution() / 4); 

			for (int i = 0; i < 2; i++) {
			
				ErrorMaskBlur[i].Bind(); 

				WaterErrorBlur.SetUniform("Vertical", i == 0); 

				DrawPostProcessQuad(); 

				ErrorMaskBlur[i].UnBind();

				ErrorMaskBlur[i].BindImage(0); 
			
			}

			WaterErrorBlur.UnBind();


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
			CloudProjection.SetUniform("SkyColor", Sky.SkyColor);

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

			CloudShape.Bind(5);

			Turbulence.Bind(6);

			DrawPostProcessQuad(); 

			

			CloudProjection.UnBind(); 

			ProjectedClouds.UnBind(Window); 

			Clouds.Bind(); 
			
			CloudRenderer.Bind(); 

			ProjectedClouds.BindImage(0);

			SimplifiedBlueNoise.Bind(2); 

			CloudNoise.Bind(3); 

			WeatherMap.Bind(4); 

			CloudShape.Bind(5);

			Turbulence.Bind(6); 

			Cirrus.Bind(7);

			CloudRenderer.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));
			
			CloudRenderer.SetUniform("LightDirection", Sky.Orientation);
			CloudRenderer.SetUniform("SunColor", Sky.SunColor);
			CloudRenderer.SetUniform("SkyColor", Sky.SkyColor);

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
			CloudRenderer.SetUniform("CheckerStep", (Window.GetFrameCount() / 4) % 2);

			DrawPostProcessQuad(); 

			CloudRenderer.UnBind();

			Clouds.UnBind();

		}

		void LightManager::DoPreUpscaling(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{
			

			CheckerUpscaler.Bind();

			CheckerUpscaledDetail.Bind();

			CheckerUpscaler.SetUniform("CheckerStep", (Window.GetFrameCount() / 4) % 2);
			CheckerUpscaler.SetUniform("State", Window.GetFrameCount() % 4);
			CheckerUpscaler.SetUniform("znear", Camera.znear);
			CheckerUpscaler.SetUniform("zfar", Camera.zfar);

			RawPathTrace[Window.GetFrameCount() % 4].BindImage(5, 0);
			RawPathTrace[Window.GetFrameCount() % 4].BindImage(7, 1);
			Deferred.RawDeferred.BindDepthImage(2);
			Deferred.Deferred.BindImage(0, 3);

			DrawPostProcessQuad();

			CheckerUpscaledDetail.UnBind();

			CheckerUpscaler.UnBind();


			int Addon = (Window.GetFrameCount() % 4) * 2;

			PreSpatialTemporalFilter.Bind();

			PreSpatialTemporal.Bind();

			CheckerUpscaledDetail.BindImage(0); 

			PreSpatialTemporal.BindImagePrevious(1);

			PackedSpatialData.BindImage(0, 2);

			RawPathTrace[Window.GetFrameCount() % 4].BindImage(7, 2);

			MotionVectors[Window.GetFrameCount() % 4].BindImage(0, 3);
			TemporalFrameCount.BindImage(0,4);

			PreSpatialTemporalFilter.SetUniform("SubFrame", Window.GetFrameCount() % 4);
			PreSpatialTemporalFilter.SetUniform("CheckerStep", (Window.GetFrameCount() / 4) % 2);

			DrawPostProcessQuad();

			PreSpatialTemporal.UnBind(Window);

			PreSpatialTemporalFilter.UnBind();


			PreUpscaled.Bind(); 
			
			PreUpscaler.Bind(); 

			Clouds.BindImage(0, 0); 
			PreUpscaled.BindImagePrevious(0, 1); 
			MotionVectors[Window.GetFrameCount() % 4].BindImage(2, 2); 
			MotionVectors[Window.GetFrameCount() % 4].BindImage(0, 3);
			PreSpatialTemporal.BindImage(4);
			PreUpscaled.BindImagePrevious(1, 5);
			PreUpscaled.BindImagePrevious(2, 6);
			PreUpscaled.BindImagePrevious(3, 7);

			PreUpscaler.SetUniform("Frame", Window.GetFrameCount() % 64); 

			DrawPostProcessQuad(); 

			PreUpscaler.UnBind();

			PreUpscaled.UnBind(); 
		}

		void LightManager::ReloadIndirect(Window& Window)
		{
			IndirectLightShader.Reload("Shaders/RawPathTracing", Chunk::GetInjectionCode());
			TemporalUpscaler.Reload("Shaders/TemporalUpscaler");
			SpatialFilter.Reload("Shaders/SpatialFilter");
			SpatialFilterFinal.Reload("Shaders/SpatialFilter", std::string("#define SPATIAL_FINAL"));
			SpatialUpscaler.Reload("Shaders/SpatialUpscaler");
			TemporalFilter.Reload("Shaders/TemporalFilter");
			RTMotionVectorCalculator.Reload("Shaders/RTMotionVectors");
			SpatialPacker.Reload("Shaders/SpatialPacker");
			FrameCount.Reload("Shaders/TemporalFrameCounter");
			Volumetrics.Reload("Shaders/Volumetrics"); 
			DirectBlocker.Reload("Shaders/DirectBlocker"); 
			CloudRenderer.Reload("Shaders/Clouds"); 
			CloudProjection.Reload("Shaders/CloudProjection"); 
			PreSpatialTemporalFilter.Reload("Shaders/PreSpatialTemporal"); 
			CheckerboardUpscaler.Reload("Shaders/CheckerBoardHandler"); 
			WaterRefraction.Reload("Shaders/WaterRefraction"); 
			WaterDepthPacker.Reload("Shaders/WaterDepthPacker"); 
			WaterErrorBlur.Reload("Shaders/ErrorMaskBlur"); 
			PreUpscaler.Reload("Shaders/PreUpscaler"); 
			CheckerUpscaler.Reload("Shaders/CheckerUpscaler");
			SpecularMotionVectorResolver.Reload("Shaders/SpecularMotionVectorResolver"); 

			SetShaderUniforms(Window); 

		}

		void LightManager::SetShaderUniforms(Window& Window)
		{

			//Indirect light -> 
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
				IndirectLightShader.SetUniform("DirectionalCascadesRaw[0]", 19);
				IndirectLightShader.SetUniform("DirectionalCascadesRaw[1]", 20);
				IndirectLightShader.SetUniform("DirectionalCascadesRaw[2]", 21);
				IndirectLightShader.SetUniform("DirectionalCascadesRaw[3]", 22);
				IndirectLightShader.SetUniform("SkyNoMie", 23);
				IndirectLightShader.SetUniform("BlockerData", 24);
				IndirectLightShader.SetUniform("DisplacementTextures", 25);
				IndirectLightShader.SetUniform("ParallaxDirections", BAKE_DIRECTIONS);
				IndirectLightShader.SetUniform("ParallaxResolution", BAKE_RESOLUTION);
				IndirectLightShader.SetUniform("ParallaxData", 26);
				IndirectLightShader.SetUniform("TCData", 39);
				IndirectLightShader.SetUniform("LowFrequencyNormal", 40);
				IndirectLightShader.SetUniform("NormalTextures", 29);
				IndirectLightShader.SetUniform("ProjectedClouds", 30);

				IndirectLightShader.SetUniform("DirectionalRefractive[0]", 31);
				IndirectLightShader.SetUniform("DirectionalRefractive[1]", 32);
				IndirectLightShader.SetUniform("DirectionalRefractive[2]", 33);
				IndirectLightShader.SetUniform("DirectionalRefractive[3]", 34);

				IndirectLightShader.SetUniform("Albedo", 35);

				IndirectLightShader.SetUniform("WaterWorldPos", 36);
				IndirectLightShader.SetUniform("WaterNormal", 37);
				IndirectLightShader.SetUniform("WaterDepth", 38);

				IndirectLightShader.SetUniform("SpecularDetail", 0);
				IndirectLightShader.SetUniform("SpecularDirectionData", 1);

				IndirectLightShader.UnBind();

			}

			//Temporal upscaling -> 
			{

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

				TemporalUpscaler.SetUniform("FramesDirect[0]", 30);
				TemporalUpscaler.SetUniform("FramesDirect[1]", 31);
				TemporalUpscaler.SetUniform("FramesDirect[2]", 32);
				TemporalUpscaler.SetUniform("FramesDirect[3]", 33);

				TemporalUpscaler.SetUniform("MotionVectorsSpecular[0]", 34);
				TemporalUpscaler.SetUniform("MotionVectorsSpecular[1]", 35);
				TemporalUpscaler.SetUniform("MotionVectorsSpecular[2]", 36);
				TemporalUpscaler.SetUniform("MotionVectorsSpecular[3]", 37);


				TemporalUpscaler.SetUniform("Resolution", Window.GetResolution() / 4);

				TemporalUpscaler.UnBind();

			}

			//Motion vectors -> 
			{

				SpecularMotionVectorResolver.Bind(); 

				SpecularMotionVectorResolver.SetUniform("SpecularDirection", 0);
				SpecularMotionVectorResolver.SetUniform("UpscaleDataSpecular", 1);

				SpecularMotionVectorResolver.UnBind(); 


				RTMotionVectorCalculator.Bind();

				RTMotionVectorCalculator.SetUniform("WorldPosPrevious", 0);
				RTMotionVectorCalculator.SetUniform("Normal", 1);
				RTMotionVectorCalculator.SetUniform("NormalPrevious", 2);
				RTMotionVectorCalculator.SetUniform("PreviousWorldPos", 3);
				RTMotionVectorCalculator.SetUniform("CurrentLighting", 4);
				RTMotionVectorCalculator.SetUniform("PreviousLighting", 5);
				RTMotionVectorCalculator.SetUniform("CloudDepth", 6);

				RTMotionVectorCalculator.SetUniform("WorldPosWaterPrevious", 7);
				RTMotionVectorCalculator.SetUniform("PreviousWorldPosWater", 8);
				RTMotionVectorCalculator.SetUniform("NormalWater", 9);
				RTMotionVectorCalculator.SetUniform("PreviousNormalWater", 10);
				RTMotionVectorCalculator.SetUniform("Depth", 11);
				RTMotionVectorCalculator.SetUniform("WaterDepth", 12);

				RTMotionVectorCalculator.SetUniform("CurrentDirection", 13);
				RTMotionVectorCalculator.SetUniform("CurrentOrigin", 14);

				RTMotionVectorCalculator.SetUniform("PreviousDirection", 15);
				RTMotionVectorCalculator.SetUniform("PreviousOrigin", 16);


				RTMotionVectorCalculator.SetUniform("Resolution", Vector2f(Window.GetResolution() / 2));

				RTMotionVectorCalculator.UnBind();

			}

			//Spatial packer -> 
			{
				SpatialPacker.Bind();

				SpatialPacker.SetUniform("InputNormal", 0);
				SpatialPacker.SetUniform("InputDepth", 1);
				SpatialPacker.SetUniform("InputNormalHF", 2);
				SpatialPacker.SetUniform("InputWaterDepth", 3);
				SpatialPacker.SetUniform("InputWaterNormal", 4);
				SpatialPacker.SetUniform("InputWaterPosition", 5);
				SpatialPacker.SetUniform("InputDeferredPosition", 6);

				SpatialPacker.UnBind();
			}

			//Spatial filter -> 
			{
				SpatialFilter.Bind();

				SpatialFilter.SetUniform("InputPacked", 0);
				SpatialFilter.SetUniform("InputLighting", 1);
				SpatialFilter.SetUniform("FrameCount", 2);
				SpatialFilter.SetUniform("InputSpecular", 3);
				SpatialFilter.SetUniform("MotionVectors", 4);
				SpatialFilter.SetUniform("Direct", 5);
				SpatialFilter.SetUniform("Detail", 6);
				SpatialFilter.SetUniform("InputSHCg", 7);
				SpatialFilter.SetUniform("RawDepth", 8);
				SpatialFilter.SetUniform("RawWaterDepth", 9);

				SpatialFilter.UnBind();

				SpatialFilterFinal.Bind();

				SpatialFilterFinal.SetUniform("InputPacked", 0);
				SpatialFilterFinal.SetUniform("InputLighting", 1);
				SpatialFilterFinal.SetUniform("FrameCount", 2);
				SpatialFilterFinal.SetUniform("InputSpecular", 3);
				SpatialFilterFinal.SetUniform("MotionVectors", 4);
				SpatialFilterFinal.SetUniform("Direct", 5);
				SpatialFilterFinal.SetUniform("Detail", 6);
				SpatialFilterFinal.SetUniform("InputSHCg", 7);
				SpatialFilterFinal.SetUniform("RawDepth", 8);
				SpatialFilterFinal.SetUniform("RawWaterDepth", 9);

				SpatialFilterFinal.UnBind();
			}

			//Spatial upscaler -> 
			{
				SpatialUpscaler.Bind();

				SpatialUpscaler.SetUniform("Depth", 0);
				SpatialUpscaler.SetUniform("Normal", 1);
				SpatialUpscaler.SetUniform("Lighting", 2);
				SpatialUpscaler.SetUniform("PackedGeometryData", 3);
				SpatialUpscaler.SetUniform("Specular", 5);
				SpatialUpscaler.SetUniform("Direct", 6);
				SpatialUpscaler.SetUniform("PackedGeometryDataSpecular", 7);
				SpatialUpscaler.SetUniform("DepthWater", 8);
				SpatialUpscaler.SetUniform("NormalHF", 9);
				SpatialUpscaler.SetUniform("WaterNormal", 10);

				SpatialUpscaler.UnBind();
			}

			//Temporal filter -> 
			{
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

				TemporalFilter.SetUniform("UpscaledDirect", 11);
				TemporalFilter.SetUniform("PreviousDirect", 12);

				TemporalFilter.SetUniform("Depth", 13);
				TemporalFilter.SetUniform("WaterDepth", 14);

				TemporalFilter.SetUniform("MotionVectorsSpecular", 15);
				TemporalFilter.SetUniform("FrameCountSpecular", 16);


				TemporalFilter.UnBind();
			}
			
			//Frame counter -> 
			{
				FrameCount.Bind();

				FrameCount.SetUniform("MotionVectors", 0);
				FrameCount.SetUniform("PreviousFrame", 1);
				FrameCount.SetUniform("PreviousSpecularFrame", 2);
				FrameCount.SetUniform("SpecularMotionVectors", 3);

				FrameCount.UnBind();
			}

			//Volumetrics -> 
			{
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
				Volumetrics.SetUniform("DirectionalRefractive[0]", 14);
				Volumetrics.SetUniform("DirectionalRefractive[1]", 15);
				Volumetrics.SetUniform("DirectionalRefractive[2]", 16);
				Volumetrics.SetUniform("DirectionalRefractive[3]", 17);
				Volumetrics.SetUniform("DirectionalRefractiveDepth[0]", 18);
				Volumetrics.SetUniform("DirectionalRefractiveDepth[1]", 19);
				Volumetrics.SetUniform("DirectionalRefractiveDepth[2]", 20);
				Volumetrics.SetUniform("DirectionalRefractiveDepth[3]", 21);
				Volumetrics.SetUniform("PrimaryRefractiveDepth", 22);

				Volumetrics.UnBind();
			}

			//Direct blocker -> 
			{
				DirectBlocker.Bind();

				DirectBlocker.SetUniform("DirectionalCascades[0]", 19);
				DirectBlocker.SetUniform("DirectionalCascades[1]", 20);
				DirectBlocker.SetUniform("DirectionalCascades[2]", 21);
				DirectBlocker.SetUniform("DirectionalCascades[3]", 22);
				DirectBlocker.SetUniform("WorldPosition", 4);
				DirectBlocker.SetUniform("Depth", 5);

				DirectBlocker.UnBind();
			}
			
			//Clouds -> 
			{
				CloudRenderer.Bind();

				CloudRenderer.SetUniform("ProjectedClouds", 0);
				CloudRenderer.SetUniform("BasicBlueNoise", 2);
				CloudRenderer.SetUniform("CloudNoise", 3);
				CloudRenderer.SetUniform("WeatherMap", 4);
				CloudRenderer.SetUniform("CloudShape", 5);
				CloudRenderer.SetUniform("Turbulence", 6);
				CloudRenderer.SetUniform("Cirrus", 7);

				CloudRenderer.SetUniform("TextureSize", Window.GetResolution() / 2);

				CloudRenderer.UnBind();

				CloudProjection.Bind();

				CloudProjection.SetUniform("PreviousCloudResult", 0);
				CloudProjection.SetUniform("BasicBlueNoise", 2);
				CloudProjection.SetUniform("CloudNoise", 3);
				CloudProjection.SetUniform("WeatherMap", 4);
				CloudProjection.SetUniform("CloudShape", 5);
				CloudProjection.SetUniform("Turbulence", 6);

				CloudProjection.UnBind();
			}

			//Pre-spatial temporal -> 
			{
				PreSpatialTemporalFilter.Bind();

				PreSpatialTemporalFilter.SetUniform("CurrentDetail", 0);
				PreSpatialTemporalFilter.SetUniform("PreviousDetail", 1);
				PreSpatialTemporalFilter.SetUniform("SpatialDenoiseData", 2);
				PreSpatialTemporalFilter.SetUniform("MotionVectors", 3);
				PreSpatialTemporalFilter.SetUniform("FrameCount", 4);

				PreSpatialTemporalFilter.UnBind();
			}

			//Checkerboard upscaler -> 
			{
				CheckerboardUpscaler.Bind();

				CheckerboardUpscaler.SetUniform("CloudsRaw", 0);
				CheckerboardUpscaler.SetUniform("VolumetricsRaw", 1);
				CheckerboardUpscaler.SetUniform("DiffuseLightingRaw", 2);
				CheckerboardUpscaler.SetUniform("SpecularLightingRaw", 3);
				CheckerboardUpscaler.SetUniform("DirectLightingRaw", 4);
				CheckerboardUpscaler.SetUniform("CloudDepthRaw", 5);
				CheckerboardUpscaler.SetUniform("UpscaleData", 6);
				CheckerboardUpscaler.SetUniform("Depth", 7);
				CheckerboardUpscaler.SetUniform("Normal", 8);
				CheckerboardUpscaler.SetUniform("UpscaleDataSpecular", 9);

				CheckerboardUpscaler.UnBind();
			}

			//Water -> 
			{

				WaterDepthPacker.Bind(); 

				WaterDepthPacker.SetUniform("Depth", 0);
				WaterDepthPacker.SetUniform("RefractedDepth", 1);

				WaterDepthPacker.UnBind(); 

				WaterRefraction.Bind(); 

				WaterRefraction.SetUniform("LFNormal", 0);
				WaterRefraction.SetUniform("WorldPosition", 1);
				WaterRefraction.SetUniform("Depth", 2);
				WaterRefraction.SetUniform("TraceDepth", 3);

				WaterRefraction.UnBind();

				WaterErrorBlur.Bind(); 

				WaterErrorBlur.SetUniform("InError", 0); 


				WaterErrorBlur.UnBind(); 

			}

			//Pre upscaler -> 

			{

				CheckerUpscaler.Bind();

				CheckerUpscaler.SetUniform("Detail", 0); 
				CheckerUpscaler.SetUniform("UpscaleData", 1);
				CheckerUpscaler.SetUniform("Depth", 2);
				CheckerUpscaler.SetUniform("Normal", 3);

				CheckerboardUpscaler.UnBind();

				PreUpscaler.Bind(); 

				PreUpscaler.SetUniform("Clouds", 0); 
				PreUpscaler.SetUniform("PreviousClouds", 1);
				PreUpscaler.SetUniform("MotionVectorsClouds", 2);
				PreUpscaler.SetUniform("MotionVectors", 3);
				PreUpscaler.SetUniform("Detail", 4);
				PreUpscaler.SetUniform("PreviousDetail", 5);
				PreUpscaler.SetUniform("PreviousTemporalDetail", 6);
				PreUpscaler.SetUniform("PreviousFrameCount", 7);

				PreUpscaler.UnBind();

			}

		}

		void LightManager::SpatialyUpscale(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{

			SpatialyUpscaled.Bind(); 

			SpatialUpscaler.Bind(); 

			SpatialUpscaler.SetUniform("znear", Camera.znear); 
			SpatialUpscaler.SetUniform("zfar", Camera.zfar);
			SpatialUpscaler.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));
			SpatialUpscaler.SetUniform("SpatialUpscaling", GetBoolean("spatialupscale"));

			Deferred.RawDeferred.BindDepthImage(0); 
			Deferred.Deferred.BindImage(0, 1);
			TemporallyFiltered.BindImage(0, 2); 
			PackedSpatialData.BindImage(0,3);
			TemporallyFiltered.BindImage(1, 4);
			TemporallyFiltered.BindImage(2, 5);
			TemporallyFiltered.BindImage(4, 6);
			PackedSpatialData.BindImage(1, 7);
			Deferred.RawWaterDeferred.BindDepthImage(8); 
			Deferred.Deferred.BindImage(3,9);
			Deferred.RawWaterDeferred.BindImage(2,10);

			DrawPostProcessQuad(); 

			SpatialUpscaler.UnBind();

			SpatialyUpscaled.UnBind();

		}

	}
}