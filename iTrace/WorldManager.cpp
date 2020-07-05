#include "WorldManager.h"
#include <iostream>

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

			Chunk::AddBlock(Chunk::BlockType()); 
			Chunk::AddBlock(Chunk::BlockType("Stone", { 0,0,0,0,0,0 }, true, false, false)); //CC0, good
			Chunk::AddBlock(Chunk::BlockType("Dirt", { 1,1,1,1,1,1 }, true, false, false));  //CC0, good
			Chunk::AddBlock(Chunk::BlockType("Grass", { 3,3,3,3,2,1 }, true, false, false)); //CC0, good
			Chunk::AddBlock(Chunk::BlockType("Glowstone", { 4 }, true, false, false, 30.0)); //CC0, okay
			Chunk::AddBlock(Chunk::BlockType("Tiles", { 5 }, true, false, false)); //CC0, good.
			Chunk::AddBlock(Chunk::BlockType("Planks", { 6 }, true, false, false));  //CC0, okay
			Chunk::AddBlock(Chunk::BlockType("Leather", { 7 }, true, false, false)); //CC0, good
			Chunk::AddBlock(Chunk::BlockType("Fabric", { 8 }, true, false, false));  //CC0, good
			Chunk::AddBlock(Chunk::BlockType("Obsidian", { 9 }, true, false, false)); //CC0, good texture
			Chunk::AddBlock(Chunk::BlockType("Concrete", { 10 }, true, false, false)); //CC0, good
			Chunk::AddBlock(Chunk::BlockType("Green glowstone", { 11 }, true, false, false, 30.0)); //CC0, good
			Chunk::AddBlock(Chunk::BlockType("Iron bars", { 12 }, true, false, false, 30.0)); //CC0, meh
			Chunk::AddBlock(Chunk::BlockType("Lamp", { 13 }, true, false, false, 30.0)); //CC0 (and a bit self made) good texture
			Chunk::AddBlock(Chunk::BlockType("Lantern", { 14 }, true, false, false, 180.0)); //CC0, good
			Chunk::AddBlock(Chunk::BlockType("red tiles", { 15 }, true, false, false)); //CC0, and a bit self-made, good
			Chunk::AddBlock(Chunk::BlockType("green tiles", { 16 }, true, false, false)); //CC0, and a bit self-made, good
			Chunk::AddBlock(Chunk::BlockType("orange tiles", { 17 }, true, false, false)); //CC0, good texture
			Chunk::AddBlock(Chunk::BlockType("iron block", { 18 }, true, false, false)); //CC0, good texture
			Chunk::AddBlock(Chunk::BlockType("solar panel", { 19 }, true, false, false)); //CC0, good texture
			Chunk::AddBlock(Chunk::BlockType("sand", { 20 }, true, false, false)); //CC0, good texture
			Chunk::AddBlock(Chunk::BlockType("marble", { 21 }, true, false, false)); //CC0 (and a bit self made), semi good
			Chunk::AddBlock(Chunk::BlockType("iron ore", { 22 }, true, false, false)); //CC0 (and a bit self made), good
			Chunk::AddBlock(Chunk::BlockType("Farmland", { 34,34,34,34,23,34 }, true, false, false)); //CC0 (and a bit self made), good
			Chunk::AddBlock(Chunk::BlockType("brick", { 24 }, true, false, false)); //CC0, meh
			Chunk::AddBlock(Chunk::BlockType("forest ground", { 37,37,37,37,25,1 }, true, false, false)); //CC0, good
			Chunk::AddBlock(Chunk::BlockType("nether bricks", { 26 }, true, false, false)); //non-CC0
			Chunk::AddBlock(Chunk::BlockType("log", { 27,27,27,27,28,28 }, true, false, false)); //CC0 (and a bit self made), good
			Chunk::AddBlock(Chunk::BlockType("wet farmland", { 34,34,34,34,29,34 }, true, false, false)); //CC0 (and a bit self made), good
			Chunk::AddBlock(Chunk::BlockType("netherrack", { 30 }, true, false, false)); //CC0, okay+
			Chunk::AddBlock(Chunk::BlockType("cobblestone", { 31 }, true, false, false)); //CC0, okay
			Chunk::AddBlock(Chunk::BlockType("snow", { 33,33,33,33,32,1 }, true, false, false)); //CC0 (and a bit self made), good
			Chunk::AddBlock(Chunk::BlockType("ice", { 35 }, true, false, false)); //CC0 (and a bit self made), semi good
			Chunk::AddBlock(Chunk::BlockType("lava", { 36 }, true, false, false,30.0)); //CC0 (and a bit self made), semi good
			Chunk::AddBlock(Chunk::BlockType("stone workbench", { 38,38,38,38,39,40 }, true, false, false)); //CC0 (and a bit self made), good
			Chunk::AddBlock(Chunk::BlockType("forge", { 41,41,41,42,43,44 }, true, false, false)); //CC0 (and a bit self made), good
			Chunk::AddBlock(Chunk::BlockType("coal block", { 45 }, true, false, false)); //CC0 (and a bit self made), good
			Chunk::AddBlock(Chunk::BlockType("wet stones", { 46 }, true, false, false)); //CC0 (and a bit self made), good
			Chunk::AddBlock(Chunk::BlockType("sandstone", { 47 }, true, false, false)); //CC0, good 
			Chunk::AddBlock(Chunk::BlockType("gravel", { 48 }, true, false, false)); //CC0, good 
			Chunk::AddBlock(Chunk::BlockType("polished wood", { 49 }, true, false, false)); //CC0, good 
			Chunk::AddBlock(Chunk::BlockType("jungle ground", { 50 }, true, false, false)); //CC0, good 
			Chunk::AddBlock(Chunk::BlockType("leaves", { 51 }, true, true, false, 0.0, Chunk::BLOCK_RENDER_TYPE::TRANSPARENT)); //CC0, good 
			Chunk::AddBlock(Chunk::BlockType("Asphalt", { 52 }, true, false, false)); //CC0, good 
			Chunk::AddBlock(Chunk::BlockType("melon", { 53 }, true, false, false)); //CC0, good 
			Chunk::AddBlock(Chunk::BlockType("Jelly", { 54 }, true, false, false, 0.0, Chunk::BLOCK_RENDER_TYPE::REFRACTIVE)); //CC0, good 
			Chunk::AddBlock(Chunk::BlockType("Gold block", { 55 }, true, false, false)); //CC0, good 
			Chunk::AddBlock(Chunk::BlockType("Hop", { 56 }, true, false, false)); //CC0, good 
			Chunk::AddBlock(Chunk::BlockType("Diamond block", { 57 }, true, false, false)); //CC0, good 

			Chunk::AddTexture("stone",0.75); 
			Chunk::AddTexture("Dirt", 0.4);
			Chunk::AddTexture("Grass",0.4f);
			Chunk::AddTexture("Grass_Side",0.4);
			Chunk::AddTexture("glowstone", 0.4f);
			Chunk::AddTexture("Tiles", 0.2f);
			Chunk::AddTexture("Planks", 0.27f);
			Chunk::AddTexture("Leather old",0.2f);
			Chunk::AddTexture("Fabric",0.3f);
			Chunk::AddTexture("Obsidian", 0.7f);
			Chunk::AddTexture("Concrete",0.6f);
			Chunk::AddTexture("Green glowstone",0.25f);
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


			Chunk::GetTextureArrayList(0); 

			Chunk::GenerateBlockTextureData(); 

		}

		void WorldManager::GenerateWorld(Camera& Camera)
		{
			if (Chunk == nullptr) {
				Chunk = new Chunk::Chunk(0, 0); 
				Chunk->Generate({ nullptr,nullptr,nullptr,nullptr }); 

				for (int x = 0; x < CHUNK_SIZE; x++) {

					for (int y = 0; y < CHUNK_SIZE; y++) {

						for (int z = 0; z < CHUNK_SIZE; z++) {

							auto BlockIndex = Chunk->GetBlock(x, y, z); 

							if (Chunk::GetBlock(BlockIndex).IsEmissive) {
								AddLightSource(Vector3i(x, y, z), GetBlockEmissiveColor(BlockIndex)); 
							}
							

						}

					}


				}

				
				Chunk->UpdateAllMeshData({ nullptr,nullptr,nullptr,nullptr }, Chunk::BLOCK_RENDER_TYPE::OPAQUE);
				Chunk->UpdateAllMeshData({ nullptr,nullptr,nullptr,nullptr }, Chunk::BLOCK_RENDER_TYPE::TRANSPARENT);

			}

			for (int x = -CHUNK_RENDER_DISTANCE; x <= CHUNK_RENDER_DISTANCE; x++) {
				for (int y = -CHUNK_RENDER_DISTANCE; y <= CHUNK_RENDER_DISTANCE; y++) {
					//TODO: add support for multiple chunks (make sure the mutli-meshing works first though!) 

				}
			}



		}

		void WorldManager::RenderWorld(Camera& Camera, Shader& RenderToShader)
		{

			RenderToShader.SetUniform("IdentityMatrix", Camera.Project * Camera.View); 
			Chunk->Draw(Camera); 

		}

		void WorldManager::RenderWorldTransparent(Camera& Camera, Shader& RenderToShader)
		{
			RenderToShader.SetUniform("IdentityMatrix", Camera.Project * Camera.View);
			Chunk->DrawTransparent(Camera);
		}

		void WorldManager::AddLightBlock(Vector3i Location, Vector3i& OriginLocation, std::vector<unsigned char> &Data, int Distance)
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

			auto Data = std::vector<unsigned char>(BoundingBox * BoundingBox * BoundingBox, LONGESTLIGHT+1);

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

								

								Chunk->BlockLighting[BlockPos.x * CHUNK_SIZE * CHUNK_SIZE + BlockPos.y * CHUNK_SIZE + BlockPos.z] += Vector4f(Color / (9.0f+Vector3f(CurrentData * CurrentData* CurrentData)),0.0);





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
							FoundSources.push_back(Vector4i(NewPosition,TypeIdx));

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

							Vector3i NewPosition = LocationBlock + Vector3i(x,y,z);

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

		

		bool WorldManager::CastBlock(Camera Camera, Chunk::BLOCK_ACTION Action, unsigned short Distance, unsigned char Block)
		{
			//for now.

			Matrix4f DView = Core::ViewMatrix(Camera.Position, Vector3f(Camera.Rotation.x - 180.f, Camera.Rotation.y, Camera.Rotation.z));
			Vector3f RayDirection = glm::transpose(Matrix3f(DView)) * Vector3f(0., 0., 1.);
			Vector3f Position = Camera.Position;
			
			int Side; 
			Vector3i CoordInt; 

			CoordInt = Chunk->CastBlock(Position, RayDirection, Action, Distance, Side);

			if (CoordInt.x < 0)
				return false; 

			if (Action == Chunk::BLOCK_ACTION::BREAK) {
				
				int BreakType = Chunk->GetBlock(CoordInt.x, CoordInt.y, CoordInt.z); 

				if (Chunk::GetBlock(BreakType).IsEmissive) {
					


					
					


					AddLightSource(CoordInt, -GetBlockEmissiveColor(BreakType));

				}
				UpdateBlockThenLighting(CoordInt, 0);

				if (Chunk->GetTallestBlock(CoordInt.x, CoordInt.z) == CoordInt.y) {

					for (int y = CoordInt.y-1; y >= 0; y--) {

						if (!Chunk::GetBlock(Chunk->GetBlock(CoordInt.x, y, CoordInt.z)).IsEmpty) {
							Chunk->SetTallestBlock(CoordInt.x, y, CoordInt.z);
							//Chunk->UpdateMeshData({ nullptr, nullptr,nullptr,nullptr,nullptr });
							Chunk->UpdateMeshDataSpecificBlock({ nullptr, nullptr,nullptr,nullptr,nullptr }, CoordInt, Chunk::GetBlock(BreakType).RenderType);
							return true; 
						}

					}


				}

				Chunk->UpdateMeshDataSpecificBlock({ nullptr, nullptr,nullptr,nullptr,nullptr }, CoordInt, Chunk::GetBlock(BreakType).RenderType);

				//Chunk->UpdateMeshData({ nullptr, nullptr,nullptr,nullptr,nullptr });

				

			}
			else if (Action == Chunk::BLOCK_ACTION::PLACE) {

				

				CoordInt += Chunk::BlockNormals[Side];



				if (CoordInt.x > -1 && CoordInt.x < CHUNK_SIZE &&
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

				}
			}

			return true; 

		}

	}

}