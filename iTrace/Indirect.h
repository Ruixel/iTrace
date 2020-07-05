#pragma once

#include "DeferredDrawer.h"
#include "SkyRenderer.h"

namespace iTrace {

	namespace Rendering {

		const std::array<int, 5> StepSizes = {1,3,9,8,16}; 

		struct LightManager {

			Shader IndirectLightShader, RTMotionVectorCalculator,
				TemporalUpscaler,SpatialPacker, SpatialFilter, 
				SpatialUpscaler, TemporalFilter, FrameCount, Volumetrics, DirectBlocker, CloudRenderer, CloudProjection; 

			MultiPassFrameBufferObject RawPathTrace[4], TemporalyUpscaled, SpatialyFiltered[8], SpatialyUpscaled, Clouds[4];
			FrameBufferObject MotionVectors[4], PackedData, DirectBlockerBuffer, PackedSpatialData;
			MultiPassFrameBufferObjectPreviousData TemporallyFiltered; 
			FrameBufferObjectPreviousData TemporalFrameCount, ProjectedClouds;
			FrameBufferObject VolumetricFBO[4];

			TextureGL WindNoise, SimplifiedBlueNoise, WeatherMap; 
			TextureGL3D CloudNoise; 

			unsigned int SobolTexture, RankingTexture, ScramblingTexture; 


			void PrepareIndirectLightingHandler(Window & Window); 
			void RenderIndirectLighting(Window& Window, Camera& Camera, DeferredRenderer & Deferred, WorldManager & World, SkyRendering& Sky);
			
			void FindDirectBlocker(Window& Window, Camera& Camera, DeferredRenderer& Deferred, SkyRendering& Sky);
			void DoRawPathTrace(Window& Window, Camera& Camera, DeferredRenderer& Deferred, WorldManager& World, SkyRendering & Sky);
			void TemporalyUpscale(Window& Window, Camera& Camera, DeferredRenderer& Deferred);
			void SpatialyFilter(Window& Window, Camera& Camera, DeferredRenderer& Deferred);
			void SpatialyUpscale(Window& Window, Camera& Camera, DeferredRenderer& Deferred);
			void TemporalyFilter(Window& Window, Camera& Camera, DeferredRenderer& Deferred);
			void DoVolumetricLighting(Window& Window, Camera& Camera, DeferredRenderer& Deferred, WorldManager& World, SkyRendering& Sky);
			void GenerateMotionVectors(Window& Window, Camera& Camera, DeferredRenderer& Deferred); 
			void RenderClouds(Window & Window,Camera& Camera, DeferredRenderer & Deferred, SkyRendering& Sky);

			void ReloadIndirect(Window& Window); 
			void SetShaderUniforms(Window& Window); 

		};

	}


}