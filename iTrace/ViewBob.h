#pragma once

#include "FootstepManager.h"

namespace iTrace {

	namespace Sound {

		const float SteppingPoint = 0.5f; //<- the point where the actual "step" occurs 

		struct ViewBobHandler {

			bool RightLeg = false; //<- if the view pop should be on the right or left leg 
			bool HasStepped = false; //<- if the stepping sound has played in the current animation sequence
			float AnimationSequence = 0.0; //<- where in the animation sequence the current view bobbing is. 



			Matrix4f PollViewBobbing(Camera & Camera,Window & Window, SoundHandler & Sound, FootStepManager& Manager, int BlockIdx, float VelocityLength); //<- returns a view bobbing matrix

		};

	}

}

