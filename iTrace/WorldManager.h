#pragma once

#include "Chunk.h"


namespace iTrace {

	namespace Rendering {

		struct WorldManager {
			
#define CHUNK_RENDER_DISTANCE 1 //likely NOT going to change, EVER. 
		
			Chunk::Chunk * Chunk = nullptr; //will be replaced with some kind of container when more chunks are allowed 

			Chunk::Chunk* Chunks[3][3]; //<- these are not ownership chunks, they are just handled by the rendering. 
			Chunk::Chunk* InternalChunks[CHUNK_SIZE * 2 + 3][CHUNK_SIZE * 2 + 3]; //<- these are the ownership chunks 

			uint64_t CenterX, CenterY; 
			int BiasX, BiasY; 
			int CurrentShiftX = 0, CurrentShiftY = 0; 

			unsigned int ChunkContainer; 

			void PrepareWorldManger(); 
			void GenerateWorld(Camera& Camera); 
			void RenderWorld(Camera& Camera, Shader & RenderToShader);
			void RenderWorldTransparent(Camera& Camera, Shader& RenderToShader); 

			void AddLightBlock(Vector3i Location, Vector3i & OriginLocation, std::vector<unsigned char> &Data, int Distance); 
			void AddLightSource(Vector3i Location, Vector3f Color); 
			void SpreadSkyLight(Vector2i Coordinate); 

			void FindLightSources(Vector3i Location, Vector3i& OriginLocation, Vector3i & ThisBlockPos, std::vector<bool>& Data, std::vector<Vector4i>& FoundSources, int Distance); 
			void UpdateBlockThenLighting(Vector3i LocationBlock, int NewBlockType); 


			bool CastBlock(Camera Camera, Chunk::BLOCK_ACTION Action, unsigned short Distance, unsigned char Block);


			//probably one of the hardest functions I'm every going to write
			void UpdateChunkTexture(Vector3i Min, Vector3i Max); 



		};

	}

}