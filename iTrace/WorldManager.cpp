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
			Chunk::AddBlock(Chunk::BlockType("Stone", { 0,0,0,0,0,0 }, true, false, false)); //Stay -> For now, the texture is decent enough 
			Chunk::AddBlock(Chunk::BlockType("Dirt", { 1,1,1,1,1,1 }, true, false, false));  //Stay -> Completely CC0, good texture
			Chunk::AddBlock(Chunk::BlockType("Grass", { 3,3,3,3,2,1 }, true, false, false)); //Stay -> Completely CC0, good texture
			Chunk::AddBlock(Chunk::BlockType("Planks", { 4,4,4,4,4,4 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Gravel", { 5,5,5,5,5,5 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Green wool", { 6,6,6,6,6,6 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Red wool", { 7,7,7,7,7,7 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Orange wool", { 8,8,8,8,8,8 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Glowstone", { 9,9,9,9,9,9 }, true, false, false, 72.0 * 0.5)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Log", { 10,10,10,10,11,11 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Crafting table", { 12,12,12,12,13,13 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Furnace", { 14,16,14,14,15,15 }, true, false, false, 15.0)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Sand", { 17,17,17,17,17,17 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Sandstone", { 18,18,18,18,18,18 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Mossy cobblestone", { 19,19,19,19,19,19 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Stone brick", { 20,20,20,20,20,20 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Mossy stone brick", { 21,21,21,21,21,21 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Smooth stone", { 22,22,22,22,22,22 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Pumpkin", { 23,25,23,23,24,24 }, true, false, false,15.0)); //Stay -> Completely CC0, good texture 

			Chunk::AddBlock(Chunk::BlockType("Green glowstone", { 26,26,26,26,26,26 }, true, false, false, 72.0 * 0.5)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Blue glowstone", { 27,27,27,27,27,27 }, true, false, false, 72.0 * 0.5)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Red glowstone", { 28,28,28,28,28,28 }, true, false, false, 72.0 * 0.5)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("White glowstone", { 29,29,29,29,29,29 }, true, false, false, 72.0 * 0.5)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Discolamp", { 30,30,30,30,30,30 }, true, false, false, 15.0)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Checkers", { 31,31,31,31,31,31 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Bricks", { 32,32,32,32,32,32 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Stone floor", { 33,33,33,33,33,33 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("White stone", { 34,34,34,34,34,34 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Asphalt", { 35,35,35,35,35,35 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Monkey", { 36,36,36,36,36,36 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Crystal", { 37,37,37,37,37,37 }, true, false, false,20.0)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("red stone", { 38,38,38,38,38,38 }, true, false, false)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Yellow glowstone", { 39,39,39,39,39,39 }, true, false, false, 36.0)); //Stay -> Completely CC0, good texture 
			Chunk::AddBlock(Chunk::BlockType("Lava", { 40,40,40,40,40,40 }, true, false, false, 36.0)); //Stay -> Completely CC0, good texture 

	
			Chunk::AddTexture("stone",0.5); 
			Chunk::AddTexture("Dirt");
			Chunk::AddTexture("Grass",0.2f);
			Chunk::AddTexture("Grass_Side");
			Chunk::AddTexture("Planks",0.1f);
			Chunk::AddTexture("gravel",0.125f);
			Chunk::AddTexture("green wool", 0.33f);
			Chunk::AddTexture("red wool");
			Chunk::AddTexture("orange wool");
			Chunk::AddTexture("glowstone", 0.5f);

			Chunk::AddTexture("log");
			Chunk::AddTexture("log top");

			Chunk::AddTexture("crafting table side");
			Chunk::AddTexture("crafting table top");

			Chunk::AddTexture("furnace side");
			Chunk::AddTexture("furnace top");
			Chunk::AddTexture("furnace front");

			Chunk::AddTexture("sand",0.4f);
			Chunk::AddTexture("sandstone",0.33f);
			Chunk::AddTexture("mossy cobblestone",0.3f);
			Chunk::AddTexture("stone brick",0.25);
			Chunk::AddTexture("mossy stone brick");
			Chunk::AddTexture("smooth stone");

			Chunk::AddTexture("pumpkin sides");
			Chunk::AddTexture("pumpkin top");
			Chunk::AddTexture("pumpkin front");

			Chunk::AddTexture("green glowstone", 0.5f);
			Chunk::AddTexture("blue glowstone", 0.5f);
			Chunk::AddTexture("red glowstone", 0.5f);
			Chunk::AddTexture("white glowstone");

			Chunk::AddTexture("discolamp");
			Chunk::AddTexture("checkers",0.075f);
			Chunk::AddTexture("bricks",0.2f);

			Chunk::AddTexture("stone floor",0.2f);
			Chunk::AddTexture("White stone",0.125f);

			Chunk::AddTexture("asphalt", 0.75);
			Chunk::AddTexture("monkey", 1.0);
			Chunk::AddTexture("crystal", 0.6);
			Chunk::AddTexture("red stone", 0.75);
			Chunk::AddTexture("yellow glowstone");
			Chunk::AddTexture("lava",0.75);


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

				
				Chunk->UpdateMeshData({ nullptr,nullptr,nullptr,nullptr });
			}
		}

		void WorldManager::RenderWorld(Camera& Camera, Shader& RenderToShader)
		{

			RenderToShader.SetUniform("IdentityMatrix", Camera.Project * Camera.View); 
			Chunk->Draw(Camera); 

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

			Vector3i CoordInt = Chunk->CastBlock(Position, RayDirection, Action, Distance, Side);

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
							Chunk->UpdateMeshData({ nullptr, nullptr,nullptr,nullptr,nullptr });
							return true; 
						}

					}


				}

				Chunk->UpdateMeshData({ nullptr, nullptr,nullptr,nullptr,nullptr });

				

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


					Chunk->UpdateMeshData({ nullptr, nullptr,nullptr,nullptr,nullptr });

				}
			}

			return true; 

		}

	}

}