#pragma once

#include "DeferredDrawer.h"
#include "SkyRenderer.h"

namespace iTrace {

	namespace Rendering {

		const std::array<int, 5> StepSizes = {1,3,9,8,16}; 

		struct LightManager {

			Shader IndirectLightShader, RTMotionVectorCalculator,
				TemporalUpscaler,SpatialPacker, SpatialFilter, 
				SpatialUpscaler, TemporalFilter, FrameCount, Volumetrics, 
				DirectBlocker, CloudRenderer, CloudProjection, CheckerboardUpscaler,
				PreSpatialTemporalFilter; 

			MultiPassFrameBufferObject RawPathTrace[4], TemporalyUpscaled, SpatialyFiltered[8], SpatialyUpscaled, Clouds[4], Checkerboarder[4];
			FrameBufferObject MotionVectors[4], PackedData, DirectBlockerBuffer, PackedSpatialData;
			MultiPassFrameBufferObjectPreviousData TemporallyFiltered, PreSpatialTemporal;
			FrameBufferObjectPreviousData TemporalFrameCount, ProjectedClouds;
			FrameBufferObject VolumetricFBO[4];

			TextureGL WindNoise, SimplifiedBlueNoise, WeatherMap, Turbulence, Cirrus; 
			TextureGL3D CloudNoise, CloudShape; 

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
			void CheckerboardUpscale(Window& Window); 
			void RenderClouds(Window & Window,Camera& Camera, DeferredRenderer & Deferred, SkyRendering& Sky);

			void ReloadIndirect(Window& Window); 
			void SetShaderUniforms(Window& Window); 

		};

	}


}