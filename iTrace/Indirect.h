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
				PreSpatialTemporalFilter, SpatialFilterFinal, WaterRefraction, 
				WaterDepthPacker, WaterErrorBlur, PreUpscaler, CheckerUpscaler; 
			//																	- gives out 2 4d floats, 1 float-	 - gives out 3 4d floats -
			MultiPassFrameBufferObject RawPathTrace[4], TemporalyUpscaled, SpatialyFilteredTemporary[2],			SpatialyFiltered[4], SpatialyUpscaled, Clouds, Checkerboarder[4],  RefractedWater, MotionVectors[4], PackedSpatialData;
			FrameBufferObject PackedData, DirectBlockerBuffer;
			MultiPassFrameBufferObjectPreviousData TemporallyFiltered, PreUpscaled;
			FrameBufferObjectPreviousData TemporalFrameCount, ProjectedClouds, PreSpatialTemporal;
			FrameBufferObject VolumetricFBO[4], PackedWaterData, ErrorMaskBlur[2], CheckerUpscaledDetail;

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
			void CheckerboardUpscale(Window& Window, Camera & Camera,DeferredRenderer & Deferred); 
			void DoWaterRayTrace(Window& Window, Camera& Camera, DeferredRenderer& Deferred); 
			void RenderClouds(Window & Window,Camera& Camera, DeferredRenderer & Deferred, SkyRendering& Sky);
			void DoPreUpscaling(Window& Window, Camera& Camera, DeferredRenderer & Deferred); 

			void ReloadIndirect(Window& Window); 
			void SetShaderUniforms(Window& Window); 

		};

	}


}