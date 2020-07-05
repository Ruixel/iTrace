#pragma once

#include <SFML/Audio.hpp>
#include <iostream>
#include <SFML/al.h>
#include <SFML/efx.h>
#include "WorldManager.h"

namespace iTrace {

	namespace Sound {

		struct ReverbParameters {

			float DecayTime;
			float Density;
			float Diffusion;
			float Gain;
			float GainHF;
			float DecayHFRatio;
			float ReflectionsGain;
			float ReflectionsDelay;
			float LateReverbGain;
			float LateReverbDelay;
			float AirAbsorptionGainHF;
			float RoomRolloffFactor;

			static ReverbParameters getReverb0()
			{
				ReverbParameters r = ReverbParameters();
				r.DecayTime = 0.15f;
				r.Density = 0.0f;
				r.Diffusion = 1.0f;
				r.Gain = 0.2f * 0.85f;
				r.GainHF = 0.99f;
				r.DecayHFRatio = 0.6;
				r.ReflectionsGain = 2.5f;
				r.ReflectionsDelay = 0.001f;
				r.LateReverbGain = 1.26f;
				r.LateReverbDelay = 0.011f;
				r.AirAbsorptionGainHF = 0.994f;
				r.RoomRolloffFactor = 0.16f;

				return r;
			}

			static ReverbParameters getReverb1()
			{
				ReverbParameters r = ReverbParameters();
				r.DecayTime = 0.55f;
				r.Density = 0.0f;
				r.Diffusion = 1.0f;
				r.Gain = 0.3f * 0.85f;
				r.GainHF = 0.99f;
				r.DecayHFRatio = 0.7f;
				r.ReflectionsGain = 0.2f;
				r.ReflectionsDelay = 0.015f;
				r.LateReverbGain = 1.26f;
				r.LateReverbDelay = 0.011f;
				r.AirAbsorptionGainHF = 0.994f;
				r.RoomRolloffFactor = 0.15f;

				return r;
			}

			static ReverbParameters getReverb2()
			{
				ReverbParameters r = ReverbParameters();
				r.DecayTime = 1.68f;
				r.Density = 0.1f;
				r.Diffusion = 1.0f;
				r.Gain = 0.5f * 0.85f;
				r.GainHF = 0.99f;
				r.DecayHFRatio = 0.7f;
				r.ReflectionsGain = 0.0f;
				r.ReflectionsDelay = 0.021f;
				r.LateReverbGain = 1.26f;
				r.LateReverbDelay = 0.021f;
				r.AirAbsorptionGainHF = 0.994f;
				r.RoomRolloffFactor = 0.13f;

				return r;
			}

			static ReverbParameters getReverb3()
			{
				ReverbParameters r = ReverbParameters();
				r.DecayTime = 4.142f;
				r.Density = 0.5f;
				r.Diffusion = 1.0f;
				r.Gain = 0.4f;
				r.GainHF = 0.89f;
				r.DecayHFRatio = 0.7f;
				r.ReflectionsGain = 0.0f;
				r.ReflectionsDelay = 0.025f;
				r.LateReverbGain = 1.26f;
				r.LateReverbDelay = 0.021f;
				r.AirAbsorptionGainHF = 0.994f;
				r.RoomRolloffFactor = 0.11f;

				return r;
			}

		};


		struct SoundBuffer : sf::SoundBuffer {
			unsigned int GetBufferID() {
				return m_buffer; 
			}
		};

		struct SoundInstance {

			Vector3f Origin;
			float Volume;
			int SourceID;

			SoundInstance(Vector3f Origin, float Volume) : Origin(Origin), Volume(Volume), SourceID(0) {}
			SoundInstance() : Origin(Vector3f(0.0, 0.0, 0.0)), Volume(0.0), SourceID(0) {}
	
		};

		struct SoundHandler {

			//OpenAL extensions: 

			std::map<std::string, SoundBuffer> Buffers;
			std::map<std::string, SoundInstance> Instances;


			unsigned int Slots[4], Reverbs[4], Filters[4];
			unsigned int DirectFilter;
			unsigned int DirectData; //does the direct data things 



			ALCdevice* Device;


			LPALGENFILTERS alGenFilters;
			LPALFILTERI alFilteri;
			LPALFILTERF alFilterf;
			LPALGENEFFECTS alGenEffects;
			LPALEFFECTI alEffecti;
			LPALEFFECTF alEffectf;
			LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
			LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;

			void GenerateOpenALExtensions();
			void SetupEFX();
			void SetReverbParameters(ReverbParameters r, int AuxFXSlot, int ReverbSlot);
			void SetEnvironment(int SourceID, float SendGain0, float SendGain1, float SendGain2, float SendGain3, float SendCutoff0, float SendCutoff1, float SendCutoff2, float SendCutoff3, float DirectCutoff, float DirectGain);
			void LoadSound(std::string SoundID, std::string FilePath);
			void AddSoundInstance(SoundInstance Instance, std::string ParentName, std::string SoundID);
			void SetSoundInstanceOrigin(std::string Key, Vector3f Origin);
			void Update(Camera& Camera, Window& Window, Rendering::WorldManager & World);
		};
		

	}

}