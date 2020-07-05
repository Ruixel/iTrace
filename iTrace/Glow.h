#pragma once

/*

glow: 

only real post-process effect. The reason for this is because I think it looks really quite lovely. 
It can also be done at a really really low resolution. 
(the glow blur buffer is going to be created at 1/4th res, then mip-mapped to 1/8th res, and then blurred 
so it is very very cheap. 




*/

#include "LightCombiner.h"

namespace iTrace {

	const int Divisors[3] = { 8,4,2 }; 

	namespace Rendering {


		struct PostProcess {

			Shader PrepShader, BlurShader[2], DofShader, GlowPrepShader, DoFPrepShader; 

			FrameBufferObject GlowBuffer[3][2], DoFBuffer; 
			MultiPassFrameBufferObject PrepBuffer, DoFPrepBuffer; 


			void PreparePostProcess(Window & Window, Camera & Camera);
			void RenderPostProcess(Window& Window, SkyRendering& Sky, LightManager & Indirect, DeferredRenderer& Deferred, LightCombiner & Combined); 
			void ReloadPostProcess(Window& Window, Camera& Camera);

		private: 
			void SetShaderUniforms(Window& Window, Camera & Camera); 

		};


	}

}
