#include "ViewBob.h"



namespace iTrace {

	namespace Sound {
		
		using namespace Rendering::Chunk; 

		Matrix4f ViewBobHandler::PollViewBobbing(Camera& Camera, Window& Window, SoundHandler& Sound, FootStepManager& Manager, int BlockIdx, float VelocityLength)
		{
		
			if (BlockIdx != 0 && VelocityLength > 1e-4f) { //<- if its an air block 

				AnimationSequence += Window.GetFrameTime() * sqrt(VelocityLength); 

				if (AnimationSequence >= 1.0f) {
					RightLeg = !RightLeg; 
					HasStepped = false; 
					AnimationSequence = 0.0f; 
				}

				if (AnimationSequence >= SteppingPoint && !HasStepped) {
					HasStepped = true; 
					auto& BlockProperties = GetBlock(BlockIdx); 

					Manager.SetActiveMaterial(BlockProperties.SoundMaterialType, Sound);
					Manager.Step(); 
				}

			}
			else {

				if (AnimationSequence > SteppingPoint && AnimationSequence <= 1.0f) {

					//need this to be smoother -> 

					float SmoothFactor = (1.0 - AnimationSequence); 

					AnimationSequence += Window.GetFrameTime() * SmoothFactor;
				}
				else if (AnimationSequence > 0.0f && AnimationSequence < SteppingPoint) {

					float SmoothFactor = (AnimationSequence);

					AnimationSequence -= Window.GetFrameTime() * SmoothFactor;
				}
				
			}
		
			//the actual stepping animation -> 

			//for now use a simple sin wave for rotation. Should be changed 

			float SequenceSine = AnimationSequence * 3.1415926535; //<- because sin intervals between 0-1 between 0 and pi


			auto GetRotationFromSequence = [](float Sequence, float Velocity) {

				//the primary stepping 

				float PrimaryStepSine = pow(sin(Sequence),3.0f); 

				//right around the step point, there should be some backwards force (as the feet hit the ground) 

				float Shift = -0.1f; 


				float SecondaryStepMultiplier = glm::mix(0.2f, 0.0f, glm::clamp((Velocity - 1.0f) / 3.0f, 0.0f, 1.0f)); 
				float RotationMultiplier = glm::mix(-2.0f, -0.3f, glm::clamp((Velocity - 1.0f) / 3.0f, 0.0f, 1.0f)); 

				float SecondaryStep = SecondaryStepMultiplier * pow(sin(Sequence + Shift), 2.0) / (1.0 + 4.0*abs(Sequence + Shift - 1.57));



				return RotationMultiplier *(PrimaryStepSine - SecondaryStep);
			}; 

			auto GetPositionFromSequence = [](float Sequence, float Velocity) {
				
				float XPositioned = pow(Sequence, 1.1) * 3.1415926535; 
				float YSine = sin(Sequence * 6.28+1.57); 
				if (YSine > 0.0f)
					YSine *= 0.3f; 

				float PositionMultiplier = glm::mix(0.025f, 0.08f, glm::clamp((Velocity - 1.0f) / 6.0f, 0.0f, 1.0f));


				return PositionMultiplier * Vector2f(sin(XPositioned), -YSine);

			}; 


			float Rotation = (RightLeg ? 1.0 : -1.0f) * GetRotationFromSequence(SequenceSine, VelocityLength);
			Vector2f Position = (RightLeg ? Vector2f(1.0) : Vector2f(-1.0, 1.0)) * GetPositionFromSequence(AnimationSequence, VelocityLength);
			



			Matrix4f Matrix = Matrix4f(1.0); 
			Matrix = glm::rotate(Matrix, glm::radians(Camera.Rotation.x), { 1,0,0 }); 
			Matrix = glm::rotate(Matrix, glm::radians(Rotation), { 0,0,1 }); 
			Matrix = glm::translate(Matrix, Vector3f(Position, 0.0f)); 
			Matrix = glm::rotate(Matrix, glm::radians(-Camera.Rotation.x), { 1,0,0 });

			return Matrix; 

		}
	}

}