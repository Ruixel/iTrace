#include "Indirect.h"
#include "External/BlueNoiseData.h"
#include "BooleanCommands.h"

namespace iTrace {

	namespace Rendering {



		void IndirectLightingHandler::PrepareIndirectLightingHandler(Window& Window)
		{
			RequestBoolean("raytracing", true); 
			RequestBoolean("volumetrics", true);
			RequestBoolean("spatial", true);
			RequestBoolean("temporal", true);

			
			for (int x = 0; x < 4; x++) {
				RawPathTrace[x] = MultiPassFrameBufferObject(Window.GetResolution() / 4, 3, { GL_RGBA16F, GL_RGBA16F, GL_RGB32F }, false); 
				MotionVectors[x] = FrameBufferObject(Window.GetResolution() / 2, GL_RG16F, false); 
				SpatialyFiltered[x * 2] = MultiPassFrameBufferObject(Window.GetResolution() / 4, 2, { GL_RGBA16F,GL_RGBA16F }, false);
				SpatialyFiltered[x * 2 + 1] = MultiPassFrameBufferObject(Window.GetResolution() / 4, 2, { GL_RGBA16F,GL_RGBA16F }, false);
				VolumetricFBO[x] = FrameBufferObject(Window.GetResolution() / 4, GL_RGBA16F, false); 
			}

			TemporalFrameCount = FrameBufferObjectPreviousData(Window.GetResolution() / 2, GL_R16F, false);
			TemporalyUpscaled = MultiPassFrameBufferObject(Window.GetResolution() / 2, 2, { GL_RGBA16F,GL_RGBA16F }, false);
			PackedSpatialData = FrameBufferObject(Window.GetResolution() / 2, GL_RGBA16F, false); 
			TemporallyFiltered = MultiPassFrameBufferObjectPreviousData(Window.GetResolution() / 2, 2, { GL_RGBA16F,GL_RGBA16F }, false);
			SpatialyUpscaled = MultiPassFrameBufferObject(Window.GetResolution(), 1, { GL_RGBA16F }, false);

			IndirectLightShader = Shader("Shaders/RawPathTracing");
			TemporalUpscaler = Shader("Shaders/TemporalUpscaler");
			SpatialFilter = Shader("Shaders/SpatialFilter"); 
			SpatialUpscaler = Shader("Shaders/SpatialUpscaler"); 
			TemporalFilter = Shader("Shaders/TemporalFilter"); 
			RTMotionVectorCalculator = Shader("Shaders/RTMotionVectors"); 
			SpatialPacker = Shader("Shaders/SpatialPacker"); 
			FrameCount = Shader("Shaders/TemporalFrameCounter");
			Volumetrics = Shader("Shaders/Volumetrics"); 

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

		}

