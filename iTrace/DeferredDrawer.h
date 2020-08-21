#pragma once

#include "WorldManager.h"
#include "Profiler.h"
#include "Texture.h"
#include "SkyRenderer.h"

namespace iTrace {

	namespace Rendering {

		struct DeferredRenderer {

			Shader DeferredManager, TransparentDeferredManager, DeferredUnwrapper, RefractiveDeferredManager, PrimaryRefractiveDeferredManager, WaterDeferred;
			MultiPassFrameBufferObject RawDeferred, RawWaterDeferred;
			MultiPassFrameBufferObjectPreviousData Deferred, PrimaryDeferredRefractive;
			FrameBufferObject DeferredRefractive; //<- fallback refraction 
			TextureGL Noise, WaterNormal;
			unsigned int RainDrop; 

			void PrepareDeferredRenderer(Window& Window); 
			void RenderDeferred(SkyRendering & Sky, Window& Window, Camera& Camera, WorldManager & World, Vector3f & SunDirection); 
			void SetUniforms(Window& Window); 
			void ReloadDeferred(Window& Window);
		};

	}

}