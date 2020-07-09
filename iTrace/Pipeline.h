#pragma once

#include "Camera.h"
#include "Compositor.h"
#include "CommandManager.h"
#include "CommandPusher.h"
#include "Inventory.h"
#include "FootstepManager.h"
#include "SoundEffectManager.h"

namespace iTrace {

	using namespace Rendering; 
	using namespace Sound; 
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
		SoundHandler Sounds; 
		SoundInstance TestSound; 
		FootStepManager FootSteps; 
		SoundEffectManager SoundEffects; 
		ParticleSystem Particles; 

		void PreparePipeline(Camera& Camera, Window& Window);
		void RunPipeline(Camera& Camera, Window& Window);
		


	};

}