		void IndirectLightingHandler::RenderIndirectLighting(Window& Window, Camera& Camera, DeferredRenderer& Deferred, WorldManager& World, SkyRendering& Sky)
		{

			
			Profiler::FlushTime(); 
			DoRawPathTrace(Window, Camera, Deferred, World, Sky); 
			Profiler::SetPerformance("Raw path tracing"); 
	
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

		void IndirectLightingHandler::DoRawPathTrace(Window& Window, Camera& Camera, DeferredRenderer& Deferred, WorldManager& World, SkyRendering& Sky)
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
			glBindTexture(GL_TEXTURE_2D_ARRAY, Chunk::GetTextureArrayList(2));

			glActiveTexture(GL_TEXTURE16);
			glBindTexture(GL_TEXTURE_1D, Chunk::GetTextureExtensionData());

			glActiveTexture(GL_TEXTURE17);
			glBindTexture(GL_TEXTURE_1D, Chunk::GetBlockExtraDataTexture());

			glActiveTexture(GL_TEXTURE10);
			glBindTexture(GL_TEXTURE_2D_ARRAY, Sky.HemiSphericalShadowMap);



			for (int i = 0; i < 48; i++)
			{
				IndirectLightShader.SetUniform("HemisphericalMatrices[" + std::to_string(i) + "]", Sky.HemiProjectionMatrix * Sky.HemiViewMatrices[i]);

			}
			for (int i = 0; i < 48; i++)
			{
				IndirectLightShader.SetUniform("HemisphericalDirections[" + std::to_string(i) + "]", Sky.HemiDirections[i]);

			}


			IndirectLightShader.SetUniform("CameraPosition", Camera.Position);
			IndirectLightShader.SetUniform("FrameCount", Window.GetFrameCount() % 1024);
			IndirectLightShader.SetUniform("State", Window.GetFrameCount() % 4);

			IndirectLightShader.SetUniform("UseWhiteNoise", sf::Keyboard::isKeyPressed(sf::Keyboard::V));
			IndirectLightShader.SetUniform("DoRayTracing", GetBoolean("raytracing"));
			IndirectLightShader.SetUniform("znear", Camera.znear);
			IndirectLightShader.SetUniform("zfar", Camera.zfar);

			DrawPostProcessQuad();

			IndirectLightShader.UnBind();

			RawPathTrace[Window.GetFrameCount() % 4].UnBind(Window);


		}

		void IndirectLightingHandler::SpatialyFilter(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
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

			for (int x = 0; x < 3; x++) {

				SpatialFilter.SetUniform("StepSize", StepSizes[x]);

				SpatialyFiltered[Addon + x % 2].Bind();

				DrawPostProcessQuad();

				SpatialyFiltered[Addon + x % 2].UnBind();

				SpatialyFiltered[Addon + x % 2].BindImage(0, 1);
				SpatialyFiltered[Addon + x % 2].BindImage(1, 4);


			}


			SpatialFilter.UnBind();

		}

