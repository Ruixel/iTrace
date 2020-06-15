#pragma once

#include "Camera.h"
#include "Compositor.h"
#include "CommandManager.h"
#include "CommandPusher.h"

namespace iTrace {

	using namespace Rendering; 

	struct Pipeline {

		WorldManager World; 
		DeferredRenderer Deferred; 
		IndirectLightingHandler Indirect; 
		SkyRendering Sky; 
		Compositing Compositor; 
		GlowRenderer Glow; 
		LightCombiner Combiner; 
		TextSystem Text; 
		CommandManager Commands; 

		void PreparePipeline(Camera& Camera, Window& Window);
		void RunPipeline(Camera& Camera, Window& Window);
		


	};

}
