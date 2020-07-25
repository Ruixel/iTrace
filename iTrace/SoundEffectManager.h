#pragma once
#include "Sound.h"


namespace iTrace {

	namespace Sound {




		enum class SoundEffectType {SPLIT, SINGLE, LOOP, NONE}; //if the sound file is a single sound effect or multiple split ones 
		//LOOP means that the sound effect is constantly playing, and the only property that can be changed is the volume 
		//this is useful for ambience sound effects, such as rain, forest sounds, etc etc 
		//do note that effects defined with either SPLIT or SINGLE cannot be looped! (especially not SPLIT) 
		
		struct SoundEffect {

			bool Ambience = false; //is the contribution hemispherical or positional 
			bool RayTraced = true; //should the sound effect use the HW accelerated ray-tracing for higher realism? 
			
			int Splits = -1; //<- reserved to effects that are SPLIT, defines the amount of splits in a sound file 
			float SplitLength = 0.0; //<- reserved to effects that are SPLIT, defines the length of each split 
			int CurrentSplit = 0; //<- the current effective split 
			float CurrentSplitDuration = 0.0; //the current duration of the split 

			
			SoundEffectType EffectType = SoundEffectType::NONE;
			std::string Name = "-", Directory = "-"; 
			Vector3f Origin; //<- not for ambience sounds 

			SoundInstance* InstancePtr = nullptr; //the pointer to the sound instance 
			SoundEffect() {}
			SoundEffect(std::string Name, std::string Directory, Vector3f Origin, SoundEffectType Type, bool Ambience, bool RayTraced, int Splits = -1, float SplitLength = 0.0) :
				Origin(Origin),Name(Name), Directory(Directory), EffectType(Type), Ambience(Ambience), RayTraced(RayTraced), Splits(Splits), SplitLength(SplitLength) {}
			void Create(SoundHandler& SoundManager); //<- assumes 
			void Poll(Window & Window, Camera& Camera); //<- required for sounds defined with SPLIT 
			void Play(int Split = -1); //<- does nothing if the sound is already playing, also cannot be used for sounds that are LOOP.
			void Stop(); //<- does nothing if the sound is not playing, or the sound type is LOOP 
			void SetVolume(float Volume = 1.0); 
			float GetVolume(); 

		};
 
		struct SoundEffectManager {

			std::map<std::string, SoundEffect> SoundEffects; 

			SoundEffect& GetSoundEffect(std::string Name); 
			void AddSoundEffect(SoundEffect Effect, SoundHandler & SoundManager); 
			void PollSoundEffects(SoundHandler & SoundManager, Window& Window, Camera& Camera);


		};
		

	}

}

