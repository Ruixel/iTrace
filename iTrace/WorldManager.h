#pragma once

#include "Chunk.h"
#include <queue>

namespace iTrace {

	namespace Rendering {

		struct WorldManager {
			
#define CHUNK_RENDER_DISTANCE 1 //likely NOT going to change, EVER. 
		
			Chunk::Chunk * Chunk = nullptr; //will be replaced with some kind of container when more chunks are allowed 


			std::array<std::array<std::unique_ptr<Chunk::Chunk>, CHUNK_RENDER_DISTANCE * 2 + 1>, CHUNK_RENDER_DISTANCE * 2 + 1> Chunks; 
			



			uint64_t CenterX, CenterY; 
			int BiasX, BiasY; 
			int CurrentShiftX = 0, CurrentShiftY = 0; 

			unsigned int ChunkContainer;
			unsigned int LightContainer; 

			void PrepareWorldManger(); 
			void GenerateWorld(Camera& Camera); 
			void RenderWorld(Camera& Camera, Shader & RenderToShader);
			void RenderWorldTransparent(Camera& Camera, Shader& RenderToShader); 

			void AddLightBlock(Vector3i Location, Vector3i & OriginLocation, std::vector<unsigned char> &Data, int Distance, Vector3i* Min = nullptr, Vector3i* Max = nullptr);
			void AddLightSource(Vector3i Location, Vector3f Color, Vector3i* Min = nullptr, Vector3i* Max = nullptr);
			void SpreadSkyLight(Vector2i Coordinate); 

			void FindLightSources(Vector3i Location, Vector3i& OriginLocation, Vector3i & ThisBlockPos, std::vector<bool>& Data, std::vector<Vector4i>& FoundSources, int Distance); 
			void UpdateBlockThenLighting(Vector3i LocationBlock, int NewBlockType); 

			Vector3i TraceBlock(Vector3f Origin, Vector3f Direction, unsigned short Distance, int& Side, Vector2i & ChunkPos, int &TypeIdx); 

			int GetBlock(Vector3i Position, Vector2i * ChunkPos=nullptr); //<- returns -1 if block is outside of loaded chunks 
			bool SetBlock(Vector3i Position, unsigned char Type, bool Update = false); 


			bool CastBlock(Camera Camera, Chunk::BLOCK_ACTION Action, unsigned short Distance, unsigned char Block);


			//probably one of the hardest functions I'm every going to write
			void UpdateChunkTexture(Vector3i Min, Vector3i Max, bool OnlyLight = false); 

			void ManageCollision(Vector3f& Position, Vector3f& Acceleration, Vector3f& Velocity); 


		};

	}

}