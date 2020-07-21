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
				Chunk::AddBlock(Chunk::BlockType("gravel", { 48 }, true, false, false, SoundType::STONE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("polished wood", { 49 }, true, false, false, SoundType::STONE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("jungle ground", { 50 }, true, false, false, SoundType::DIRT)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("leaves", { 51 }, true, true, false, SoundType::STONE, 0.0, Chunk::BLOCK_RENDER_TYPE::TRANSPARENT)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("Asphalt", { 52 }, true, false, false, SoundType::STONE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("melon", { 53 }, true, false, false, SoundType::STONE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("Jelly", { 54 }, true, false, false, SoundType::STONE, 0.0, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("Gold block", { 55 }, true, false, false, SoundType::STONE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("Hop", { 56 }, true, false, false, SoundType::STONE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("Diamond block", { 57 }, true, false, false, SoundType::STONE)); //CC0, good 
				Chunk::AddBlock(Chunk::BlockType("Metal test", { 58 }, true, false, false, SoundType::METAL)); //CC0, good 

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
				Chunk::AddTexture("Concrete", 0.6f);
				Chunk::AddTexture("Green glowstone", 0.25f);
				Chunk::AddTexture("Iron bars", 0.2f);
				Chunk::AddTexture("Lamp", 0.2f);
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

		}
		bool Update = true;
		bool FirstGen = true;
		void WorldManager::GenerateWorld(Camera& Camera)
		{

			sf::Clock GenTime;

			//figure out where the camera is located in chunk-space 

			Vector3i CameraChunkSpace = Vector3i(Camera.Position) / CHUNK_SIZE;

			//figure out if we should gen or not 

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
							Neighboors[0] = Chunks[x][y+1].get();
						if (y != 0)
							Neighboors[2] = Chunks[x][y-1].get();

						Chunks[x][y]->UpdateAllMeshData(Neighboors, Chunk::BLOCK_RENDER_TYPE::OPAQUE); 
						Chunks[x][y]->UpdateAllMeshData(Neighboors, Chunk::BLOCK_RENDER_TYPE::TRANSPARENT);
						Chunks[x][y]->UpdateAllMeshData(Neighboors, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE);

					}

				}

				UpdateChunkTexture(Vector3i(0), Vector3i(CHUNK_RENDER_DISTANCE * 2 + 1, 1, CHUNK_RENDER_DISTANCE * 2 + 1) * CHUNK_SIZE - Vector3i(1)); 

				FirstGen = false; 
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

		void WorldManager::AddLightBlock(Vector3i Location, Vector3i& OriginLocation, std::vector<unsigned char>& Data, int Distance)
		{

			const int BoundingBox = LONGESTLIGHT * 2 + 1;

			Vector3i DataLocation = Location - OriginLocation;

			Data[DataLocation.x * BoundingBox * BoundingBox + DataLocation.y * BoundingBox + DataLocation.z] = Distance;


			if (Distance > LONGESTLIGHT)
				return;

			for (int x = 0; x < 6; x++) {

				Vector3i NewPosition = Location + Vector3i(Chunk::BlockNormals[x]);

				if (NewPosition.x >= 0 && NewPosition.y >= 0 && NewPosition.z >= 0 &&
					NewPosition.x < CHUNK_SIZE && NewPosition.y < CHUNK_SIZE && NewPosition.z < CHUNK_SIZE) {

					Vector3i DataLocationNew = NewPosition - OriginLocation;

					if (DataLocationNew.x >= 0 && DataLocationNew.y >= 0 && DataLocationNew.z >= 0 &&
						DataLocationNew.x < CHUNK_SIZE && DataLocationNew.y < CHUNK_SIZE && DataLocationNew.z < CHUNK_SIZE) {

						if (Data[DataLocationNew.x * BoundingBox * BoundingBox + DataLocationNew.y * BoundingBox + DataLocationNew.z] > Distance + 1) {

							int TypeIdx = Chunk->Blocks[NewPosition.x * CHUNK_SIZE * CHUNK_SIZE + NewPosition.y * CHUNK_SIZE + NewPosition.z];

							auto& Type = Chunk::GetBlock(TypeIdx);

							if (TypeIdx == 0) {

								AddLightBlock(NewPosition, OriginLocation, Data, Distance + 1);

							}
						}
					}
				}


			}


		}

		void WorldManager::AddLightSource(Vector3i Location, Vector3f Color)
		{

			int BoundingBox = LONGESTLIGHT * 2 + 1;

			auto Data = std::vector<unsigned char>(BoundingBox * BoundingBox * BoundingBox, LONGESTLIGHT + 1);

			Vector3i OriginLocation = Location - Vector3i(LONGESTLIGHT);

			for (int x = 0; x < 6; x++) {

				Vector3i NewPosition = Location + Vector3i(Chunk::BlockNormals[x]);

				if (NewPosition.x >= 0 && NewPosition.y >= 0 && NewPosition.z >= 0 &&
					NewPosition.x < CHUNK_SIZE && NewPosition.y < CHUNK_SIZE && NewPosition.z < CHUNK_SIZE) {

					int TypeIdx = Chunk->Blocks[NewPosition.x * CHUNK_SIZE * CHUNK_SIZE + NewPosition.y * CHUNK_SIZE + NewPosition.z];

					auto& Type = Chunk::GetBlock(TypeIdx);

					if (TypeIdx == 0) {

						AddLightBlock(NewPosition, OriginLocation, Data, 1);

					}

				}


			}



			for (int x = 0; x < BoundingBox; x++) {
				for (int y = 0; y < BoundingBox; y++) {
					for (int z = 0; z < BoundingBox; z++) {

						//Data[x * BoundingBox * BoundingBox + y * BoundingBox + z] != LONGESTLIGHT+1

						auto CurrentData = Data[x * BoundingBox * BoundingBox + y * BoundingBox + z];

						if (CurrentData != LONGESTLIGHT + 1) {

							Vector3i BlockPos = OriginLocation + Vector3i(x, y, z);

							if (BlockPos.x >= 0 && BlockPos.y >= 0 && BlockPos.z >= 0 &&
								BlockPos.x < CHUNK_SIZE && BlockPos.y < CHUNK_SIZE && BlockPos.z < CHUNK_SIZE) {



								Chunk->BlockLighting[BlockPos.x * CHUNK_SIZE * CHUNK_SIZE + BlockPos.y * CHUNK_SIZE + BlockPos.z] += Vector4f(Color / (9.0f + Vector3f(CurrentData * CurrentData * CurrentData)), 0.0);





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

				if (NewPosition.x >= 0 && NewPosition.y >= 0 && NewPosition.z >= 0 &&
					NewPosition.x < CHUNK_SIZE && NewPosition.y < CHUNK_SIZE && NewPosition.z < CHUNK_SIZE) {

					Vector3i DataLocationNew = NewPosition - OriginLocation;

					if (NewPosition == ThisBlockPos)
						continue;

					if (!Visited[DataLocationNew.x * BoundingBox * BoundingBox + DataLocationNew.y * BoundingBox + DataLocationNew.z]) {

						int TypeIdx = Chunk->Blocks[NewPosition.x * CHUNK_SIZE * CHUNK_SIZE + NewPosition.y * CHUNK_SIZE + NewPosition.z];

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

							if (NewPosition.x >= 0 && NewPosition.y >= 0 && NewPosition.z >= 0 &&
								NewPosition.x < CHUNK_SIZE && NewPosition.y < CHUNK_SIZE && NewPosition.z < CHUNK_SIZE) {

								if (x == 0 && y == 0 && z == 0)
									continue;

								auto TypeIdx = Chunk->GetBlock(NewPosition.x, NewPosition.y, NewPosition.z);

								if (Chunk::GetBlock(TypeIdx).IsEmissive) {
									LightSources.push_back(Vector4i(NewPosition, TypeIdx));
								}


							}

						}

					}

				}

			}

			for (auto& Sources : LightSources) {
				AddLightSource(Sources, -GetBlockEmissiveColor(Sources.w));
			}

			Chunk->SetBlock(LocationBlock.x, LocationBlock.y, LocationBlock.z, NewBlockType);

			for (auto& Sources : LightSources) {
				AddLightSource(Sources, GetBlockEmissiveColor(Sources.w));
			}

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

				if (abs(Relative.x) > 1 || abs(Relative.z) > 1 || RelativeBlockPosition.y >= CHUNK_SIZE || RelativeBlockPosition.y < 0)
					return -1;

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



			return Vector3i(-1);
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

			if (CoordInt.x < 0)
				return false;

			if (Action == Chunk::BLOCK_ACTION::BREAK) {



				if (Chunk::GetBlock(TypeIdx).IsEmissive) {







					//AddLightSource(CoordInt, -GetBlockEmissiveColor(BreakType));

				}
				//UpdateBlockThenLighting(CoordInt, 0);
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

				Vector3i RelativePosition = CoordInt % CHUNK_SIZE; 

				Chunks[ChunkIdx.x][ChunkIdx.y]->SetBlock(RelativePosition.x, RelativePosition.y, RelativePosition.z, 0); 
				Chunks[ChunkIdx.x][ChunkIdx.y]->UpdateMeshDataSpecificBlock({ nullptr, nullptr,nullptr,nullptr,nullptr }, RelativePosition, Chunk::GetBlock(TypeIdx).RenderType);
				UpdateChunkTexture(Vector3i(0), Vector3i(CHUNK_RENDER_DISTANCE * 2 + 1, 1, CHUNK_RENDER_DISTANCE * 2 + 1) * CHUNK_SIZE - 1);

				//Chunk->UpdateMeshData({ nullptr, nullptr,nullptr,nullptr,nullptr });



			}
			else if (Action == Chunk::BLOCK_ACTION::PLACE) {



				CoordInt += Chunk::BlockNormals[Side];



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

		void WorldManager::UpdateChunkTexture(Vector3i Min, Vector3i Max) //<- always assume local space! 
		{

			glFinish();

			float TimeCopy, TimeUpload, TimeBitMask, TimeBitMaskGen, TimeMipMap;
			sf::Clock clock;



			//step 1: LOD 

			Vector3i ActualMax = Max + 1;
			Vector3i Size = ActualMax - Min;

			std::vector<unsigned char> Pixels = std::vector<unsigned char>((Size.x) * (Size.y+1) * (Size.z), 0);

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

						//if(FirstGen) 
						Pixels[Texel.x * Size.y * Size.z + Texel.y * Size.z + Texel.z] = Block;

						//^ what is going on here? 


					}
				}
			}

			TimeCopy = clock.getElapsedTime().asSeconds();
			clock.restart();

			glBindTexture(GL_TEXTURE_3D, ChunkContainer);
			glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, Size.x, Size.y, Size.z, GL_RED, GL_UNSIGNED_BYTE, Pixels.data());
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