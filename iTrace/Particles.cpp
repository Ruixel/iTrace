#include "Particles.h"
#include "BooleanCommands.h"

#include <iostream>
namespace iTrace {

	namespace Rendering {





		void ParticleSystem::PrepareParticleSystem(Window& Window)
		{

			ParticleSystemFBO = FrameBufferObject(Window.GetResolution(), GL_RGBA16F); 
			ParticleSystemShader = Shader("Shaders/ParticleShader"); 
			SetShaderUniforms(); 
			

			RainDropNormal = LoadTextureGL("Resources/BakeData/RainDrop_Normal.png"); 

			glGenTextures(1, &ParticleContainer); 

		}

		void ParticleSystem::AddParticle(Particle Particle)
		{

			Particles.push_back(Particle); 

		}

		void ParticleSystem::PollParticles(Window& Window, WorldManager& World)
		{

			for (auto& Particle : Particles) {
				if(!GetBoolean("freezetime"))
					Particle.Position.y -= Window.GetFrameTime() * 10.0; 

			}

			//assume constant speed :)

			auto DeleteStatement = [&](Particle Particle) {
			
				//for now: 

				//assume a particle is never travelling more than 1 block per frame 
				//todo: replace this with a velocity based method in the future to allow for lower framerates (<10) 

				Vector3i Floored = Particle.Position; 

				if (Chunk::GetBlock(World.GetBlock(Floored)).IsSolid)
					return true; 
				return false; 
			};

			Particles.erase(std::remove_if(Particles.begin(), Particles.end(), DeleteStatement), Particles.end()); 

			Resolution = ceil(sqrt(Particles.size())); 

			std::vector<Vector4f> ParticleData(Resolution* Resolution,Vector4f(-1.0));

			int x = 0; 
			for (auto& P : Particles)
				ParticleData[x++] = { P.Position, 0.07 };


			glBindTexture(GL_TEXTURE_2D, ParticleContainer); 
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, Resolution, Resolution, 0, GL_RGBA, GL_FLOAT, ParticleData.data()); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glBindTexture(GL_TEXTURE_2D, 0);


		}

		void ParticleSystem::DrawParticles(Window& Window, Camera& Camera)
		{

			//std::cout << Particles.size() << '\n'; 

			ParticleSystemFBO.Bind(); 
			ParticleSystemShader.Bind(); 

			ParticleSystemShader.SetUniform("Resolution", Resolution); 
			ParticleSystemShader.SetUniform("ViewMatrix", Camera.View); 
			ParticleSystemShader.SetUniform("IdentityMatrix", Camera.Project * Camera.View); 
			ParticleSystemShader.SetUniform("CameraPosition", Camera.Position);

			glActiveTexture(GL_TEXTURE0); 
			glBindTexture(GL_TEXTURE_2D, ParticleContainer); 

			RainDropNormal.Bind(1); 

			DrawPostProcessQuadInstanced(Particles.size()); 

			ParticleSystemShader.UnBind(); 
			ParticleSystemFBO.UnBind();

		}

		void ParticleSystem::ReloadParticles()
		{
			ParticleSystemShader.Reload("Shaders/ParticleShader");
			SetShaderUniforms();
		}

		void ParticleSystem::SetShaderUniforms()
		{
			ParticleSystemShader.Bind();

			ParticleSystemShader.SetUniform("InstanceData", 0);
			ParticleSystemShader.SetUniform("RainNormalMap", 1);

			ParticleSystemShader.UnBind();
		}

	}

}