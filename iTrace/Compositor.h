#pragma once

#include "LightCombiner.h"
#include "Glow.h"


namespace iTrace {

	namespace Rendering {


		struct Compositing {

			Shader CompositorShader; 
			TextureGL LensDirt; 


			void PrepareCompositing(); 
			void DoCompositing(Camera & Camera,DeferredRenderer & Deferred,LightCombiner & Combined, PostProcess & Glow); 

		};
		



	}


}