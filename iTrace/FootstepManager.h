#pragma once

#include "Sound.h"

namespace iTrace {

	namespace Sound {

		struct FootStepManager {

			SoundInstance* ActiveInstance = nullptr; 
			SoundType ActiveType = SoundType::NONE; 
			int RandomSound = 0; 
			float ElapsedTime = 0; 
			float Length = 1.0; 

			void PrepareFootStepManager(SoundHandler& SoundManager);
			
			void SetStepLength(float Length); 
			void SetActiveMaterial(SoundType TYPE, SoundHandler& SoundManager);
			void Step();
			bool Poll(Camera & Camera, SoundHandler& SoundManager, Window & Window);

		};

	}

}
