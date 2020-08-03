#include "WorldManager.h"
#include <iostream>
#include "SoundReflectivity.h"

namespace iTrace {

	namespace Rendering {

		Vector3f GetBlockEmissiveColor(int BlockIdx) {

			auto& BlockType = Chunk::GetBlock(BlockIdx);

			Vector3f Color = Vector3f(0.0);

			for (int i = 0; i < 6; i++) {

				Color += Chunk::GetTextureData(BlockType.TexIds[i]).EmissiveAverage;

			}

			return (Color / 6.f) * BlockType.EmissiveStrength;

		}

		void WorldManager::PrepareWorldManger()
		{
			{
				Chunk::AddBlock(Chunk::BlockType());
				Chunk::AddBlock(Chunk::BlockType("Green glass", { 59 }, true, true, false, SoundType::STONE, 0.0, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("Red glass", { 60 }, true, true, false, SoundType::STONE, 0.0, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("blue glass", { 61 }, true, true, false, SoundType::STONE, 0.0, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("Jelly", { 54 }, true, true, false, SoundType::STONE, 0.0, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE)); //CC0, good 


				Chunk::AddBlock(Chunk::BlockType("Stone", { 0,0,0,0,0,0 }, true, false, false, SoundType::STONE)); //CC0, good
				Chunk::AddBlock(Chunk::BlockType("Dirt", { 1,1,1,1,1,1 }, true, false, false, SoundType::DIRT));  //CC0, good
				Chunk::AddBlock(Chunk::BlockType("Grass", { 3,3,3,3,2,1 }, true, false, false, SoundType::GRASS)); //CC0, good
				Chunk::AddBlock(Chunk::BlockType("Glowstone", { 4 }, true, false, false, SoundType::METAL, 30.0)); //CC0, okay
				Chunk::AddBlock(Chunk::BlockType("Tiles", { 5 }, true, false, false, SoundType::STONE)); //CC0, good.
				Chunk::AddBlock(Chunk::BlockType("Planks", { 6 }, true, false, false, SoundType::WOOD));  //CC0, okay
				Chunk::AddBlock(Chunk::BlockType("Leather", { 7 }, true, false, false, SoundType::FABRIC)); //CC0, good
				Chunk::AddBlock(Chunk::BlockType("Fabric", { 8 }, true, false, false, SoundType::METAL));  //CC0, good
				Chunk::AddBlock(Chunk::BlockType("Obsidian", { 9 }, true, false, false, SoundType::STONE)); //CC0, good texture
				Chunk::AddBlock(Chunk::BlockType("Concrete", { 10 }, true, false, false, SoundType::STONE)); //CC0, good
				Chunk::AddBlock(Chunk::BlockType("Green glowstone", { 11 }, true, false, false, SoundType::METAL, 30.0)); //CC0, good
				Chunk::AddBlock(Chunk::BlockType("Iron bars", { 12 }, true, false, false, SoundType::METAL, 30.0)); //CC0, meh
				Chunk::AddBlock(Chunk::BlockType("Lamp", { 13 }, true, false, false, SoundType::METAL, 30.0)); //CC0 (and a bit self made) good texture
				Chunk::AddBlock(Chunk::BlockType("Lantern", { 14 }, true, false, false, SoundType::METAL, 180.0)); //CC0, good
				Chunk::AddBlock(Chunk::BlockType("red tiles", { 15 }, true, false, false, SoundType::STONE)); //CC0, and a bit self-made, good
				Chunk::AddBlock(Chunk::BlockType("green tiles", { 16 }, true, false, false, SoundType::STONE)); //CC0, and a bit self-made, good
				Chunk::AddBlock(Chunk::BlockType("orange tiles", { 17 }, true, false, false, SoundType::STONE)); //CC0, good texture
				Chunk::AddBlock(Chunk::BlockType("iron block", { 18 }, true, false, false, SoundType::METAL)); //CC0, good texture
				Chunk::AddBlock(Chunk::BlockType("solar panel", { 19 }, true, false, false, SoundType::METAL)); //CC0, good texture
				Chunk::AddBlock(Chunk::BlockType("sand", { 20 }, true, false, false, SoundType::SAND)); //CC0, good texture
				Chunk::AddBlock(Chunk::BlockType("marble", { 21 }, true, false, false, SoundType::STONE)); //CC0 (and a bit self made), semi good
				Chunk::AddBlock(Chunk::BlockType("iron ore", { 22 }, true, false, false, SoundType::STONE)); //CC0 (and a bit self made), good
				Chunk::AddBlock(Chunk::BlockType("Farmland", { 34,34,34,34,23,34 }, true, false, false, SoundType::DIRT)); //CC0 (and a bit self made), good
				Chunk::AddBlock(Chunk::BlockType("brick", { 24 }, true, false, false, SoundType::STONE)); //CC0, meh
				Chunk::AddBlock(Chunk::BlockType("forest ground", { 37,37,37,37,25,1 }, true, false, false, SoundType::DIRT)); //CC0, good
				Chunk::AddBlock(Chunk::BlockType("nether bricks", { 26 }, true, false, false, SoundType::STONE)); //non-CC0
				Chunk::AddBlock(Chunk::BlockType("log", { 27,27,27,27,28,28 }, true, false, false, SoundType::WOOD)); //CC0 (and a bit self made), good
				Chunk::AddBlock(Chunk::BlockType("wet farmland", { 34,34,34,34,29,34 }, true, false, false, SoundType::DIRT)); //CC0 (and a bit self made), good
				Chunk::AddBlock(Chunk::BlockType("netherrack", { 30 }, true, false, false, SoundType::STONE)); //CC0, okay+
				Chunk::AddBlock(Chunk::BlockType("cobblestone", { 31 }, true, false, false, SoundType::STONE)); //CC0, okay
				Chunk::AddBlock(Chunk::BlockType("snow", { 33,33,33,33,32,1 }, true, false, false, SoundType::SNOW)); //CC0 (and a bit self made), good
				Chunk::AddBlock(Chunk::BlockType("ice", { 35 }, true, false, false, SoundType::STONE)); //CC0 (and a bit self made), semi good
				Chunk::AddBlock(Chunk::BlockType("lava", { 36 }, true, false, false, SoundType::STONE, 30.0)); //CC0 (and a bit self made), semi good
				Chunk::AddBlock(Chunk::BlockType("stone workbench", { 38,38,38,38,39,40 }, true, false, false, SoundType::WOOD)); //CC0 (and a bit self made), good
				Chunk::AddBlock(Chunk::BlockType("forge", { 41,41,41,42,43,44 }, true, false, false, SoundType::STONE)); //CC0 (and a bit self made), good
				Chunk::AddBlock(Chunk::BlockType("coal block", { 45 }, true, false, false, SoundType::STONE)); //CC0 (and a bit self made), good
				Chunk::AddBlock(Chunk::BlockType("wet stones", { 46 }, true, false, false, SoundType::STONE)); //CC0 (and a bit self made), good
				Chunk::AddBlock(Chunk::BlockType("sandstone", { 47 }, true, false, false, SoundType::STONE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("gravel", { 48 }, true, false, false, SoundType::GRAVEL)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("polished wood", { 49 }, true, false, false, SoundType::STONE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("jungle ground", { 50 }, true, false, false, SoundType::DIRT)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("leaves", { 51 }, true, true, false, SoundType::STONE, 0.0, Chunk::BLOCK_RENDER_TYPE::TRANSPARENT)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("Asphalt", { 52 }, true, false, false, SoundType::STONE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("melon", { 53 }, true, false, false, SoundType::STONE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("Gold block", { 55 }, true, false, false, SoundType::STONE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("Hop", { 56 }, true, false, false, SoundType::STONE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("Diamond block", { 57 }, true, false, false, SoundType::STONE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("Metal test", { 58 }, true, false, false, SoundType::METAL)); //CC0, good (50)
				Chunk::AddBlock(Chunk::BlockType("Red concrete", { 62 }, true, false, false, SoundType::STONE)); //CC0, good (50)
				Chunk::AddBlock(Chunk::BlockType("Blue concrete", { 63 }, true, false, false, SoundType::STONE)); //CC0, good (50)


				Chunk::AddTexture("stone", 0.75);
				Chunk::AddTexture("Dirt", 0.4);
				Chunk::AddTexture("Grass", 0.1f);
				Chunk::AddTexture("Grass_Side", 0.4);
				Chunk::AddTexture("glowstone", 0.4f);
				Chunk::AddTexture("Tiles", 0.2f);
				Chunk::AddTexture("Planks", 0.27f);
				Chunk::AddTexture("Leather old", 0.2f);
				Chunk::AddTexture("Fabric", 0.3f);
				Chunk::AddTexture("Obsidian", 0.7f);
				Chunk::AddTexture("Concrete RTX", 0.6f);
				Chunk::AddTexture("Green glowstone", 0.25f);
				Chunk::AddTexture("Iron bars", 0.2f);
				Chunk::AddTexture("Lamp RTX", 0.2f);
				Chunk::AddTexture("Lantern", 0.05f);
				Chunk::AddTexture("red", 0.4);
				Chunk::AddTexture("green", 0.4);
				Chunk::AddTexture("orange tiles", 0.4);
				Chunk::AddTexture("iron block MC", 0.125);
				Chunk::AddTexture("solar panel", 0.01);
				Chunk::AddTexture("sand", 0.6);
				Chunk::AddTexture("marble", 0.1);
				Chunk::AddTexture("iron ore", 0.75);
				Chunk::AddTexture("farmland_old", 0.3);
				Chunk::AddTexture("brick", 0.58);
				Chunk::AddTexture("forest ground", 0.4);
				Chunk::AddTexture("nether bricks MC", 0.2);
				Chunk::AddTexture("log_sides", 0.58);
				Chunk::AddTexture("log_top", 0.58);
				Chunk::AddTexture("farmland_stratum", 0.3);
				Chunk::AddTexture("netherrack", 0.58);
				Chunk::AddTexture("cobblestone", 0.1);
				Chunk::AddTexture("snow", 0.4);
				Chunk::AddTexture("snow_sides", 0.4);
				Chunk::AddTexture("farmland_sides", 0.3);
				Chunk::AddTexture("ice", 0.7);
				Chunk::AddTexture("lava", 0.7);
				Chunk::AddTexture("forest ground_sides", 0.4);
				Chunk::AddTexture("stone_workbench_sides", 0.4);
				Chunk::AddTexture("stone_workbench_top", 0.8);
				Chunk::AddTexture("stone_workbench_bottom", 0.4);
				Chunk::AddTexture("forge_sides", 0.6);
				Chunk::AddTexture("forge_front", 0.8);
				Chunk::AddTexture("forge_top", 0.0);
				Chunk::AddTexture("forge_bottom", 0.6);
				Chunk::AddTexture("coal block", 0.75);
				Chunk::AddTexture("wet stones", 0.0625);
				Chunk::AddTexture("sandstone", 0.3);
				Chunk::AddTexture("gravel", 0.7);
				Chunk::AddTexture("polished wood", 0.0325);
				Chunk::AddTexture("jungle ground", 0.25);
				Chunk::AddTexture("leaves", 0.0);
				Chunk::AddTexture("Asphalt", 0.125);
				Chunk::AddTexture("Melon", 0.125);
				Chunk::AddTexture("Jelly", 0.125);
				Chunk::AddTexture("Gold block", 0.0625);
				Chunk::AddTexture("Hop", 0.25);
				Chunk::AddTexture("Diamond block MC", 0.25);
				Chunk::AddTexture("Metal", 0.125);
				Chunk::AddTexture("Green glass", 0.125);
				Chunk::AddTexture("red glass RTX", 0.125);
				Chunk::AddTexture("blue glass RTX", 0.125);
				Chunk::AddTexture("red concrete RTX", 0.125);
				Chunk::AddTexture("blue concrete RTX", 0.125);


				Chunk::GetTextureArrayList(0);

				Chunk::GenerateBlockTextureData();
				//Chunk::CreateStylizedTextures(); 
			}

			glGenTextures(1, &ChunkContainer);
			glBindTexture(GL_TEXTURE_3D, ChunkContainer);
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, (CHUNK_RENDER_DISTANCE * 2 + 1) * CHUNK_SIZE, CHUNK_SIZE, (CHUNK_RENDER_DISTANCE * 2 + 1) * CHUNK_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glGenerateMipmap(GL_TEXTURE_3D);
			glBindTexture(GL_TEXTURE_3D, 0);

			glGenTextures(1, &LightContainer);
			glBindTexture(GL_TEXTURE_3D, LightContainer);
			glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, (CHUNK_RENDER_DISTANCE * 2 + 1)* CHUNK_SIZE, CHUNK_SIZE, (CHUNK_RENDER_DISTANCE * 2 + 1)* CHUNK_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glGenerateMipmap(GL_TEXTURE_3D);
			glBindTexture(GL_TEXTURE_3D, 0);


		}
		bool Update = true;
		bool FirstGen = true;
		void WorldManager::GenerateWorld(Camera& Camera)
		{

			sf::Clock GenTime;

			//figure out where the camera is located in chunk-space 

			Vector3i CameraChunkSpace = Vector3i(Camera.Position) / CHUNK_SIZE;

			//figure out if we should gen or not 


			//CenterX = Chunks[CHUNK_RENDER_DISTANCE][CHUNK_RENDER_DISTANCE]->X;
			//CenterY = Chunks[CHUNK_RENDER_DISTANCE][CHUNK_RENDER_DISTANCE]->Y;


			if (FirstGen) {
				for (int x = 0; x < CHUNK_RENDER_DISTANCE * 2 + 1; x++) {
					for (int y = 0; y < CHUNK_RENDER_DISTANCE * 2 + 1; y++) {
						Chunks[x][y] = std::make_unique<Chunk::Chunk>(x, y);
						Chunks[x][y]->Generate({ nullptr,nullptr,nullptr,nullptr });
					}
				}

				for (int x = 0; x < CHUNK_RENDER_DISTANCE * 2 + 1; x++) {
					for (int y = 0; y < CHUNK_RENDER_DISTANCE * 2 + 1; y++) {

						std::vector<Chunk::Chunk*> Neighboors = { nullptr, nullptr, nullptr, nullptr }; 
						//^ non-ownership raw pointers, so its fine! 

						if (x != CHUNK_RENDER_DISTANCE * 2)
							Neighboors[0] = Chunks[x + 1][y].get(); 
						if (x != 0)
							Neighboors[2] = Chunks[x - 1][y].get(); 

						if (y != CHUNK_RENDER_DISTANCE * 2)
							Neighboors[1] = Chunks[x][y+1].get();
						if (y != 0)
							Neighboors[3] = Chunks[x][y-1].get();

						Chunks[x][y]->UpdateAllMeshData(Neighboors, Chunk::BLOCK_RENDER_TYPE::OPAQUE); 
						Chunks[x][y]->UpdateAllMeshData(Neighboors, Chunk::BLOCK_RENDER_TYPE::TRANSPARENT);
						Chunks[x][y]->UpdateAllMeshData(Neighboors, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE);

					}

				}

				UpdateChunkTexture(Vector3i(0), Vector3i(CHUNK_RENDER_DISTANCE * 2 + 1, 1, CHUNK_RENDER_DISTANCE * 2 + 1) * CHUNK_SIZE - Vector3i(1)); 

				FirstGen = false; 
			}
			else {

				CenterX = Chunks[CHUNK_RENDER_DISTANCE][CHUNK_RENDER_DISTANCE]->X;
				CenterY = Chunks[CHUNK_RENDER_DISTANCE][CHUNK_RENDER_DISTANCE]->Y;

				Vector2i Distance = (Vector2i(CenterX, CenterY) * CHUNK_SIZE + CHUNK_SIZE / 2) - Vector2i(Camera.Position.x, Camera.Position.z);
				
				//check if we should generate 

				if ((abs(Distance.x) > CHUNK_SIZE / 2 + 10 || abs(Distance.y) > CHUNK_SIZE / 2 + 10) && Queue.size() == 0) {

					//figure out which of the axis -> 

					QueueIdx = 0; 

					if (abs(Distance.x) > CHUNK_SIZE / 2 + 10) {

						if (Distance.x > 0) {

							//add the 3 to be generated chunks -> 

							for (int i = 0; i < 3; i++) {
								Queue.push_back({
									ChunkQueueItemState::BASESTATE,
									ChunkGenDirection::POSITIVE_X,
									std::make_unique<Chunk::Chunk>(Chunks[0][i]->X - 1, Chunks[0][i]->Y)
									}); 

							}

						}
						else {

							for (int i = 0; i < 3; i++) {
								Queue.push_back({
									ChunkQueueItemState::BASESTATE,
									ChunkGenDirection::NEGATIVE_X,
									std::make_unique<Chunk::Chunk>(Chunks[CHUNK_RENDER_DISTANCE*2][i]->X + 1, Chunks[CHUNK_RENDER_DISTANCE * 2][i]->Y)
									});

							}

						}
					}
					else  {

						if (Distance.x > 0) {

							//add the 3 to be generated chunks -> 

							for (int i = 0; i < 3; i++) {
								Queue.push_back({
									ChunkQueueItemState::BASESTATE,
									ChunkGenDirection::POSITIVE_Y,
									std::make_unique<Chunk::Chunk>(Chunks[i][0]->X, Chunks[i][0]->Y-1)
									});

							}

						}
						else {

							for (int i = 0; i < 3; i++) {
								Queue.push_back({
									ChunkQueueItemState::BASESTATE,
									ChunkGenDirection::NEGATIVE_Y,
									std::make_unique<Chunk::Chunk>(Chunks[i][CHUNK_RENDER_DISTANCE * 2]->X, Chunks[i][CHUNK_RENDER_DISTANCE * 2]->Y + 1)
									});

							}

						}

					}
					


				}
				

				if (Queue.size() != 0) {

					sf::Clock TimeClock; 

					auto& QueueItem = Queue[QueueIdx]; 

					bool AllowIncrement = true; 

					if (QueueItem.State == ChunkQueueItemState::BASESTATE) {
						QueueItem.Chunk->Generate({ nullptr,nullptr,nullptr,nullptr }); 
					}
					else {

						std::vector<Chunk::Chunk*> Neighbours = { nullptr,nullptr,nullptr,nullptr };

						AllowIncrement = false; 

						switch (QueueItem.Direction) {
							
						case ChunkGenDirection::POSITIVE_X:

							if (QueueItem.CurrentChunkUpdate < 4) {

								Neighbours[2] = Queue[QueueIdx].Chunk.get();

								if (QueueIdx != CHUNK_RENDER_DISTANCE * 2) {
									Neighbours[1] = Chunks[0][QueueIdx + 1].get();
								}
								if (QueueIdx != 0) {
									Neighbours[3] = Chunks[0][QueueIdx - 1].get();
								}

								Chunks[0][QueueIdx]->UpdateMeshData(Neighbours, 0, QueueItem.CurrentChunkUpdate, Chunk::BLOCK_RENDER_TYPE::OPAQUE);
								Chunks[0][QueueIdx]->UpdateMeshData(Neighbours, 0, QueueItem.CurrentChunkUpdate, Chunk::BLOCK_RENDER_TYPE::TRANSPARENT);
								Chunks[0][QueueIdx]->UpdateMeshData(Neighbours, 0, QueueItem.CurrentChunkUpdate, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE);

								QueueItem.CurrentChunkUpdate++;
							}
							else {

								int UpdateIndex = QueueItem.CurrentChunkUpdate - 4; 
				
								Neighbours[0] = Chunks[0][QueueIdx].get();

								if (QueueIdx != CHUNK_RENDER_DISTANCE * 2) {
									//edge case 1: 
									Neighbours[1] = Queue[QueueIdx + 1].Chunk.get();
								}
								if (QueueIdx != 0) {
									Neighbours[3] = Queue[QueueIdx - 1].Chunk.get();
								}
								
								if (UpdateIndex == 16) {
									AllowIncrement = true; 
								}
								else {
									std::cout << UpdateIndex / 4 << ' ' << UpdateIndex % 4 << '\n';

									QueueItem.Chunk->UpdateMeshData(Neighbours, UpdateIndex/4,UpdateIndex % 4, Chunk::BLOCK_RENDER_TYPE::OPAQUE);
									QueueItem.Chunk->UpdateMeshData(Neighbours, UpdateIndex/4,UpdateIndex % 4, Chunk::BLOCK_RENDER_TYPE::TRANSPARENT);
									QueueItem.Chunk->UpdateMeshData(Neighbours, UpdateIndex/4,UpdateIndex % 4, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE);
									

								}
								QueueItem.CurrentChunkUpdate++; 
							}


							break; 
						case ChunkGenDirection::NEGATIVE_X: 

							if (QueueItem.CurrentChunkUpdate < 4) {

								Neighbours[0] = Queue[QueueIdx].Chunk.get();

								if (QueueIdx != CHUNK_RENDER_DISTANCE * 2) {
									Neighbours[1] = Chunks[CHUNK_RENDER_DISTANCE*2][QueueIdx + 1].get();
								}
								if (QueueIdx != 0) {
									Neighbours[3] = Chunks[CHUNK_RENDER_DISTANCE*2][QueueIdx - 1].get();
								}

								Chunks[CHUNK_RENDER_DISTANCE*2][QueueIdx]->UpdateMeshData(Neighbours, 3, QueueItem.CurrentChunkUpdate, Chunk::BLOCK_RENDER_TYPE::OPAQUE);
								Chunks[CHUNK_RENDER_DISTANCE*2][QueueIdx]->UpdateMeshData(Neighbours, 3, QueueItem.CurrentChunkUpdate, Chunk::BLOCK_RENDER_TYPE::TRANSPARENT);
								Chunks[CHUNK_RENDER_DISTANCE*2][QueueIdx]->UpdateMeshData(Neighbours, 3, QueueItem.CurrentChunkUpdate, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE);

								QueueItem.CurrentChunkUpdate++;
							}
							else {

								int UpdateIndex = QueueItem.CurrentChunkUpdate - 4;

								Neighbours[2] = Chunks[CHUNK_RENDER_DISTANCE*2][QueueIdx].get();

								if (QueueIdx != CHUNK_RENDER_DISTANCE * 2) {
									//edge case 1: 
									Neighbours[1] = Queue[QueueIdx + 1].Chunk.get();
								}
								if (QueueIdx != 0) {
									Neighbours[3] = Queue[QueueIdx - 1].Chunk.get();
								}

								if (UpdateIndex == 16) {
									AllowIncrement = true;
								}
								else {
									

									QueueItem.Chunk->UpdateMeshData(Neighbours, UpdateIndex / 4, UpdateIndex % 4, Chunk::BLOCK_RENDER_TYPE::OPAQUE);
									QueueItem.Chunk->UpdateMeshData(Neighbours, UpdateIndex / 4, UpdateIndex % 4, Chunk::BLOCK_RENDER_TYPE::TRANSPARENT);
									QueueItem.Chunk->UpdateMeshData(Neighbours, UpdateIndex / 4, UpdateIndex % 4, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE);


								}
								QueueItem.CurrentChunkUpdate++;
							}


							break; 
						case ChunkGenDirection::POSITIVE_Y: 

							if (QueueItem.CurrentChunkUpdate < 4) {

								Neighbours[2] = Queue[QueueIdx].Chunk.get();

								if (QueueIdx != CHUNK_RENDER_DISTANCE * 2) {
									Neighbours[1] = Chunks[QueueIdx + 1][0].get();
								}
								if (QueueIdx != 0) {
									Neighbours[3] = Chunks[QueueIdx - 1][0].get();
								}

								Chunks[QueueIdx][0]->UpdateMeshData(Neighbours, 3, QueueItem.CurrentChunkUpdate, Chunk::BLOCK_RENDER_TYPE::OPAQUE);
								Chunks[QueueIdx][0]->UpdateMeshData(Neighbours, 3, QueueItem.CurrentChunkUpdate, Chunk::BLOCK_RENDER_TYPE::TRANSPARENT);
								Chunks[QueueIdx][0]->UpdateMeshData(Neighbours, 3, QueueItem.CurrentChunkUpdate, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE);

								QueueItem.CurrentChunkUpdate++;
							}
							else {

								int UpdateIndex = QueueItem.CurrentChunkUpdate - 4;

								Neighbours[0] = Chunks[QueueIdx][0].get();

								if (QueueIdx != CHUNK_RENDER_DISTANCE * 2) {
									//edge case 1: 
									Neighbours[1] = Queue[QueueIdx + 1].Chunk.get();
								}
								if (QueueIdx != 0) {
									Neighbours[3] = Queue[QueueIdx - 1].Chunk.get();
								}

								if (UpdateIndex == 16) {
									AllowIncrement = true;
								}
								else {


									QueueItem.Chunk->UpdateMeshData(Neighbours, UpdateIndex / 4, UpdateIndex % 4, Chunk::BLOCK_RENDER_TYPE::OPAQUE);
									QueueItem.Chunk->UpdateMeshData(Neighbours, UpdateIndex / 4, UpdateIndex % 4, Chunk::BLOCK_RENDER_TYPE::TRANSPARENT);
									QueueItem.Chunk->UpdateMeshData(Neighbours, UpdateIndex / 4, UpdateIndex % 4, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE);


								}
								QueueItem.CurrentChunkUpdate++;
							}

							break; 
						case ChunkGenDirection::NEGATIVE_Y:

							if (QueueItem.CurrentChunkUpdate < 4) {

								Neighbours[0] = Queue[QueueIdx].Chunk.get();

								if (QueueIdx != CHUNK_RENDER_DISTANCE * 2) {
									Neighbours[1] = Chunks[QueueIdx + 1][CHUNK_RENDER_DISTANCE * 2].get();
								}
								if (QueueIdx != 0) {
									Neighbours[3] = Chunks[QueueIdx - 1][CHUNK_RENDER_DISTANCE * 2].get();
								}

								Chunks[QueueIdx][CHUNK_RENDER_DISTANCE * 2]->UpdateMeshData(Neighbours, 3, QueueItem.CurrentChunkUpdate, Chunk::BLOCK_RENDER_TYPE::OPAQUE);
								Chunks[QueueIdx][CHUNK_RENDER_DISTANCE * 2]->UpdateMeshData(Neighbours, 3, QueueItem.CurrentChunkUpdate, Chunk::BLOCK_RENDER_TYPE::TRANSPARENT);
								Chunks[QueueIdx][CHUNK_RENDER_DISTANCE * 2]->UpdateMeshData(Neighbours, 3, QueueItem.CurrentChunkUpdate, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE);

								QueueItem.CurrentChunkUpdate++;
							}
							else {

								int UpdateIndex = QueueItem.CurrentChunkUpdate - 4;

								Neighbours[2] = Chunks[QueueIdx][CHUNK_RENDER_DISTANCE * 2].get();

								if (QueueIdx != CHUNK_RENDER_DISTANCE * 2) {
									//edge case 1: 
									Neighbours[1] = Queue[QueueIdx + 1].Chunk.get();
								}
								if (QueueIdx != 0) {
									Neighbours[3] = Queue[QueueIdx - 1].Chunk.get();
								}

								if (UpdateIndex == 16) {
									AllowIncrement = true;
								}
								else {


									QueueItem.Chunk->UpdateMeshData(Neighbours, UpdateIndex / 4, UpdateIndex % 4, Chunk::BLOCK_RENDER_TYPE::OPAQUE);
									QueueItem.Chunk->UpdateMeshData(Neighbours, UpdateIndex / 4, UpdateIndex % 4, Chunk::BLOCK_RENDER_TYPE::TRANSPARENT);
									QueueItem.Chunk->UpdateMeshData(Neighbours, UpdateIndex / 4, UpdateIndex % 4, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE);


								}
								QueueItem.CurrentChunkUpdate++;
							}

							break; 

						}




						

					}

					//move on to the next state 
					if(AllowIncrement)
						QueueItem.State = static_cast<ChunkQueueItemState>(
							static_cast<int>(QueueItem.State) + 1); 

					std::cout << "Type: " << static_cast<int>(QueueItem.State) << " Time: " << TimeClock.getElapsedTime().asSeconds() * 1000.0f << '\n';


					if (QueueIdx == Queue.size() - 1 && AllowIncrement) {
						if (Queue[QueueIdx].State == ChunkQueueItemState::FINISHED) {
							TimeClock.restart(); 
							//move the memory -> 

							switch (QueueItem.Direction) {
							case ChunkGenDirection::NEGATIVE_X:

								for (int y = 0; y < CHUNK_RENDER_DISTANCE * 2 + 1; y++) {

									for (int x = 0; x < CHUNK_RENDER_DISTANCE * 2; x++) {

										Chunks[x][y] = std::move(Chunks[x + 1][y]);
										 
									}

									Chunks[CHUNK_RENDER_DISTANCE * 2][y] = std::move(Queue[y].Chunk);

								}

								break;
							case ChunkGenDirection::POSITIVE_X:

								for (int y = CHUNK_RENDER_DISTANCE*2; y != -1; y--) {

									for (int x = CHUNK_RENDER_DISTANCE*2; x != 0; x--) {

										Chunks[x][y] = std::move(Chunks[x - 1][y]);

									}

									Chunks[0][y] = std::move(Queue[y].Chunk);
									
								}
								
								break;
							case ChunkGenDirection::NEGATIVE_Y:
								
								for (int x = 0; x < CHUNK_RENDER_DISTANCE * 2 + 1; x++) {

									for (int y = 0; y < CHUNK_RENDER_DISTANCE * 2; y++) {

										Chunks[x][y] = std::move(Chunks[x][y+1]);
								
									}

									Chunks[x][CHUNK_RENDER_DISTANCE * 2] = std::move(Queue[x].Chunk);

								}
								
								break;
							case ChunkGenDirection::POSITIVE_Y:
								
								for (int x = 0; x < CHUNK_RENDER_DISTANCE * 2 + 1; x++) {

									for (int y = CHUNK_RENDER_DISTANCE*2; y != 0; y--) {

										Chunks[x][y] = std::move(Chunks[x][y - 1]); 

									}

									Chunks[x][0] = std::move(Queue[x].Chunk);

								}
								
								break;
							}

							Queue.clear();

							UpdateChunkTexture(Vector3i(0), Vector3i(CHUNK_RENDER_DISTANCE * 2 + 1, 1, CHUNK_RENDER_DISTANCE * 2 + 1)* CHUNK_SIZE - Vector3i(1));

							glFinish(); 

							std::cout << "Final time: " << TimeClock.getElapsedTime().asSeconds() * 1000.0f << '\n';

						}
						else {
							QueueIdx = -1;
						}
					}
					if (AllowIncrement) {
						QueueIdx++;
					}





				}


			}
		

			CenterX = Chunks[CHUNK_RENDER_DISTANCE][CHUNK_RENDER_DISTANCE]->X;
			CenterY = Chunks[CHUNK_RENDER_DISTANCE][CHUNK_RENDER_DISTANCE]->Y;

			BiasX = Chunks[0][0]->X * CHUNK_SIZE;
			BiasY = Chunks[0][0]->Y * CHUNK_SIZE;

		}

		void WorldManager::RenderWorld(Camera& Camera, Shader& RenderToShader)
		{
			for (int x = 0; x < (CHUNK_RENDER_DISTANCE * 2 + 1); x++) {
				for (int y = 0; y < (CHUNK_RENDER_DISTANCE * 2 + 1); y++) {
					Chunks[x][y]->Draw(RenderToShader, Camera);

				}

			}
		}

		void WorldManager::RenderWorldTransparent(Camera& Camera, Shader& RenderToShader)
		{
			for (int x = 0; x < (CHUNK_RENDER_DISTANCE * 2 + 1); x++) {
				for (int y = 0; y < (CHUNK_RENDER_DISTANCE * 2 + 1); y++) {
					Chunks[x][y]->DrawTransparent(RenderToShader, Camera);

				}

			}
		}

		void iTrace::Rendering::WorldManager::RenderWorldRefractive(Camera& Camera, Shader& RenderToShader)
		{
			for (int x = 0; x < (CHUNK_RENDER_DISTANCE * 2 + 1); x++) {
				for (int y = 0; y < (CHUNK_RENDER_DISTANCE * 2 + 1); y++) {
					Chunks[x][y]->DrawRefractive(RenderToShader, Camera);

				}

			}
		}

		void WorldManager::AddLightBlock(Vector3i Location, Vector3i& OriginLocation, std::vector<unsigned char>& Data, int Distance, Vector3i* Min , Vector3i* Max )
		{

			const int BoundingBox = LONGESTLIGHT * 2 + 1;

			Vector3i DataLocation = Location - OriginLocation;

			Data[DataLocation.x * BoundingBox * BoundingBox + DataLocation.y * BoundingBox + DataLocation.z] = Distance;


			if (Distance > LONGESTLIGHT)
				return;

		
			for (int x = 0; x < 6; x++) {

				Vector3i NewPosition = Location + Vector3i(Chunk::BlockNormals[x]);

				int t = GetBlock(NewPosition + Vector3i(BiasX, 0, BiasY)); 

				if(t != -1) {

					Vector3i DataLocationNew = NewPosition - OriginLocation;
					if (GetBlock(DataLocationNew + Vector3i(BiasX, 0, BiasY)) != -1) {

						if (Data[DataLocationNew.x * BoundingBox * BoundingBox + DataLocationNew.y * BoundingBox + DataLocationNew.z] > Distance + 1) {

							int TypeIdx = t;

							auto& Type = Chunk::GetBlock(TypeIdx);

							if (TypeIdx == 0) {

								AddLightBlock(NewPosition, OriginLocation, Data, Distance + 1, Min, Max);

								if (Min != nullptr && Max != nullptr) {
									*Min = glm::min(*Min, Location);
									*Max = glm::max(*Max, Location);
								}

							}
						}
					}
				}


			}


		}

		void WorldManager::AddLightBlock(unsigned char WaveLength, Vector3i Location, Vector3i& OriginLocation, std::vector<unsigned char>& Data, std::vector<float>& Energy, float EnergyMultiplier, int Distance, Vector3i* Min, Vector3i* Max)
		{

			const int BoundingBox = LONGESTLIGHT * 2 + 1;

			Vector3i DataLocation = Location - OriginLocation;


			int Index = DataLocation.x * BoundingBox * BoundingBox + DataLocation.y * BoundingBox + DataLocation.z; 
			Data[Index] = Distance;
			
			float InverseLaw = 1.0 / float(9.0 + Distance * Distance * Distance); 

			Energy[Index] = InverseLaw * EnergyMultiplier;



			if (Distance > LONGESTLIGHT)
				return;


			for (int x = 0; x < 6; x++) {

				Vector3i NewPosition = Location + Vector3i(Chunk::BlockNormals[x]);

				int t = GetBlock(NewPosition + Vector3i(BiasX, 0, BiasY));

				if (t != -1) {

					Vector3i DataLocationNew = NewPosition - OriginLocation;
					if (GetBlock(DataLocationNew + Vector3i(BiasX, 0, BiasY)) != -1) {


						int TypeIdx = t;

						auto& Type = Chunk::GetBlock(TypeIdx);

						if (TypeIdx == 0 || Type.RenderType == Chunk::BLOCK_RENDER_TYPE::REFRACTIVE) {

							float EnergyMultiplierNext = EnergyMultiplier; 

							if (TypeIdx != 0)
								EnergyMultiplierNext *= Chunk::GetTextureData(Type.TexIds[0]).AlbedoAverage[WaveLength]; 

							float EnergyNext = (1.0 / (9.0 + (Distance + 1) * (Distance + 1) * (Distance + 1))) * EnergyMultiplierNext;

							if (Energy[DataLocationNew.x * BoundingBox * BoundingBox + DataLocationNew.y * BoundingBox + DataLocationNew.z] < EnergyNext) {

								AddLightBlock(WaveLength, NewPosition, OriginLocation, Data, Energy, EnergyMultiplierNext, Distance + 1, Min, Max);

								if (Min != nullptr && Max != nullptr) {
									*Min = glm::min(*Min, Location);
									*Max = glm::max(*Max, Location);
								}
							}
						}
					}
				}


			}


		}

		void WorldManager::AddLightSource(Vector3i Location, Vector3f Color, Vector3i * Min, Vector3i * Max)
		{

			std::cout << "Light source function called!\n"; 

			int BoundingBox = LONGESTLIGHT * 2 + 1;

			int BoundSize = BoundingBox * BoundingBox * BoundingBox; 

			std::vector<unsigned char> Data[3] = { std::vector<unsigned char>(BoundSize, LONGESTLIGHT + 1),std::vector<unsigned char>(BoundSize, LONGESTLIGHT + 1),std::vector<unsigned char>(BoundSize, LONGESTLIGHT + 1)};
			std::vector<float> Energy[3] = { std::vector<float>(BoundSize, 0.0), std::vector<float>(BoundSize, 0.0), std::vector<float>(BoundSize,0.0) }; 

			Vector3i OriginLocation = Location - Vector3i(LONGESTLIGHT);

			for (int x = 0; x < 6; x++) {

				Vector3i NewPosition = Location + Vector3i(Chunk::BlockNormals[x]);

				auto TypeIdx = GetBlock(NewPosition + Vector3i(BiasX, 0, BiasY));

				if (TypeIdx != -1) {

					auto& Type = Chunk::GetBlock(TypeIdx);

					

					if (TypeIdx == 0 || Type.RenderType == Chunk::BLOCK_RENDER_TYPE::REFRACTIVE) {

						Vector3f Multiplier = Vector3f(1.0); 

						if (TypeIdx != 0)
							Multiplier *= Chunk::GetTextureData(Type.TexIds[0]).AlbedoAverage; 

						for(int i = 0; i < 3; i++)
							AddLightBlock(i, NewPosition, OriginLocation, Data[i], Energy[i], Multiplier[i], 1, Min, Max);

					}

				}


			}



			for (int x = 0; x < BoundingBox; x++) {
				for (int y = 0; y < BoundingBox; y++) {
					for (int z = 0; z < BoundingBox; z++) {

						int Index = x * BoundingBox * BoundingBox + y * BoundingBox + z; 

						Vector3f Energies = {
							Energy[0][Index],
							Energy[1][Index],
							Energy[2][Index]
						}; 

						//std::cout << Energies << '\n'; 

						if (Energies.x > 0.0 || Energies.y > 0.0 || Energies.z > 0.0) {

							Vector3i BlockPos = OriginLocation + Vector3i(x, y, z);
							Vector2i ChunkIdx; 
							auto t = GetBlock(BlockPos + Vector3i(BiasX, 0, BiasY), &ChunkIdx);



							if (t != -1) {

								Vector3i RelativeBlockPos = BlockPos % CHUNK_SIZE; 

								Chunks[ChunkIdx.x][ChunkIdx.y]->BlockLighting
									[RelativeBlockPos.x * CHUNK_SIZE * CHUNK_SIZE + RelativeBlockPos.y * CHUNK_SIZE + RelativeBlockPos.z] += Vector4f(Color * Energies, 0.0);

							}
						}

					}
				}
			}

		}

		void WorldManager::SpreadSkyLight(Vector2i Coordinate)
		{



		}

		void WorldManager::FindLightSources(Vector3i Location, Vector3i& OriginLocation, Vector3i& ThisBlockPos, std::vector<bool>& Visited, std::vector<Vector4i>& FoundSources, int Distance)
		{

			const int BoundingBox = LONGESTLIGHT * 2 + 1;

			Vector3i DataLocation = Location - OriginLocation;

			Visited[DataLocation.x * BoundingBox * BoundingBox + DataLocation.y * BoundingBox + DataLocation.z] = true;


			if (Distance > LONGESTLIGHT)
				return;

			for (int x = 0; x < 6; x++) {

				Vector3i NewPosition = Location + Vector3i(Chunk::BlockNormals[x]);

				auto TypeIdx = GetBlock(NewPosition);

				if (TypeIdx != -1) {

					Vector3i DataLocationNew = NewPosition - OriginLocation;

					if (NewPosition == ThisBlockPos)
						continue;

					if (!Visited[DataLocationNew.x * BoundingBox * BoundingBox + DataLocationNew.y * BoundingBox + DataLocationNew.z]) {

						auto& Type = Chunk::GetBlock(TypeIdx);

						if (TypeIdx == 0) {

							FindLightSources(NewPosition, OriginLocation, ThisBlockPos, Visited, FoundSources, Distance + 1);

						}
						else if (Chunk::GetBlock(TypeIdx).IsEmissive) {



							Visited[DataLocationNew.x * BoundingBox * BoundingBox + DataLocationNew.y * BoundingBox + DataLocationNew.z] = true;
							FoundSources.push_back(Vector4i(NewPosition, TypeIdx));

						}



					}

				}


			}


		}

		void WorldManager::UpdateBlockThenLighting(Vector3i LocationBlock, int NewBlockType)
		{



			auto LightSources = std::vector<Vector4i>(0);

			int BoundingBox = LONGESTLIGHT * 2 + 1;

			auto Data = std::vector<bool>(BoundingBox * BoundingBox * BoundingBox, false);



			Vector3i OriginLocation = LocationBlock - Vector3i(LONGESTLIGHT);

			Vector3i DataLocation = LocationBlock - OriginLocation;

			Data[DataLocation.x * BoundingBox * BoundingBox + DataLocation.y * BoundingBox + DataLocation.z] = true;


			if (false) {
				for (int x = 0; x < 6; x++) {

					Vector3i NewPosition = LocationBlock + Vector3i(Chunk::BlockNormals[x]);

					if (NewPosition.x >= 0 && NewPosition.y >= 0 && NewPosition.z >= 0 &&
						NewPosition.x < CHUNK_SIZE && NewPosition.y < CHUNK_SIZE && NewPosition.z < CHUNK_SIZE) {

						int TypeIdx = Chunk->Blocks[NewPosition.x * CHUNK_SIZE * CHUNK_SIZE + NewPosition.y * CHUNK_SIZE + NewPosition.z];

						auto& Type = Chunk::GetBlock(TypeIdx);



						if (TypeIdx == 0) {

							FindLightSources(NewPosition, OriginLocation, LocationBlock, Data, LightSources, 1);

						}

					}


				}
			}
			else {

				for (int x = -LONGESTLIGHT; x <= LONGESTLIGHT; x++) {

					for (int y = -LONGESTLIGHT; y <= LONGESTLIGHT; y++) {

						for (int z = -LONGESTLIGHT; z <= LONGESTLIGHT; z++) {

							Vector3i NewPosition = LocationBlock + Vector3i(x, y, z);

							int t = GetBlock(NewPosition); 

							if(t != -1){

								if (x == 0 && y == 0 && z == 0)
									continue;

								auto TypeIdx = t;

								if (Chunk::GetBlock(TypeIdx).IsEmissive) {
									LightSources.push_back(Vector4i(NewPosition, TypeIdx));
								}


							}

						}

					}

				}

			}

			Vector3i Min = Vector3i(LocationBlock) - Vector3i(BiasX, 0, BiasY);
			Vector3i Max = Vector3i(LocationBlock) - Vector3i(BiasX, 0, BiasY);

			for (auto& Sources : LightSources) {
				AddLightSource(Vector3i(Sources) - Vector3i(BiasX, 0, BiasY), -GetBlockEmissiveColor(Sources.w));
			}

			std::cout << "Adding block at: " << LocationBlock << '\n'; 

			auto RenderType = static_cast<int>(Chunk::GetBlock(GetBlock(LocationBlock)).RenderType); 

			SetBlock(LocationBlock, NewBlockType, true, RenderType);

			//Chunk->SetBlock(LocationBlock.x, LocationBlock.y, LocationBlock.z, NewBlockType);

			for (auto& Sources : LightSources) {
				AddLightSource(Vector3i(Sources) - Vector3i(BiasX, 0, BiasY), GetBlockEmissiveColor(Sources.w), &Min, &Max);
			}

			SetBlock(LocationBlock, NewBlockType, true, RenderType); //<- hacky 

			UpdateChunkTexture(Min, Max, true);



		}

		Vector3i iTrace::Rendering::WorldManager::TraceBlock(Vector3f Position, Vector3f RayDirection, unsigned short Distance, int& Side, Vector2i &ChunkPos, int & TypeIdx)
		{
			Vector3f Clamped = Vector3f(RayDirection.x > 0. ? 1. : 0., RayDirection.y > 0. ? 1. : 0., RayDirection.z > 0. ? 1. : 0.);

			Vector3f NextPlane = glm::floor(Position + Clamped);

			auto GetBlockIdx = [&](Vector3i BlockPosition, Vector2i & ChunkPos) {





				Vector3i RelativeBlockPosition = BlockPosition - Vector3i(Chunks[CHUNK_RENDER_DISTANCE][CHUNK_RENDER_DISTANCE]->X, 0, Chunks[CHUNK_RENDER_DISTANCE][CHUNK_RENDER_DISTANCE]->Y) * CHUNK_SIZE;


				std::cout << "Block position: " << RelativeBlockPosition.x << ' ' << RelativeBlockPosition.z << '\n';

				Vector3i Relative = RelativeBlockPosition / CHUNK_SIZE;

				if (RelativeBlockPosition.x < 0)
					Relative.x -= 1; 
				if (RelativeBlockPosition.z < 0)
					Relative.z -= 1;

				if (abs(Relative.x) > 1 || abs(Relative.z) > 1 || RelativeBlockPosition.y >= CHUNK_SIZE || RelativeBlockPosition.y < 0) {
					std::cout << "Exit!\n";
					return -1; 
				}
				ChunkPos = Vector2i(CHUNK_RENDER_DISTANCE + Relative.x, CHUNK_RENDER_DISTANCE + Relative.z);

				std::cout << "Chunk position: " << ChunkPos.x << ' ' << ChunkPos.y << '\n';


				Vector3i BlockPos = RelativeBlockPosition - Relative * CHUNK_SIZE;

				return static_cast<int>(Chunks[CHUNK_RENDER_DISTANCE + Relative.x][CHUNK_RENDER_DISTANCE + Relative.z]->GetBlock(BlockPos.x, BlockPos.y, BlockPos.z));

			};

			for (int Step = 0; Step < Distance; Step++) {
				glm::vec3 Next = (NextPlane - Position) / RayDirection;

				unsigned short SideHit = 0;


				if (Next.x < std::min(Next.y, Next.z)) {
					Position += RayDirection * Next.x;
					Position.x = NextPlane.x;
					NextPlane.x += glm::sign(RayDirection.x);

				}
				else if (Next.y < Next.z) {
					Position += RayDirection * Next.y;
					Position.y = NextPlane.y;
					NextPlane.y += glm::sign(RayDirection.y);
					SideHit = 1;
				}
				else {
					Position += RayDirection * Next.z;
					Position.z = NextPlane.z;
					NextPlane.z += glm::sign(RayDirection.z);
					SideHit = 2;
				}

				Vector3f TexelCoord = glm::floor(NextPlane - Clamped);

				Vector3i CoordInt = Vector3i(TexelCoord.x, TexelCoord.y, TexelCoord.z);

				//	std::cout << CoordInt.x << ' ' << CoordInt.y << ' ' << CoordInt.z << '\n'; 

				TypeIdx = GetBlockIdx(CoordInt, ChunkPos);

				if (TypeIdx == -1)
					return Vector3i(-1);

				auto Block = Chunk::GetBlock(TypeIdx);

				if (Block.IsSolid) {

					Side = ((SideHit + 1) * 2) - 1;

					if (SideHit == 0) {
						if (Position.x - TexelCoord.x > 0.5)
							Side = 0;
					}
					else if (SideHit == 1) {
						if (Position.y - TexelCoord.y > 0.5)
							Side = 2;
					}
					else {
						if (Position.z - TexelCoord.z > 0.5)
							Side = 4;
					}

					return CoordInt;

				}


			}

			std::cout << "Exit2!\n";


			return Vector3i(-1);
		}

		int Rendering::WorldManager::GetBlock(Vector3i Position, Vector2i* ChunkPos)
		{
			Vector3i RelativeBlockPosition = Position - Vector3i(Chunks[CHUNK_RENDER_DISTANCE][CHUNK_RENDER_DISTANCE]->X, 0, Chunks[CHUNK_RENDER_DISTANCE][CHUNK_RENDER_DISTANCE]->Y) * CHUNK_SIZE;

			Vector3i Relative = RelativeBlockPosition / CHUNK_SIZE;

			if (RelativeBlockPosition.x < 0)
				Relative.x -= 1;
			if (RelativeBlockPosition.z < 0)
				Relative.z -= 1;

			if (abs(Relative.x) > 1 || abs(Relative.z) > 1 || RelativeBlockPosition.y >= CHUNK_SIZE || RelativeBlockPosition.y < 0)
				return -1;

			if(ChunkPos!=nullptr)
				*ChunkPos = Vector2i(CHUNK_RENDER_DISTANCE + Relative.x, CHUNK_RENDER_DISTANCE + Relative.z);

			Vector3i BlockPos = RelativeBlockPosition - Relative * CHUNK_SIZE;

			return static_cast<int>(Chunks[CHUNK_RENDER_DISTANCE + Relative.x][CHUNK_RENDER_DISTANCE + Relative.z]->GetBlock(BlockPos.x, BlockPos.y, BlockPos.z));

		}

		bool Rendering::WorldManager::SetBlock(Vector3i Position, unsigned char Type, bool Update, int _overRideType)
		{
			Vector3i RelativeBlockPosition = Position - Vector3i(Chunks[CHUNK_RENDER_DISTANCE][CHUNK_RENDER_DISTANCE]->X, 0, Chunks[CHUNK_RENDER_DISTANCE][CHUNK_RENDER_DISTANCE]->Y) * CHUNK_SIZE;

			Vector3i Relative = RelativeBlockPosition / CHUNK_SIZE;

			if (RelativeBlockPosition.x < 0)
				Relative.x -= 1;
			if (RelativeBlockPosition.z < 0)
				Relative.z -= 1;

			if (abs(Relative.x) > 1 || abs(Relative.z) > 1 || RelativeBlockPosition.y >= CHUNK_SIZE || RelativeBlockPosition.y < 0)
				return false;

			Vector3i BlockPos = RelativeBlockPosition - Relative * CHUNK_SIZE;

			unsigned char _PrevType = Chunks[CHUNK_RENDER_DISTANCE + Relative.x][CHUNK_RENDER_DISTANCE + Relative.z]->GetBlock(BlockPos.x, BlockPos.y, BlockPos.z); 


			Chunks[CHUNK_RENDER_DISTANCE + Relative.x][CHUNK_RENDER_DISTANCE + Relative.z]->SetBlock(BlockPos.x, BlockPos.y, BlockPos.z, Type); 

			if (Update) {

				int x = CHUNK_RENDER_DISTANCE + Relative.x; 
				int y = CHUNK_RENDER_DISTANCE + Relative.y; 

				std::vector<Chunk::Chunk*> Neighboors = { nullptr, nullptr, nullptr, nullptr };
				//^ non-ownership raw pointers, so its fine! 

				if (x != CHUNK_RENDER_DISTANCE * 2)
					Neighboors[0] = Chunks[x + 1][y].get();
				if (x != 0)
					Neighboors[2] = Chunks[x - 1][y].get();

				if (y != CHUNK_RENDER_DISTANCE * 2)
					Neighboors[1] = Chunks[x][y + 1].get();
				if (y != 0)
					Neighboors[3] = Chunks[x][y - 1].get();


				Chunks[CHUNK_RENDER_DISTANCE + Relative.x][CHUNK_RENDER_DISTANCE + Relative.z]->UpdateMeshDataSpecificBlock(Neighboors, BlockPos, Chunk::GetBlock(Type == 0 ? _PrevType : Type).RenderType);
				UpdateChunkTexture(Vector3i(Position) - Vector3i(BiasX, 0, BiasY), Position - Vector3i(BiasX, 0, BiasY));
			}

			return static_cast<int>(Chunks[CHUNK_RENDER_DISTANCE + Relative.x][CHUNK_RENDER_DISTANCE + Relative.z]->GetBlock(BlockPos.x, BlockPos.y, BlockPos.z));

		}

		bool WorldManager::CastBlock(Camera Camera, Chunk::BLOCK_ACTION Action, unsigned short Distance, unsigned char Block)
		{
			//for now.

			Matrix4f DView = Core::ViewMatrix(Camera.Position, Vector3f(Camera.Rotation.x - 180.f, Camera.Rotation.y, Camera.Rotation.z));
			Vector3f RayDirection = glm::transpose(Matrix3f(DView)) * Vector3f(0., 0., 1.);
			Vector3f Position = Camera.Position;

			int Side;
			Vector3i CoordInt;

			Vector2i ChunkIdx; 

			int TypeIdx; 

			CoordInt = TraceBlock(Position, RayDirection, Distance, Side, ChunkIdx, TypeIdx);

			if (CoordInt.y < 0) {
				return false;
			}
			if (Action == Chunk::BLOCK_ACTION::BREAK) {



				if (Chunk::GetBlock(TypeIdx).IsEmissive) {







					Vector3i Min = CoordInt - Vector3i(BiasX, 0, BiasY);
					Vector3i Max = CoordInt - Vector3i(BiasX, 0, BiasY);
					AddLightSource(CoordInt- Vector3i(BiasX, 0, BiasY), -GetBlockEmissiveColor(TypeIdx), &Min, &Max);

					UpdateChunkTexture(Min, Max, true);

				}
				UpdateBlockThenLighting(CoordInt, 0);
				/*
				if (Chunk->GetTallestBlock(CoordInt.x, CoordInt.z) == CoordInt.y) {

					for (int y = CoordInt.y - 1; y >= 0; y--) {

						if (!Chunk::GetBlock(Chunk->GetBlock(CoordInt.x, y, CoordInt.z)).IsEmpty) {
							Chunk->SetTallestBlock(CoordInt.x, y, CoordInt.z);
							//Chunk->UpdateMeshData({ nullptr, nullptr,nullptr,nullptr,nullptr });
							Chunk->UpdateMeshDataSpecificBlock({ nullptr, nullptr,nullptr,nullptr,nullptr }, CoordInt, Chunk::GetBlock(BreakType).RenderType);
							return true;
						}

					}


				}*/


				/*
				Vector3i RelativePosition = CoordInt % CHUNK_SIZE; 

				Chunks[ChunkIdx.x][ChunkIdx.y]->SetBlock(RelativePosition.x, RelativePosition.y, RelativePosition.z, 0); 
				Chunks[ChunkIdx.x][ChunkIdx.y]->UpdateMeshDataSpecificBlock({ nullptr, nullptr,nullptr,nullptr,nullptr }, RelativePosition, Chunk::GetBlock(TypeIdx).RenderType);
				UpdateChunkTexture(Vector3i(0), Vector3i(CHUNK_RENDER_DISTANCE * 2 + 1, 1, CHUNK_RENDER_DISTANCE * 2 + 1) * CHUNK_SIZE - 1);
				*/
				//Chunk->UpdateMeshData({ nullptr, nullptr,nullptr,nullptr,nullptr });



			}
			else if (Action == Chunk::BLOCK_ACTION::PLACE) {

				/*Vector3i RelativePosition = CoordInt % CHUNK_SIZE;
				Vector3i Offset = Chunk::BlockNormals[Side]; 

				for (int i = 0; i < 2; i++) {
					if (Offset[i*2] < 0 && RelativePosition[i*2] == 0) {
						if (ChunkIdx[i] == 0)
							return false;
						else {
							ChunkIdx[i] --;
							RelativePosition[i*2] = CHUNK_SIZE-1; 
						}
					}
					else if (Offset[i * 2] > 0 && RelativePosition[i * 2] == CHUNK_SIZE - 1) {
						if (ChunkIdx[i] == CHUNK_RENDER_DISTANCE * 2)
							return false; 
						else {
							ChunkIdx[i]++; 
							RelativePosition[i * 2] = 0; 
						}
					}
					else {
						RelativePosition[i * 2] += Offset[i * 2]; 
					}
				}


				RelativePosition.y += Offset.y; 

				if (CoordInt.y < 0 || CoordInt.y >= CHUNK_SIZE)
					return false; 
				
				Chunks[ChunkIdx.x][ChunkIdx.y]->SetBlock(RelativePosition.x, RelativePosition.y, RelativePosition.z, Block);
				Chunks[ChunkIdx.x][ChunkIdx.y]->UpdateMeshDataSpecificBlock({ nullptr, nullptr,nullptr,nullptr,nullptr }, RelativePosition, Chunk::GetBlock(Block).RenderType);
				UpdateChunkTexture(Vector3i(0), Vector3i(CHUNK_RENDER_DISTANCE * 2 + 1, 1, CHUNK_RENDER_DISTANCE * 2 + 1) * CHUNK_SIZE - 1);

				*/

				CoordInt += Chunk::BlockNormals[Side]; 
				
				if (GetBlock(CoordInt) == -1) {
					return false;
				}

				UpdateBlockThenLighting(CoordInt, Block);
				if (Chunk::GetBlock(Block).IsEmissive) {

					Vector3i Min = CoordInt - Vector3i(BiasX, 0, BiasY);
					Vector3i Max = CoordInt - Vector3i(BiasX, 0, BiasY);
					AddLightSource(CoordInt- Vector3i(BiasX, 0, BiasY), GetBlockEmissiveColor(Block), &Min, &Max);

					UpdateChunkTexture(Min, Max, true);


				}
				
				//UpdateChunkTexture(Vector3i(CoordInt), CoordInt);


				/*if (CoordInt.x > -1 && CoordInt.x < CHUNK_SIZE &&
					CoordInt.y > -1 && CoordInt.y < CHUNK_SIZE &&
					CoordInt.z > -1 && CoordInt.z < CHUNK_SIZE) {

					UpdateBlockThenLighting(CoordInt, Block);

					if (Chunk::GetBlock(Block).IsEmissive) {

						AddLightSource(CoordInt, GetBlockEmissiveColor(Block));

					}

					if (Chunk->GetTallestBlock(CoordInt.x, CoordInt.z) <= CoordInt.y) {

						Chunk->SetTallestBlock(CoordInt.x, CoordInt.y, CoordInt.z);
					}

					Chunk->UpdateMeshDataSpecificBlock({ nullptr, nullptr,nullptr,nullptr,nullptr }, CoordInt, Chunk::GetBlock(Block).RenderType);
					//Chunk->UpdateMeshData({ nullptr, nullptr,nullptr,nullptr,nullptr });

				}*/
			}

			return true;

		}

		void WorldManager::UpdateChunkTexture(Vector3i Min, Vector3i Max, bool OnlyLight) //<- always assume local space! 
		{

			glFinish();

			float TimeCopy, TimeUpload, TimeBitMask, TimeBitMaskGen, TimeMipMap;
			sf::Clock clock;



			//step 1: LOD 

			Vector3i ActualMax = Max + 1;
			Vector3i Size = ActualMax - Min;

			std::vector<unsigned char> Pixels = std::vector<unsigned char>((Size.x) * (Size.y+1) * (Size.z), 0);
			std::vector<Vector4f> ChunkLight = std::vector<Vector4f>((Size.x) * (Size.y) * (Size.z)); 


			for (int x = 0; x < Pixels.size(); x++) {
				//Pixels[x] = 1; 
			}


			//fetch the data -> 

			for (int x = Min.x; x < ActualMax.x; x++) {
				for (int y = Min.y; y < ActualMax.y; y++) {
					for (int z = Min.z; z < ActualMax.z; z++) {

						Vector3i BlockPos = Vector3i(x, y, z);

						Vector3i Texel = BlockPos - Min;

						Vector3i ChunkBlockPosition = BlockPos & 127;

						int SubChunkX = BlockPos.x / 128;
						int SubChunkZ = BlockPos.z / 128;

						if (SubChunkX > 2 || SubChunkZ > 2) {

							std::cout << x << ' ' << y << ' ' << z << '\n';
							std::cin.get();

							throw std::exception("Chunk out of range!");
						}
						auto Block = Chunks[SubChunkX][SubChunkZ]->Blocks[ChunkBlockPosition.x * CHUNK_SIZE * CHUNK_SIZE + ChunkBlockPosition.y * CHUNK_SIZE + ChunkBlockPosition.z];
						auto Light = Chunks[SubChunkX][SubChunkZ]->BlockLighting[ChunkBlockPosition.x * CHUNK_SIZE * CHUNK_SIZE + ChunkBlockPosition.y * CHUNK_SIZE + ChunkBlockPosition.z];

						
						//if(FirstGen) 
						Pixels[Texel.x * Size.y * Size.z + Texel.y * Size.z + Texel.z] = Block;
						ChunkLight[Texel.x * Size.y * Size.z + Texel.y * Size.z + Texel.z] = Light; 
						//^ what is going on here? 


					}
				}
			}

			TimeCopy = clock.getElapsedTime().asSeconds();
			clock.restart();

			if (!OnlyLight) {
				glBindTexture(GL_TEXTURE_3D, ChunkContainer);
				glTexSubImage3D(GL_TEXTURE_3D, 0, Min.z, Min.y, Min.x, Size.z, Size.y, Size.x, GL_RED, GL_UNSIGNED_BYTE, Pixels.data());
				glBindTexture(GL_TEXTURE_3D, 0);
			}

			glBindTexture(GL_TEXTURE_3D, this->LightContainer);
			glTexSubImage3D(GL_TEXTURE_3D, 0, Min.z, Min.y, Min.x, Size.z, Size.y, Size.x, GL_RGBA, GL_FLOAT, ChunkLight.data());
			glBindTexture(GL_TEXTURE_3D, 0);
			glFinish();
			
			TimeUpload = clock.getElapsedTime().asSeconds();
			clock.restart();


			//glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, Size.x, Size.y, Size.z, 0, GL_RED, GL_UNSIGNED_BYTE, Pixels.data()); 

			//bitmask -> 

			glBindTexture(GL_TEXTURE_3D, ChunkContainer);

			//glGenerateMipmap(GL_TEXTURE_3D); //<- can we avoid this call? very slow. 

			glBindTexture(GL_TEXTURE_3D, 0);
			glFinish();

			TimeMipMap = clock.getElapsedTime().asSeconds();
			clock.restart();


			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			auto ComputeBitMask = [=](Vector3i Pos) {

				unsigned char BitMask = 0;

				for (int SubVoxel = 0; SubVoxel < 8; SubVoxel++) {

					Vector3i SubVoxelLocation = Chunk::VIndicies[SubVoxel];

					Vector3i Voxel = Pos * 2 + SubVoxelLocation;

					Vector3i ChunkBlockPosition = Voxel & 127;

					int SubChunkX = Voxel.x / 128;
					int SubChunkZ = Voxel.z / 128;

					auto Block = Chunks[SubChunkX][SubChunkZ]->Blocks[ChunkBlockPosition.x * CHUNK_SIZE * CHUNK_SIZE + ChunkBlockPosition.y * CHUNK_SIZE + ChunkBlockPosition.z];

					if (Block != 0) {
						BitMask |= 1 << SubVoxel;
					}

				}

				return BitMask;

			};

			Vector3i DivMin = Min / 2;
			Vector3i DivMax = Max / 2 + 1;

			Vector3i DivSize = DivMax - DivMin;

			std::vector<unsigned char> SubPixels = std::vector<unsigned char>(DivSize.x * DivSize.y * DivSize.z, 0);


			for (int x = DivMin.x; x < DivMax.x; x++) {

				for (int y = DivMin.y; y < DivMax.y; y++) {

					for (int z = DivMin.z; z < DivMax.z; z++) {

						Vector3i BlockPos = Vector3i(x, y, z);

						Vector3i Texel = BlockPos - DivMin;

						SubPixels[Texel.x * DivSize.y * DivSize.z + Texel.y * DivSize.z + Texel.z] = ComputeBitMask(BlockPos);


					}

				}


			}

			TimeBitMaskGen = clock.getElapsedTime().asSeconds();
			clock.restart();

			glBindTexture(GL_TEXTURE_3D, ChunkContainer);

			//glTexSubImage3D(GL_TEXTURE_3D, 1, DivMin.x, DivMin.y, DivMin.z, DivSize.x, DivSize.y, DivSize.z, GL_RED, GL_UNSIGNED_BYTE, SubPixels.data());

			glBindTexture(GL_TEXTURE_3D, 0);
			glFinish();
			TimeBitMask = clock.getElapsedTime().asSeconds();

			std::cout << "Performance numbers: \nTime copy: " << TimeCopy << "\nTime upload: " << TimeUpload << "\nTime mipmap: " << TimeMipMap << "\nTime bitmask gen: " << TimeBitMaskGen << "\nTime bitmask: " << TimeBitMask << '\n';


		}

		void iTrace::Rendering::WorldManager::ManageCollision(Vector3f& Position, Vector3f& Acceleration, Vector3f& Velocity)
		{
			for (int x = 0; x < CHUNK_RENDER_DISTANCE * 2 + 1; x++) {
				for (int y = 0; y < CHUNK_RENDER_DISTANCE * 2 + 1; y++) {
					Chunks[x][y]->ManageCollision(Position, Acceleration, Velocity);
				}
			}


		}

	}

}