#include "SoundEffectManager.h"

namespace iTrace {

	namespace Sound {

		void SoundEffect::Create(SoundHandler& SoundManager)
		{


			if (Name == "-" || Directory == "-")
				return; 

			std::string SoundPath = "Sound/" + Directory + "/" + Name + "/" + Directory + ".wav"; 
			std::string SoundBufferID = Directory + "_" + Name; 
			std::string SoundInstanceID = Directory + "_" + Name + "_instance_" + std::to_string(rand()); //<- can cause problems! 

			std::cout << "Loading sound effect: " << SoundPath << '\n'; 

			if (SoundManager.Buffers.find(SoundBufferID) == SoundManager.Buffers.end()) {
				SoundManager.LoadSound(SoundBufferID, SoundPath); 
			}

			SoundManager.AddSoundInstance(SoundInstance(Origin, 1.0, Ambience), SoundInstanceID, SoundBufferID);

			InstancePtr = &SoundManager.Instances[SoundInstanceID]; 
			InstancePtr->SetVolume(100.0); 

			if (EffectType == SoundEffectType::LOOP) {
				InstancePtr->SetLoop(true); 
				InstancePtr->Play();
				if (InstancePtr->IsPlaying())
					std::cout << "Playing!\n"; 
			}


		}

		void SoundEffect::Poll(Window& Window, Camera& Camera)
		{
			if (InstancePtr == nullptr)
				return; 

			if (EffectType == SoundEffectType::SPLIT && InstancePtr->IsPlaying()) {

				CurrentSplitDuration += Window.GetTimeOpened(); 

				if (CurrentSplitDuration > SplitLength) {
					CurrentSplitDuration = 0.0; 
					InstancePtr->Stop(); 
				}

			}

			if (Ambience && InstancePtr->IsPlaying()) {
				
			//	std::cout << "Setting origin for ambience!\n"; 
				InstancePtr->SetOrigin(Camera.Position); 
			}


		}

		void SoundEffect::Play(int Split)
		{
			if (InstancePtr == nullptr)
				return;

			if (EffectType == SoundEffectType::SINGLE && !InstancePtr->IsPlaying())
				InstancePtr->Play(); 
			else if (EffectType == SoundEffectType::SPLIT && !InstancePtr->IsPlaying()) {

				if (Split >= Splits || Split < 0)
					return; 

				InstancePtr->Play();
				InstancePtr->SetPlayingOffset(SplitLength * Split); 
				
			}

		}

		void SoundEffect::Stop()
		{
			if (InstancePtr == nullptr)
				return;

			if (EffectType != SoundEffectType::LOOP && (InstancePtr->IsPlaying() || InstancePtr->IsPaused()))
				InstancePtr->Stop(); 

		}

		void SoundEffect::SetVolume(float Volume)
		{
			if (InstancePtr == nullptr)
				return;

			return InstancePtr->SetVolume(Volume); 

		}

		float SoundEffect::GetVolume()
		{
			if (InstancePtr == nullptr)
				return 0.0;

			return InstancePtr->GetVolume(); 
			
		}

		SoundEffect& SoundEffectManager::GetSoundEffect(std::string Name)
		{
			return SoundEffects[Name]; 
		}

		void SoundEffectManager::AddSoundEffect(SoundEffect Effect, SoundHandler& SoundManager)
		{
			SoundEffects[Effect.Name] = Effect; 
			SoundEffects[Effect.Name].Create(SoundManager); 

		}

		void SoundEffectManager::PollSoundEffects(SoundHandler& SoundManager, Window & Window, Camera& Camera)
		{
			for (auto& Effect : SoundEffects)
				Effect.second.Poll(Window, Camera); 
		}

	}

}