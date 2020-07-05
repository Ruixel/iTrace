#pragma once

#include "WorldManager.h"
#include "Profiler.h"
#include "Texture.h"

namespace iTrace {

	namespace Rendering {

		struct DeferredRenderer {

			Shader DeferredManager, TransparentDeferredManager, DeferredUnwrapper;
			FrameBufferObject RawDeferred; 
			MultiPassFrameBufferObjectPreviousData Deferred; 
			TextureGL ParallaxMap; 

			void PrepareDeferredRenderer(Window& Window); 
			void RenderDeferred(Window& Window, Camera& Camera, WorldManager & World, Vector3f & SunDirection); 
			void SetUniforms(Window& Window); 
			void ReloadDeferred(Window& Window);
		};

	}

}