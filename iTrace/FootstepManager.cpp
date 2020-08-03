#include "FootstepManager.h"

namespace iTrace {

	namespace Sound {



		void FootStepManager::PrepareFootStepManager(SoundHandler& SoundManager)
		{
			//load the sounds and create the respective instances 

			for (auto& Instance : SoundMaterialTypes) {

				std::string Path = "Sound/Footsteps/" + Instance.ExtensionName + "/step.wav"; 

				SoundManager.LoadSound(Instance.ExtensionName + "_step", Path); 
				SoundManager.AddSoundInstance(SoundInstance(), Instance.ExtensionName + "_step_instance", Instance.ExtensionName + "_step"); 
				SoundManager.Instances[Instance.ExtensionName + "_step_instance"].SetVolume(0.2); 
			}
			
		}

		void FootStepManager::SetStepLength(float Length)
		{
			this->Length = Length; 
		}

		void FootStepManager::SetActiveMaterial(SoundType TYPE, SoundHandler& SoundManager)
		{
			
			if (TYPE == SoundType::NONE) {
				ActiveInstance->Stop();
				ActiveInstance = nullptr;
				return; 
			}

			if (TYPE == ActiveType)
				return; 

			if (ActiveInstance != nullptr) {
				ActiveInstance->Stop(); 
			}

			ActiveType = TYPE; 
			
			std::string InstanceName = SoundMaterialTypes[static_cast<int>(ActiveType)].ExtensionName + "_step_instance"; 

			if (SoundManager.Instances.find(InstanceName) == SoundManager.Instances.end())
				return; 

			ActiveInstance = &SoundManager.Instances[InstanceName]; 

			//std::cout << "Set active instance to: " << InstanceName << '\n'; 

		}

		void FootStepManager::Step()
		{
			
			if (ActiveInstance == nullptr)
				return;

			if (ElapsedTime <= Length && ActiveInstance->IsPlaying())
				return; 

			RandomSound = (RandomSound + 1 + (rand() % 3)) % 4; 

			//std::cout << RandomSound << '\n'; 

			ActiveInstance->SetPlayingOffset(float(RandomSound)); 
			ActiveInstance->Play(); 
			
		}

		bool FootStepManager::Poll(Camera& Camera, SoundHandler& SoundManager, Window& Window)
		{

			if (ActiveInstance == nullptr)
				return false;

			if (!ActiveInstance->IsPlaying())
				return false; 

			ElapsedTime += Window.GetFrameTime(); 

			ActiveInstance->SetOrigin(Camera.Position - Vector3f(0.0, 1.0, 0.0)); 

			if (ElapsedTime > Length) {
				ActiveInstance->Stop();
				ElapsedTime = 0.0; 
				return true; 
			}

			return false;


		}

	}

}