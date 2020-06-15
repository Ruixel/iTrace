#pragma once

#include "Indirect.h"
#include "SkyRenderer.h"

namespace iTrace {

	namespace Rendering {

		struct LightCombiner {

			Shader LightCombinerShader; 

			FrameBufferObject CombinedLighting; 

			void PrepareLightCombiner(Window& Window); 
			void CombineLighting(Window& Window, Camera & Camera, IndirectLightingHandler& Indirect, DeferredRenderer& Deferred, SkyRendering& Sky); 
			void SetUniforms(Window& Window); 
			void ReloadLightCombiner(Window& Window); 

		};



	}

}