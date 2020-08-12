#pragma once

#include "Indirect.h"
#include "SkyRenderer.h"
#include "Particles.h"
#include "ShaderBuffer.h"

namespace iTrace {

	namespace Rendering {

		struct LightCombiner {

			Shader LightCombinerShader, RefractiveCombiner; 

			ShaderBuffer<Vector4f> FocusSSBO; 

			MultiPassFrameBufferObject CombinedLighting, CombinedRefraction; 

			float FocusPoint = 1.0;
			float CurrentFocusPoint = 0.0; 
			float ActualFocusPoint = 1.0; 
			float PreviousFocusPoint = 1.0; 

			void PrepareLightCombiner(Window& Window); 
			void CombineLighting(Window& Window, Camera & Camera, LightManager& Indirect, DeferredRenderer& Deferred, SkyRendering& Sky, ParticleSystem & Particles); 
			void SetUniforms(Window& Window); 
			void ReloadLightCombiner(Window& Window); 

		};



	}

}