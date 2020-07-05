#pragma once

#include "Camera.h"
#include "Compositor.h"
#include "CommandManager.h"
#include "CommandPusher.h"
#include "Inventory.h"

namespace iTrace {

	using namespace Rendering; 

	struct Pipeline {

		WorldManager World; 
		DeferredRenderer Deferred; 
		LightManager Indirect; 
		SkyRendering Sky; 
		Compositing Compositor; 
		PostProcess Glow; 
		LightCombiner Combiner; 
		TextSystem Text; 
		CommandManager Commands; 
		Inventory Inventory; 

		void PreparePipeline(Camera& Camera, Window& Window);
		void RunPipeline(Camera& Camera, Window& Window);
		


	};

}
