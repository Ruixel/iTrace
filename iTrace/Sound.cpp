#include "Sound.h"
#include <fstream>

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
				throw std::exception("EFX Extension not found on current device.");
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
		void SoundHandler::SetEnvironmentAmbience(int SourceID, float DirectCutoff, float DirectGain)
		{
			alFilterf(DirectFilter, AL_LOWPASS_GAIN, DirectGain);
			checkErrorLog("Error whilst setting aiFilterF (AI_LOWPASS_GAIN,direct) ");

			alFilterf(DirectFilter, AL_LOWPASS_GAINHF, DirectCutoff);
			checkErrorLog("Error whilst setting aiFilterF (AL_LOWPASS_GAINHF,direct) ");

			alSourcei(SourceID, AL_DIRECT_FILTER, DirectFilter);
			checkErrorLog("Error whilst setting aiSource3i (AL_DIRECT_FILTER,direct) ");

			alSourcef(SourceID, AL_AIR_ABSORPTION_FACTOR, 1.0);
			checkErrorLog("Error whilst setting alSourcef (AL_AIR_ABSORPTION_FACTOR) ");
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


			//alSourcei(SourceID, AL_DIRECT_FILTER, DirectFilter);
			//checkErrorLog("Error whilst setting aiSource3i (AL_DIRECT_FILTER,direct) ");

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

			std::ifstream f(FilePath);

			if (!f.good())
				std::cout << "File: " << FilePath << " not found!\n";


			unsigned int Buffer;
			Buffer = alutCreateBufferFromFile(FilePath.c_str());
			Buffers[SoundID] = Buffer;
		}
		void SoundHandler::AddSoundInstance(SoundInstance Instance, std::string ParentName, std::string SoundID)
		{



			if (Buffers.find(SoundID) == Buffers.end())
				return;

			auto Buffer = Buffers[SoundID];

			unsigned int Source;

			alGenSources(1, &Source);
			alSourcei(Source, AL_BUFFER, Buffers[SoundID]);
			alSource3f(Source, AL_POSITION, Instance.Origin.x, Instance.Origin.y, Instance.Origin.z);
			alSource3f(Source, AL_VELOCITY, 0, 0, 0);
			alSourcef(Source, AL_PITCH, 1);
			alSourcef(Source, AL_GAIN, 100.0);

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
		bool ActivateRT = false;
		void SoundHandler::Update(Camera& Camera, Window& Window, Rendering::WorldManager& World)
		{

			if (Instances.size() == 0)
				return;

			int PlayingInstances = 0;

			for (auto& Instance : Instances) {
				if (Instance.second.IsPlaying() && Instance.second.GetVolume() > 0.0) {
					PlayingInstances++;
				}
			}

			if (PlayingInstances == 0)
				return;

			auto m = glm::value_ptr(Camera.View);

			Vector3f position = Vector3f(m[12], m[13], m[14]);  // world space position
			Vector3f up = Vector3f(m[4], m[5], m[6]);     // world space up vector
			Vector3f front = Vector3f(-m[8], -m[9], -m[10]); // vector facing forward from camera/listener position

			Vector3f UpVector = Vector3f(0.0, 1.0, 0.0) * Matrix3f(Camera.View);
			Vector3f FrontVector = Vector3f(0.0, 0.0, -1.0) * Matrix3f(Camera.View);

			ALfloat listenerOri[] = { FrontVector.x, FrontVector.y, FrontVector.z, UpVector.x, UpVector.y, UpVector.z };

			alListener3f(AL_POSITION, Camera.Position.x, Camera.Position.y, Camera.Position.z);
			alListener3f(AL_VELOCITY, 0, 0, 0);
			alListenerfv(AL_ORIENTATION, listenerOri);

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::B))
				ActivateRT = true;

			if (!ActivateRT)
				return;

			//HW accelerated sound-tracing! 

			float* DirectDataPixels = new float[MAX_OBJECTS * 4];
			for (int i = 0; i < MAX_OBJECTS * 4; i++)
				DirectDataPixels[i] = 0.0;

			int ObjectID = 0;

			for (auto& Instance : Instances) {

				if (Instance.second.IsPlaying() && Instance.second.GetVolume() > 0.0 && !Instance.second.Ambience) {

					DirectDataPixels[ObjectID * 4] = Instance.second.Origin.x;
					DirectDataPixels[ObjectID * 4 + 1] = Instance.second.Origin.y;
					DirectDataPixels[ObjectID * 4 + 2] = Instance.second.Origin.z;

					ObjectID++;

				}

			}

			glBindTexture(GL_TEXTURE_2D, DirectData);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, MAX_OBJECTS, 1, 0, GL_RGBA, GL_FLOAT, DirectDataPixels);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);

			float* GainsShared = new float[3 * NUM_RAYS * (MAX_OBJECTS + 1)];
			float* ReflectivityRatios = new float[3 * NUM_RAYS * (MAX_OBJECTS + 1)];
			float* TotalOcclusion = new float[MAX_OBJECTS];




			SecondarySoundTracingBuffer.Bind();

			SecondarySoundTracingShader.Bind();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, DirectData);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_3D, World.Chunk->ChunkTexID);

			SecondarySoundTracingShader.SetUniform("PlayerPosition", Camera.Position);

			Rendering::DrawPostProcessQuad();

			SecondarySoundTracingShader.UnBind();

			SecondarySoundTracingBuffer.UnBind(Window);

			PrimarySoundTracingBuffer.Bind();

			PrimarySoundTracingShader.Bind();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, DirectData);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_3D, World.Chunk->ChunkTexID);

			PrimarySoundTracingShader.SetUniform("PlayerPosition", Camera.Position);

			Rendering::DrawPostProcessQuad();

			PrimarySoundTracingShader.UnBind();

			PrimarySoundTracingBuffer.UnBind(Window);


			glFinish();

			glBindTexture(GL_TEXTURE_2D, SecondarySoundTracingBuffer.ColorBuffers[0]);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, GainsShared);
			glBindTexture(GL_TEXTURE_2D, SecondarySoundTracingBuffer.ColorBuffers[1]);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, ReflectivityRatios);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindTexture(GL_TEXTURE_2D, PrimarySoundTracingBuffer.ColorBuffer);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, TotalOcclusion);
			glBindTexture(GL_TEXTURE_2D, 0);

			glFinish();

			//TODO: Push to secondary sound thread! 

			int x = 0;

			float PrecomputedHemisphericalGain = -1.0f, PrecomputedHemisphericalCutoff = -1.0f;

			for (auto& Instance : Instances) {

				if (Instance.second.IsPlaying() && Instance.second.GetVolume() > 0.0 && !Instance.second.Ambience) {

					float SendGain0 = 0.0, SendGain1 = 0.0, SendGain2 = 0.0, SendGain3 = 0.0;
					float BounceReflectivityRatios[4] = { 0.0,0.0,0.0,0.0 };
					float SharedAirSpace = 0.0;
					float OcclusionAccumulation = TotalOcclusion[x];
					float rcpTotalRays = 1.0 / float(NUM_RAYS * MAX_BOUNCES);
					float absorptionCoeff = 3.0f;

					for (int y = 0; y < NUM_RAYS; y++) {

						int BasePixel = (y * (MAX_OBJECTS + 1) + x) * 3;

						Vector3f VGainsShared = Vector3f(GainsShared[BasePixel], GainsShared[BasePixel + 1], GainsShared[BasePixel + 2]);
						Vector3f VReflectivityRatios = Vector3f(ReflectivityRatios[BasePixel], ReflectivityRatios[BasePixel + 1], ReflectivityRatios[BasePixel + 2]);

						Vector2f SendGain01 = glm::unpackHalf2x16(glm::floatBitsToUint(VGainsShared.x));
						Vector2f SendGain23 = glm::unpackHalf2x16(glm::floatBitsToUint(VGainsShared.y));
						Vector2f ReflRatios01 = glm::unpackHalf2x16(glm::floatBitsToUint(VReflectivityRatios.x));
						Vector2f ReflRatios23 = glm::unpackHalf2x16(glm::floatBitsToUint(VReflectivityRatios.y));

						SendGain0 += SendGain01.x;
						SendGain1 += SendGain01.y;
						SendGain2 += SendGain23.x;
						SendGain3 += SendGain23.y;

						SharedAirSpace += VGainsShared.z;

						BounceReflectivityRatios[0] += ReflRatios01.x;
						BounceReflectivityRatios[1] += ReflRatios01.y;
						BounceReflectivityRatios[2] += ReflRatios23.x;
						BounceReflectivityRatios[3] += ReflRatios23.y;

					}

					float directCutoff = (float)exp(-OcclusionAccumulation * absorptionCoeff);
					float directGain = (float)pow(directCutoff, 0.1);

					SharedAirSpace *= 64.0;
					SharedAirSpace *= rcpTotalRays;

					for (int i = 0; i < 4; i++)
						BounceReflectivityRatios[i] = BounceReflectivityRatios[i] / float(NUM_RAYS);

					float sharedAirspaceWeight0 = glm::clamp(SharedAirSpace / 20.0f, 0.0f, 1.0f);
					float sharedAirspaceWeight1 = glm::clamp(SharedAirSpace / 15.0f, 0.0f, 1.0f);
					float sharedAirspaceWeight2 = glm::clamp(SharedAirSpace / 10.0f, 0.0f, 1.0f);
					float sharedAirspaceWeight3 = glm::clamp(SharedAirSpace / 10.0f, 0.0f, 1.0f);

					float averageSharedAirspace = (sharedAirspaceWeight0 + sharedAirspaceWeight1 + sharedAirspaceWeight2 + sharedAirspaceWeight3) * 0.25f;

					directCutoff = (float)glm::max((float)pow(averageSharedAirspace, 0.5) * 0.2f, directCutoff);

					directGain = (float)pow(directCutoff, 0.1);


					float sendCutoff0 = (float)exp(-OcclusionAccumulation * absorptionCoeff * 1.0f) * (1.0f - sharedAirspaceWeight0) + sharedAirspaceWeight0;
					float sendCutoff1 = (float)exp(-OcclusionAccumulation * absorptionCoeff * 1.0f) * (1.0f - sharedAirspaceWeight1) + sharedAirspaceWeight1;
					float sendCutoff2 = (float)exp(-OcclusionAccumulation * absorptionCoeff * 1.5f) * (1.0f - sharedAirspaceWeight2) + sharedAirspaceWeight2;
					float sendCutoff3 = (float)exp(-OcclusionAccumulation * absorptionCoeff * 1.5f) * (1.0f - sharedAirspaceWeight3) + sharedAirspaceWeight3;



					SendGain1 *= (float)pow(BounceReflectivityRatios[1], 1.0);
					//sendGain1 += sendGain2 * (1.0f - (float)Math.pow(bounceReflectivityRatio[2], 3.0)) * 0.5f; 
					//sendGain1 += sendGain3 * (1.0f - (float)Math.pow(bounceReflectivityRatio[3], 4.0)) * 0.5f;
					SendGain2 *= (float)pow(BounceReflectivityRatios[2], 3.0);
					SendGain3 *= (float)pow(BounceReflectivityRatios[3], 4.0);

					SendGain0 = glm::clamp(SendGain0, 0.0f, 1.0f);
					SendGain1 = glm::clamp(SendGain1, 0.0f, 1.0f);
					SendGain2 = glm::clamp(SendGain2 * 1.05f - .05f, 0.0f, 1.0f);
					SendGain3 = glm::clamp(SendGain3 * 1.05f - .05f, 0.0f, 1.0f);

					SendGain0 *= (float)pow(sendCutoff0, 0.1);
					SendGain1 *= (float)pow(sendCutoff1, 0.1);
					SendGain2 *= (float)pow(sendCutoff2, 0.1);
					SendGain3 *= (float)pow(sendCutoff3, 0.1);

					OcclusionAccumulation = OcclusionAccumulation * OcclusionAccumulation;
					OcclusionAccumulation = OcclusionAccumulation * 4.0;
					
					SetEnvironment(Instance.second.SourceID, SendGain0, SendGain1, SendGain2, SendGain3, sendCutoff0, sendCutoff1, sendCutoff2, sendCutoff3, directCutoff, directGain);
					x++;

				}
				else if (Instance.second.Ambience && Instance.second.IsPlaying() && Instance.second.GetVolume() > 0.0) {

					if (PrecomputedHemisphericalGain < -.1) {

						float OcclusionAccumulation = 0.0;
						float rcpTotalRays = 1.0 / float(NUM_RAYS);
						float absorptionCoeff = 3.0f;

						for (int y = 0; y < NUM_RAYS; y++) {

							int BasePixel = (y * (MAX_OBJECTS + 1) + (MAX_OBJECTS)) * 3;

							Vector3f VGainsShared = Vector3f(GainsShared[BasePixel], GainsShared[BasePixel + 1], GainsShared[BasePixel + 2]);

							OcclusionAccumulation += VGainsShared.z;

						}

						PrecomputedHemisphericalCutoff = (float)exp(-OcclusionAccumulation * absorptionCoeff * rcpTotalRays);
						PrecomputedHemisphericalGain = (float)pow(PrecomputedHemisphericalCutoff, 0.1);

					}

					SetEnvironmentAmbience(Instance.second.SourceID, PrecomputedHemisphericalCutoff, PrecomputedHemisphericalGain);

				}
			}

			delete[] GainsShared;
			delete[] ReflectivityRatios;
			delete[] TotalOcclusion;


		}
		void SoundHandler::PrepareSoundBlockData()
		{
			PrimarySoundTracingBuffer = Rendering::FrameBufferObject(Vector2i(MAX_OBJECTS, 1), GL_R32F, false, false);
			SecondarySoundTracingBuffer = Rendering::MultiPassFrameBufferObject(Vector2i(MAX_OBJECTS + 1, NUM_RAYS), 2, { GL_RGB32F, GL_RGB32F }, false, false);
			PrimarySoundTracingShader = Rendering::Shader("Shaders/PrimarySoundTracing");
			SecondarySoundTracingShader = Rendering::Shader("Shaders/SecondarySoundTracing");


			glGenTextures(1, &DirectData);

			PrimarySoundTracingShader.Bind();

			PrimarySoundTracingShader.SetUniform("SoundLocations", 0);
			PrimarySoundTracingShader.SetUniform("VoxelData", 1);

			PrimarySoundTracingShader.UnBind();


			SecondarySoundTracingShader.Bind();

			SecondarySoundTracingShader.SetUniform("NumRays", NUM_RAYS);
			SecondarySoundTracingShader.SetUniform("SoundLocations", 0);
			SecondarySoundTracingShader.SetUniform("VoxelData", 1);
			SecondarySoundTracingShader.SetUniform("MaxSounds", MAX_OBJECTS);

			SecondarySoundTracingShader.UnBind();




			std::vector<float> BlockReflectivities;

			for (int i = 0; i < Rendering::Chunk::GetBlockSize(); i++) {

				auto& Block = Rendering::Chunk::GetBlock(i);

				BlockReflectivities.push_back(SoundMaterialTypes[static_cast<int>(Block.SoundMaterialType)].Reflectivity);

			}

			glGenTextures(1, &BlockDataImage);

			glBindTexture(GL_TEXTURE_1D, BlockDataImage);
			glTexImage1D(GL_TEXTURE_1D, 0, GL_R16F, BlockReflectivities.size(), 0, GL_RED, GL_FLOAT, BlockReflectivities.data());
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_1D, 0);

		}

		void SoundHandler::ReloadSounds()
		{

			PrimarySoundTracingShader.Reload("Shaders/PrimarySoundTracing");
			SecondarySoundTracingShader.Reload("Shaders/SecondarySoundTracing");

			PrimarySoundTracingShader.Bind();

			PrimarySoundTracingShader.SetUniform("SoundLocations", 0);
			PrimarySoundTracingShader.SetUniform("VoxelData", 1);

			PrimarySoundTracingShader.UnBind();


			SecondarySoundTracingShader.Bind();

			SecondarySoundTracingShader.SetUniform("NumRays", NUM_RAYS);
			SecondarySoundTracingShader.SetUniform("SoundLocations", 0);
			SecondarySoundTracingShader.SetUniform("VoxelData", 1);
			SecondarySoundTracingShader.SetUniform("MaxSounds", MAX_OBJECTS);

			SecondarySoundTracingShader.UnBind();
		}

		void SoundInstance::Play()
		{

			if (SourceID == -1)
				return;

			if (IsPlaying())
				return;

			//std::cout << "Plays the sound!\n"; 

			alSourcePlay(SourceID);

		}

		void SoundInstance::Stop()
		{
			if (SourceID == -1)
				return;

			if (!IsPlaying())
				return;

			alSourceStop(SourceID);

		}

		void SoundInstance::Pause()
		{
			if (SourceID == -1)
				return;

			if (!IsPlaying() || IsPaused())
				return;

			alSourcePause(SourceID);

		}

		void SoundInstance::SetLoop(bool Loop)
		{
			if (SourceID == -1)
				return;
			Looping = Loop;
			alSourcei(SourceID, AL_LOOPING, Loop ? AL_TRUE : AL_FALSE);
			//std::cout << "Setting loop data\n"; 
		}

		bool SoundInstance::IsLooped()
		{
			if (SourceID == -1)
				return false;

			return Looping;
		}

		bool SoundInstance::IsPlaying()
		{
			if (SourceID == -1)
				return false;

			int State;

			alGetSourcei(SourceID, AL_SOURCE_STATE, &State);

			return State == AL_PLAYING;
		}

		bool SoundInstance::IsPaused()
		{
			if (SourceID == -1)
				return false;

			int State;

			alGetSourcei(SourceID, AL_SOURCE_STATE, &State);

			return State == AL_PAUSED;

		}

		void SoundInstance::SetOrigin(Vector3f Origin)
		{
			if (SourceID == -1)
				return;
			this->Origin = Origin;
			alSource3f(SourceID, AL_POSITION, Origin.x, Origin.y, Origin.z);
		}

		void SoundInstance::SetPlayingOffset(float Time)
		{
			if (SourceID == -1)
				return;
			alSourcef(SourceID, AL_SEC_OFFSET, Time);
		}

		void SoundInstance::SetVolume(float Volume)
		{
			if (SourceID == -1)
				return;

			this->Volume = Volume;

			std::cout << "set volumne to: " << Volume << '\n';

			alSourcef(SourceID, AL_GAIN, Volume);

		}

		float SoundInstance::GetVolume()
		{
			if (SourceID == -1)
				return 0.f;

			float vol;

			alGetSourcef(SourceID, AL_GAIN, &vol);

			return vol;
		}

	}
}