#pragma once

#include "WorldManager.h"

namespace iTrace {
	
	namespace Rendering {

		struct Particle {
			//for now, just have a position and size  

			Vector3f Position; 
			float Size; 

		};

		struct ParticleSystem {
			Shader ParticleSystemShader;
			FrameBufferObject ParticleSystemFBO; 
			TextureGL RainDropNormal; 
			unsigned char Resolution; 

			std::vector<Particle> Particles; 

			unsigned int ParticleContainer; 

			//<-.->

			void PrepareParticleSystem(Window& Window); 
			void AddParticle(Particle Particle); 

			void PollParticles(Window& Window, WorldManager& World); //<- physics calculations
			void DrawParticles(Window& Window, Camera& Camera); 

		};


	}

}