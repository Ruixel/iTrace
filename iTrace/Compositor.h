#pragma once

#include "LightCombiner.h"
#include "Glow.h"


namespace iTrace {

	namespace Rendering {


		struct Compositing {

			Shader CompositorShader, ToneMapShader;
			FrameBufferObjectPreviousData BlitFBO; 
			TextureGL LensDirt; 
			ShaderBuffer<Vector4f> AverageColor;

			float Exposure = 1.0f, CurrentExposure = 1.0f; 


			void PrepareCompositing(Window & Window); 
			void DoCompositing(Window & Window,Camera & Camera,DeferredRenderer & Deferred,LightCombiner & Combined, PostProcess & Glow); 
			void ReloadCompositing(); 

		private: 

			void SetShaderUniforms(); 

		};
		



	}


}