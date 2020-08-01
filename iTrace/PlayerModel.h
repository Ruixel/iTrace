#pragma once

#include "FrameBuffer.h"

namespace iTrace {

	namespace Rendering {
		
		const enum class AnimationStage {IDLE, STEP_LEFT, STEP_RIGHT};

		struct PlayerModel {

			Vector3f Origins[6] = {
				Vector3f(0.014693, 0.302329, 0.176809),
				Vector3f(0.014693, 0.302329, -0.176809),
				Vector3f(0.025274, 0.988038,0.0),
				Vector3f(0.014693, 0.718496, 0.436037),
				Vector3f(0.014693, 0.718496, -0.436037),
				Vector3f(0.0, 1.56763, 0.0)
			};

			Vector3f Scales[6] = {
				Vector3f(0.1,0.3,0.1),
				Vector3f(0.1,0.3,0.1),
				Vector3f(0.203, 0.4, 0.326),
				Vector3f(0.121, 0.522, 0.121),
				Vector3f(0.121, 0.522, 0.121),
				Vector3f(0.177)
			};


		};


	}

}