#pragma once

#include "Indirect.h"
#include "SkyRenderer.h"
#include "Particles.h"


namespace iTrace {

	namespace Rendering {

		struct LightCombiner {

			Shader LightCombinerShader, RefractiveCombiner; 

			MultiPassFrameBufferObject CombinedLighting, CombinedRefraction; 

			void PrepareLightCombiner(Window& Window); 
			void CombineLighting(Window& Window, Camera & Camera, LightManager& Indirect, DeferredRenderer& Deferred, SkyRendering& Sky, ParticleSystem & Particles); 
			void SetUniforms(Window& Window); 
			void ReloadLightCombiner(Window& Window); 

		};



	}

}