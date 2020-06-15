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

	const int Divisors[3] = { 16,8,4 }; 

	namespace Rendering {


		struct GlowRenderer {

			Shader PrepShader, BlurShader[2], DofShader[2], GlowPrepShader; 

			FrameBufferObject GlowBuffer[3][2], DoFBuffer[2]; 
			MultiPassFrameBufferObject PrepBuffer; 


			void PrepareGlow(Window & Window, Camera & Camera);
			void RenderGlow(Window& Window, SkyRendering& Sky, IndirectLightingHandler & Indirect, DeferredRenderer& Deferred, LightCombiner & Combined); 


		};


	}

}
