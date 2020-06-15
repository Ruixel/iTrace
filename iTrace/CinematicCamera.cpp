#include "CinematicCamera.h"
#include <unordered_map>
#include "CommandPusher.h"

namespace iTrace {

	namespace Rendering {

		std::unordered_map<std::string, KeyFrame> KeyFrames; 
		std::unordered_map<std::string, Animation> Animations; 

		Animation* PlayingAnimation = nullptr; 

		std::vector<std::string> KeyFrameCommand(std::vector<std::string> Input)
		{


			auto ParseInterpolationMode = [](std::string& Text) {

				if (Text == "linear") {
					return InterpolationMode::LINEAR; 
				}
				else if (Text == "sine") {
					return InterpolationMode::SINE; 
				}
				else {
					return InterpolationMode::NONE; 
				}

			}; 


			if (Input.size() == 0) {
				return { "Not enough data provided for keyframe command" }; 
			}

			if (Input[0] == ">help") {

				//todo: write help text (probably needs to be pretty long)

				return { "temporary help text" }; 

			}

			if (Input.size() < 2) {

				return { "Not enough data provided for keyframe command" };

			}

			if (Input[0] == "add") {

				if (Input.size() < 4) {
					return { "Not enough data provided for keyframe command" };
				}

				std::string& Name = Input[1];

				if (KeyFrames.find(Name) == KeyFrames.end()) {
					
					KeyFrame ToAddKeyFrame; 

					ToAddKeyFrame.FirstHalf = ParseInterpolationMode(Input[2]); 
					ToAddKeyFrame.SecondHalf = ParseInterpolationMode(Input[3]);

					if (ToAddKeyFrame.FirstHalf == NONE || ToAddKeyFrame.SecondHalf == NONE) {
						return { "incorrect interpolation mode has been used", "available modes are: linear and sine" };
					}

					ToAddKeyFrame.Position = std::any_cast<Vector3f>(GetGlobalCommandPusher().GivenConstantData["camera_pos"]); 
					ToAddKeyFrame.Rotation = glm::mod(std::any_cast<Vector3f>(GetGlobalCommandPusher().GivenConstantData["camera_rot"]),360.f);
					
					KeyFrames[Name] = ToAddKeyFrame; 

					return { "Added keyframe: " + Name }; 

				}
				else {
					return { "There is already a keyframe with the name: " + Name, 
						"if you wish to modify this keyframe, use 'keyframe modify [...]' instead" };
				}


			}
			else if (Input[0] == "remove") {

				std::string& Name = Input[1]; 

				if (KeyFrames.find(Name) == KeyFrames.end()) {
					return { "Could not find keyframe called: " + Name }; 
				}
				else {
					KeyFrames.erase(Name); 
				}


			}
			else if (Input[0] == "modify") {
				
				if (Input.size() < 4) {
					return { "Not enough data provided for keyframe command" };
				}

				std::string& Name = Input[1];

				if (KeyFrames.find(Name) == KeyFrames.end()) {
					return { "Could not find keyframe called: " + Name };
				}
				
			}
			else {

				return { Input[0] + " is not a correct syntax for the keyframe command, see keyframe >help" }; 

			}

			return std::vector<std::string>();
		}
		std::vector<std::string> AnimationCommand(std::vector<std::string> Input)
		{
			if (Input.size() < 1) {
				return { "Not enough data provided for animation command" };
			}

			if (Input[0] == ">help") {
				return { "temporary help text" }; 
			}

			if (Input[0] == "add") {

				if (Input.size() < 2) {
					return { "Not enough data provided for animation command" };
				}

				auto& Name = Input[1]; 

				Animation ToAddAnimation; 

				if (Animations.find(Name) == Animations.end()) {

					//parse keyframes + keyframe timings 

					for (int i = 0; i < (Input.size() - 2) / 2; i++) {

						auto& KeyFrameName = Input[i * 2 + 2]; 
						float KeyFrameTiming;
						AnimationKeyFrame AnimationKeyFrame; 

						if (KeyFrames.find(KeyFrameName) == KeyFrames.end()) {
							return { "No keyframe with the name: " + KeyFrameName }; 
						}
						if (Core::SafeParseFloat(Input[i * 2 + 3], KeyFrameTiming)) {

							auto& KeyFrame = KeyFrames[KeyFrameName]; 
							AnimationKeyFrame = KeyFrame; 
							AnimationKeyFrame.Time = ToAddAnimation.LatestTime + KeyFrameTiming; 

						}
						else {

							return { "Failed to parse timing: " + Input[i * 2 + 3] }; 

						}

						ToAddAnimation.KeyFrames.push_back(AnimationKeyFrame); 
						ToAddAnimation.LatestTime += KeyFrameTiming; 
					}

					Animations[Name] = ToAddAnimation; 

					return { "Added animation: " + Name }; 
					
				}
				else {

					return { "There already exists an animation called: " + Name }; 


				}


			}
			else if (Input[0] == "play") {

				if (Input.size() < 2) {
					return { "Not enough data provided for animation command" };
				}

				auto& Name = Input[1]; 

				if (Animations.find(Name) == Animations.end()) {

					return { "No animation called: " + Name }; 

				}
				else {

					PlayingAnimation = &Animations[Name]; 
					//ideally integrate some kind of safety here to ensure you cannot remove
					//a playing animation 

					return { "Now playing animation: " + Name }; 

				}




			}

		}
		bool PollAnimation(Camera& Camera, Window& Window)
		{
			if (PlayingAnimation == nullptr) {
				return false; 
			}
			else {

				PlayingAnimation->TimeInAnimation += Window.GetFrameTime(); 

				
				if (PlayingAnimation->TimeInAnimation >= PlayingAnimation->LatestTime) {
					PlayingAnimation->TimeInAnimation = 0.0; 
					PlayingAnimation->ActiveKeyFrame = 0; 
					PlayingAnimation = nullptr; 
					return false; 
				}

				//begin by interpolating only the positions! 

				while (PlayingAnimation->TimeInAnimation >= PlayingAnimation->KeyFrames[PlayingAnimation->ActiveKeyFrame].Time) {
					PlayingAnimation->ActiveKeyFrame++; 
				}

				if (PlayingAnimation->ActiveKeyFrame == 0) {

					Camera.Position = PlayingAnimation->KeyFrames[0].Position; 
					Camera.Rotation = PlayingAnimation->KeyFrames[0].Rotation;

				}
				else {

					int KeyFrameIdx = PlayingAnimation->ActiveKeyFrame; 
					
					auto& CurrentKeyFrame = PlayingAnimation->KeyFrames[KeyFrameIdx - 1]; 
					auto& NextKeyFrame = PlayingAnimation->KeyFrames[KeyFrameIdx]; 

					float Interpolation = (PlayingAnimation->TimeInAnimation - CurrentKeyFrame.Time) / (NextKeyFrame.Time - CurrentKeyFrame.Time);

					//for now, linear! 

					Interpolation = 1.0 - (sin(Interpolation * 3.14159265 + 1.570796) * 0.5 + 0.5);

					Camera.Position = glm::mix(CurrentKeyFrame.Position, NextKeyFrame.Position, Interpolation); 

					auto MixRotation = [](float a, float b, float interp) {
					
						float dist = abs(a - b); 

						if (dist > 180.0) {
							//we need to use the different direction! 

							//one of our angles needs to be converted! 

							if (a > 180.0 && b < 180.0) {

								//here we use an example, a is 350 degrees,
								//and b is 10 degrees 
								//here b should be converted to 370 degrees,
								//and then it should be interpolated

								b = b + 360.0;

								return glm::mod(glm::mix(a, b, interp), 360.f);

							}
							else {

								//here we use an example, a is 10 degrees,
								//and b is 350 degrees 
								//here a should be converted to 370 degrees,
								//and then it should be interpolated

								a = a + 360.0; 

								return glm::mod(glm::mix(a, b, interp), 360.f);

							}




						}
						else {
							return glm::mix(a, b, interp); 
						}




					
					}; 

					Camera.Rotation = Vector3f(
						MixRotation(CurrentKeyFrame.Rotation.x, NextKeyFrame.Rotation.x, Interpolation),
						MixRotation(CurrentKeyFrame.Rotation.y, NextKeyFrame.Rotation.y, Interpolation),
						MixRotation(CurrentKeyFrame.Rotation.z, NextKeyFrame.Rotation.z, Interpolation)
					);


				}


				return true; 
			}
		}
	}

}