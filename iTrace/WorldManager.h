#pragma once

#include "Chunk.h"


namespace iTrace {

	namespace Rendering {

		struct WorldManager {

		
			Chunk::Chunk * Chunk = nullptr; //will be replaced with some kind of container when more chunks are allowed 


			void PrepareWorldManger(); 
			void GenerateWorld(Camera& Camera); 
			void RenderWorld(Camera& Camera, Shader & RenderToShader);
			
			void AddLightBlock(Vector3i Location, Vector3i & OriginLocation, std::vector<unsigned char> &Data, int Distance); 
			void AddLightSource(Vector3i Location, Vector3f Color); 
			void SpreadSkyLight(Vector2i Coordinate); 

			void FindLightSources(Vector3i Location, Vector3i& OriginLocation, Vector3i & ThisBlockPos, std::vector<bool>& Data, std::vector<Vector4i>& FoundSources, int Distance); 
			void UpdateBlockThenLighting(Vector3i LocationBlock, int NewBlockType); 


			bool CastBlock(Camera Camera, Chunk::BLOCK_ACTION Action, unsigned short Distance, unsigned char Block);


		};

	}

}