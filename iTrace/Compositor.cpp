#include "Indirect.h"
#include "Compositor.h"
#include "BooleanCommands.h"

namespace iTrace {

	namespace Rendering {

		void Compositing::PrepareCompositing()
		{

			RequestBoolean("hdrpostprocess", true);

			CompositorShader = Shader("Shaders/Composite"); 

			CompositorShader.Bind();

			CompositorShader.SetUniform("CombinedLighting", 0);
			CompositorShader.SetUniform("Depth", 1);
			CompositorShader.SetUniform("Glow", 2);
			CompositorShader.SetUniform("DoF", 3);
			CompositorShader.SetUniform("LensDirt", 5);

			CompositorShader.UnBind();

			LensDirt = LoadTextureGL("Resources/Post Process/Lens Dirt.png"); 

		}

		void Compositing::DoCompositing(Camera& Camera, DeferredRenderer& Deferred, LightCombiner& Combined, PostProcess& Glow)
		{


			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); 

			CompositorShader.Bind();

			CompositorShader.SetUniform("znear", Camera.znear); 
			CompositorShader.SetUniform("zfar", Camera.zfar);
			CompositorShader.SetUniform("DoGlow", GetBoolean("hdrpostprocess"));

			Combined.CombinedRefraction.BindImage(0,0); 
			Deferred.Deferred.BindDepthImage(1); 
			Glow.GlowBuffer[2][1].BindImage(2); 
			Glow.DoFBuffer.BindImage(3);
			LensDirt.Bind(5); 

			DrawPostProcessQuad(); 

			CompositorShader.UnBind(); 
			

		}

	}


}