		void IndirectLightingHandler::TemporalyUpscale(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{

			

			TemporalyUpscaled.Bind(); 

			TemporalUpscaler.Bind(); 

			for (int x = 0; x < 4; x++) {
				RawPathTrace[x].BindImage(1, x);
				RawPathTrace[x].BindImage(2, x + 4);
				
				SpatialyFiltered[x * 2].BindImage(0, x + 8); 
				SpatialyFiltered[x * 2].BindImage(1, x + 12);

				MotionVectors[x].BindImage(x + 16); 

			}

			Deferred.Deferred.BindImage(1, 20); 
			PackedSpatialData.BindImage(21); 


			TemporalUpscaler.SetUniform("Upscale", sf::Keyboard::isKeyPressed(sf::Keyboard::U)); 
			TemporalUpscaler.SetUniform("CurrentFrame", Window.GetFrameCount()%4);
			TemporalUpscaler.SetUniform("NewFiltering", GetBoolean("newfiltering"));

			DrawPostProcessQuad(); 

			TemporalUpscaler.UnBind(); 

			TemporalyUpscaled.UnBind(Window); 



		}

		void IndirectLightingHandler::TemporalyFilter(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
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
			
			TemporalFilter.SetUniform("DoTemporal", GetBoolean("temporal")); 
			TemporalFilter.SetUniform("NewFiltering", GetBoolean("newfiltering"));

			DrawPostProcessQuad();

			TemporalFilter.UnBind();

			TemporallyFiltered.UnBind(Window);


		}

		void IndirectLightingHandler::DoVolumetricLighting(Window& Window, Camera& Camera, DeferredRenderer& Deferred, WorldManager& World, SkyRendering& Sky)
		{

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
			glBindTexture(GL_TEXTURE_CUBE_MAP, Sky.SkyCube.Texture[0]);

			WindNoise.Bind(6); 

			for (int i = 0; i < 48; i++)
			{
				Volumetrics.SetUniform("HemisphericalMatrices[" + std::to_string(i) + "]", Sky.HemiProjectionMatrix * Sky.HemiViewMatrices[i]);

			}
			for (int i = 0; i < 48; i++)
			{
				Volumetrics.SetUniform("HemisphericalDirections[" + std::to_string(i) + "]", Sky.HemiDirections[i]);

			}

			DrawPostProcessQuad(); 

			VolumetricFBO[Window.GetFrameCount() % 4].UnBind();

			Volumetrics.UnBind(); 


		}

		void IndirectLightingHandler::GenerateMotionVectors(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{

			MotionVectors[Window.GetFrameCount() % 4].Bind();

			RTMotionVectorCalculator.Bind();

			Deferred.Deferred.BindImage(1, 0);
			Deferred.Deferred.BindImage(0, 1);
			Deferred.Deferred.BindImagePrevious(0, 2);
			Deferred.Deferred.BindImagePrevious(1, 3);

			RTMotionVectorCalculator.SetUniform("CameraPosition", Camera.Position);
			RTMotionVectorCalculator.SetUniform("MotionMatrix", Camera.Project * Camera.PrevView);
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

		void IndirectLightingHandler::ReloadIndirect(Window& Window)
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

			SetShaderUniforms(Window); 

		}

		void IndirectLightingHandler::SetShaderUniforms(Window& Window)
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

			TemporalUpscaler.SetUniform("Resolution", Window.GetResolution() / 4);

			TemporalUpscaler.UnBind();

			RTMotionVectorCalculator.Bind();

			RTMotionVectorCalculator.SetUniform("WorldPosPrevious", 0);
			RTMotionVectorCalculator.SetUniform("Normal", 1);
			RTMotionVectorCalculator.SetUniform("NormalPrevious", 2);
			RTMotionVectorCalculator.SetUniform("PreviousWorldPos", 3);
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


			SpatialFilter.UnBind();

			SpatialUpscaler.Bind();

			SpatialUpscaler.SetUniform("Depth", 0);
			SpatialUpscaler.SetUniform("Normal", 1);
			SpatialUpscaler.SetUniform("Lighting", 2);
			SpatialUpscaler.SetUniform("PackedGeometryData", 3);
			SpatialUpscaler.SetUniform("Specular", 4);

			SpatialUpscaler.UnBind();

			TemporalFilter.Bind();

			TemporalFilter.SetUniform("UpscaledDiffuse", 0);
			TemporalFilter.SetUniform("PreviousDiffuse", 1);
			TemporalFilter.SetUniform("UpscaledVolumetrics", 2);
			TemporalFilter.SetUniform("PreviousVolumetrics", 3);
			TemporalFilter.SetUniform("MotionVectors", 4);
			TemporalFilter.SetUniform("FrameCount", 5);
			TemporalFilter.SetUniform("NormalRoughness", 6);

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

			Volumetrics.UnBind(); 

		}

		void IndirectLightingHandler::SpatialyUpscale(Window& Window, Camera& Camera, DeferredRenderer& Deferred)
		{

			SpatialyUpscaled.Bind(); 

			SpatialUpscaler.Bind(); 

			SpatialUpscaler.SetUniform("znear", Camera.znear); 
			SpatialUpscaler.SetUniform("zfar", Camera.zfar);
			SpatialUpscaler.SetUniform("IncidentMatrix", glm::inverse(Camera.Project * Matrix4f(Matrix3f(Camera.View))));
			SpatialUpscaler.SetUniform("UsePreviousUpscaling", sf::Keyboard::isKeyPressed(sf::Keyboard::O));

			Deferred.Deferred.BindDepthImage(0); 
			Deferred.Deferred.BindImage(3, 1);
			TemporallyFiltered.BindImage(0, 2); 
			PackedSpatialData.BindImage(3);
			TemporallyFiltered.BindImage(1, 4);
			
			DrawPostProcessQuad(); 

			SpatialUpscaler.UnBind();

			SpatialyUpscaled.UnBind();

		}

		

	

		

	}

}