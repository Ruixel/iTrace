#include "Sound.h"

namespace iTrace {
	namespace Sound {

		bool checkErrorLog(std::string errorMessage)
		{
			int error = alGetError();
			std::string errorName;

			if (error != 0)
			{
				switch (error)
				{
				case AL_INVALID_NAME: 			errorName = "AL_INVALID_NAME"; break;
				case AL_INVALID_ENUM: 			errorName = "AL_INVALID_ENUM"; break;
				case AL_INVALID_VALUE: 		errorName = "AL_INVALID_VALUE"; break;
				case AL_INVALID_OPERATION: 	errorName = "AL_INVALID_OPERATION"; break;
				case AL_OUT_OF_MEMORY: 		errorName = "AL_OUT_OF_MEMORY"; break;
				default: errorName = std::to_string(error); break;
				}



				std::cout << "[SOUND ERROR] " << errorMessage << " OpenAL error " << errorName << '\n';
				std::cin.get();
				return true;
			}
			else
			{
				return false;
			}
		}

		void SoundHandler::GenerateOpenALExtensions()
		{
			alGenFilters = (LPALGENFILTERS)alGetProcAddress("alGenFilters");
			alFilteri = (LPALFILTERI)alGetProcAddress("alFilteri");
			alFilterf = (LPALFILTERF)alGetProcAddress("alFilterf");
			alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
			alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
			alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
			alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
			alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
		}
		void SoundHandler::SetupEFX()
		{
			ALCcontext* currentContext = alcGetCurrentContext();

			ALCdevice* currentDevice = alcGetContextsDevice(currentContext);

			if (alcIsExtensionPresent(currentDevice, "ALC_EXT_EFX"))
			{
				std::cout << "EFX Extension recognized.\n";
			}
			else
			{
				//throw std::exception("EFX Extension not found on current device.");
			}


			ALsizei max;
			alcGetIntegerv(currentDevice, ALC_MAX_AUXILIARY_SENDS, sizeof(max), &max);

			std::cout << "Max slots: " << max << '\n';

			for (int i = 0; i < 4; i++) {
				alGenAuxiliaryEffectSlots(1, &Slots[i]);
				checkErrorLog("Error when generating slot[" + std::to_string(i) + "] ");
			}
			alGenEffects(4, Reverbs);
			checkErrorLog("Error when generating Reverbs");
			alGenFilters(4, Filters);
			checkErrorLog("Error when generating Filters");
			alGenFilters(1, &DirectFilter);
			checkErrorLog("Error when creating direct filter");


			alFilteri(DirectFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);

			checkErrorLog("Error when setting direct filter type");


			for (int i = 0; i < 4; i++) {
				alAuxiliaryEffectSloti(Slots[i], AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, AL_TRUE);
				checkErrorLog("Error when setting AL_EFFECTSLOT_AUXILIARY_SEND_AUTO to AL_TRUE");
				alEffecti(Reverbs[i], AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
				checkErrorLog("Error when setting AL_EFFECT_TYPE to AL_EFFECT_EAXREVERB");
				alFilteri(Filters[i], AL_FILTER_TYPE, AL_FILTER_LOWPASS);
				checkErrorLog("Error when setting AL_FILTER_TYPE to AL_FILTER_LOWPASS");
			}

			SetReverbParameters(ReverbParameters::getReverb0(), Slots[0], Reverbs[0]);
			SetReverbParameters(ReverbParameters::getReverb1(), Slots[1], Reverbs[1]);
			SetReverbParameters(ReverbParameters::getReverb2(), Slots[2], Reverbs[2]);
			SetReverbParameters(ReverbParameters::getReverb3(), Slots[3], Reverbs[3]);
		}
		void SoundHandler::SetReverbParameters(ReverbParameters r, int AuxFXSlot, int ReverbSlot)
		{
			alEffectf(ReverbSlot, AL_EAXREVERB_DENSITY, r.Density);
			checkErrorLog("Error while assigning reverb density: " + std::to_string(r.Density));
			alEffectf(ReverbSlot, AL_EAXREVERB_DIFFUSION, r.Diffusion);
			checkErrorLog("Error while assigning reverb diffusion: " + std::to_string(r.Diffusion));
			alEffectf(ReverbSlot, AL_EAXREVERB_GAIN, r.Gain);
			checkErrorLog("Error while assigning reverb gain: " + std::to_string(r.Gain));
			alEffectf(ReverbSlot, AL_EAXREVERB_GAINHF, r.GainHF);
			checkErrorLog("Error while assigning reverb gainHF: " + std::to_string(r.GainHF));
			alEffectf(ReverbSlot, AL_EAXREVERB_DECAY_TIME, r.DecayTime);
			checkErrorLog("Error while assigning reverb decayTime: " + std::to_string(r.DecayTime));
			alEffectf(ReverbSlot, AL_EAXREVERB_DECAY_HFRATIO, r.DecayHFRatio);
			checkErrorLog("Error while assigning reverb decayHFRatio: " + std::to_string(r.DecayHFRatio));
			alEffectf(ReverbSlot, AL_EAXREVERB_REFLECTIONS_GAIN, r.ReflectionsGain);
			checkErrorLog("Error while assigning reverb reflectionsGain: " + std::to_string(r.ReflectionsGain));
			alEffectf(ReverbSlot, AL_EAXREVERB_LATE_REVERB_GAIN, r.LateReverbGain);
			checkErrorLog("Error while assigning reverb lateReverbGain: " + std::to_string(r.LateReverbGain));
			alEffectf(ReverbSlot, AL_EAXREVERB_LATE_REVERB_DELAY, r.LateReverbDelay);
			checkErrorLog("Error while assigning reverb lateReverbDelay: " + std::to_string(r.LateReverbDelay));
			alEffectf(ReverbSlot, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, r.AirAbsorptionGainHF);
			checkErrorLog("Error while assigning reverb airAbsorptionGainHF: " + std::to_string(r.AirAbsorptionGainHF));
			alEffectf(ReverbSlot, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, r.RoomRolloffFactor);
			checkErrorLog("Error while assigning reverb roomRolloffFactor: " + std::to_string(r.RoomRolloffFactor));
			alAuxiliaryEffectSloti(AuxFXSlot, AL_EFFECTSLOT_EFFECT, ReverbSlot);
			checkErrorLog("Error while assigning reverb density: " + std::to_string(r.Density));
		}
		void SoundHandler::SetEnvironment(int SourceID, float SendGain0, float SendGain1, float SendGain2, float SendGain3, float SendCutoff0, float SendCutoff1, float SendCutoff2, float SendCutoff3, float DirectCutoff, float DirectGain)
		{
			alFilterf(Filters[0], AL_LOWPASS_GAIN, SendGain0);
			checkErrorLog("Error whilst setting aiFilterF (AI_LOWPASS_GAIN,0) ");
			alFilterf(Filters[0], AL_LOWPASS_GAINHF, SendCutoff0);
			checkErrorLog("Error whilst setting aiFilterF (AL_LOWPASS_GAINHF,0) ");

			alSource3i(SourceID, AL_AUXILIARY_SEND_FILTER, Slots[0], 0, Filters[0]);
			checkErrorLog("Error whilst setting aiSource3i (AL_AUXILIARY_SEND_FILTER,0) ");


			alFilterf(Filters[1], AL_LOWPASS_GAIN, SendGain1);
			checkErrorLog("Error whilst setting aiFilterF (AI_LOWPASS_GAIN,1) ");

			alFilterf(Filters[1], AL_LOWPASS_GAINHF, SendCutoff1);
			checkErrorLog("Error whilst setting aiFilterF (AL_LOWPASS_GAINHF,0) ");

			alSource3i(SourceID, AL_AUXILIARY_SEND_FILTER, Slots[1], 1, Filters[1]);
			checkErrorLog("Error whilst setting aiSource3i (AL_AUXILIARY_SEND_FILTER,1) ");


			alFilterf(Filters[2], AL_LOWPASS_GAIN, SendGain2);
			checkErrorLog("Error whilst setting aiFilterF (AI_LOWPASS_GAIN,2) ");

			alFilterf(Filters[2], AL_LOWPASS_GAINHF, SendCutoff2);
			checkErrorLog("Error whilst setting aiFilterF (AL_LOWPASS_GAINHF,0) ");

			alSource3i(SourceID, AL_AUXILIARY_SEND_FILTER, Slots[2], 2, Filters[2]);
			checkErrorLog("Error whilst setting aiSource3i (AL_AUXILIARY_SEND_FILTER,2) (Slots:" + std::to_string(Slots[2]) + " Filters:" + std::to_string(Filters[2]) + ")");


			alFilterf(Filters[3], AL_LOWPASS_GAIN, SendGain3);
			checkErrorLog("Error whilst setting aiFilterF (AI_LOWPASS_GAIN,3) ");

			alFilterf(Filters[3], AL_LOWPASS_GAINHF, SendCutoff3);
			checkErrorLog("Error whilst setting aiFilterF (AL_LOWPASS_GAINHF,0) ");

			alSource3i(SourceID, AL_AUXILIARY_SEND_FILTER, Slots[3], 3, Filters[3]);
			checkErrorLog("Error whilst setting aiSource3i (AL_AUXILIARY_SEND_FILTER,3) (Slots:" + std::to_string(Slots[3]) + " Filters:" + std::to_string(Filters[3]) + ")");


			alFilterf(DirectFilter, AL_LOWPASS_GAIN, DirectGain);
			checkErrorLog("Error whilst setting aiFilterF (AI_LOWPASS_GAIN,direct) ");

			alFilterf(DirectFilter, AL_LOWPASS_GAINHF, DirectCutoff);
			checkErrorLog("Error whilst setting aiFilterF (AL_LOWPASS_GAINHF,direct) ");

			alSourcei(SourceID, AL_DIRECT_FILTER, DirectFilter);
			checkErrorLog("Error whilst setting aiSource3i (AL_DIRECT_FILTER,direct) ");

			alFilterf(DirectFilter, AL_LOWPASS_GAIN, DirectGain);
			checkErrorLog("Error whilst setting aiFilterF (AI_LOWPASS_GAIN,direct) ");

			alFilterf(DirectFilter, AL_LOWPASS_GAINHF, DirectCutoff);
			checkErrorLog("Error whilst setting aiFilterF (AL_LOWPASS_GAINHF,direct) ");

			alSourcei(SourceID, AL_DIRECT_FILTER, DirectFilter);
			checkErrorLog("Error whilst setting aiSource3i (AL_DIRECT_FILTER,direct) ");

			alSourcef(SourceID, AL_AIR_ABSORPTION_FACTOR, 1.0);
			checkErrorLog("Error whilst setting alSourcef (AL_AIR_ABSORPTION_FACTOR) ");
		}
		void SoundHandler::LoadSound(std::string SoundID, std::string FilePath)
		{
			Buffers[SoundID] = SoundBuffer(); 
			Buffers[SoundID].loadFromFile(FilePath); 
		}
		void SoundHandler::AddSoundInstance(SoundInstance Instance, std::string ParentName, std::string SoundID)
		{
			

			if (Buffers.find(SoundID) == Buffers.end())
				return;

			auto Buffer = Buffers[SoundID]; 

			unsigned int Source;

			alGenSources(1, &Source);
			alSourcei(Source, AL_BUFFER, Buffers[SoundID].GetBufferID());
			alSource3f(Source, AL_POSITION, Instance.Origin.x, Instance.Origin.y, Instance.Origin.z);
			alSource3f(Source, AL_VELOCITY, 0, 0, 0);
			alSourcef(Source, AL_PITCH, 1);
			alSourcef(Source, AL_GAIN, 100.0);
			alSourcei(Source, AL_LOOPING, AL_TRUE);

			alSourcePlay(Source);

			Instance.SourceID = Source;
			Instances[ParentName] = Instance;
		}
		void SoundHandler::SetSoundInstanceOrigin(std::string Key, Vector3f Origin)
		{
			if (Instances.find(Key) == Instances.end())
				return;

			Instances[Key].Origin = Origin;
			alSource3f(Instances[Key].SourceID, AL_POSITION, Origin.x, Origin.y, Origin.z);
		}
		void SoundHandler::Update(Camera& Camera, Window& Window, Rendering::WorldManager& World)
		{
		}
	}
}