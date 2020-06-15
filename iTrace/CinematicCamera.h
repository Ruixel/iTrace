#pragma once

#include <vector>
#include <string>
#include "Core.h"
#include "Camera.h"


namespace iTrace {

	namespace Rendering {



		enum InterpolationMode {

			LINEAR, SINE, NONE

		};

		struct KeyFrame {

			InterpolationMode FirstHalf = LINEAR, SecondHalf = LINEAR; 

			Vector3f Position, Rotation; 

		};

		struct AnimationKeyFrame : KeyFrame {
			float Time;

			void operator=(KeyFrame& key) {

				this->FirstHalf = key.FirstHalf;
				this->SecondHalf = key.SecondHalf;
				this->Position = key.Position;
				this->Rotation = key.Rotation;

			}

		};

		struct Animation {

			std::vector<AnimationKeyFrame> KeyFrames = {};

			float TimeInAnimation = 0.0; 
			float LatestTime = 0.0; 
			int ActiveKeyFrame = 0; 

		};



		std::vector<std::string> KeyFrameCommand(std::vector<std::string> Input); 
		std::vector<std::string> AnimationCommand(std::vector<std::string> Input);

		bool PollAnimation(Camera& Camera, Window& Window); //-> returns true if an animation is playing


	}

}