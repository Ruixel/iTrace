#include "Chunk.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include "glm-master/glm/gtc/noise.hpp"
#include "ItemRenderer.h"
#include <sstream>

namespace iTrace {
	namespace Rendering {

		namespace Chunk {

			using namespace glm;

			std::vector<BlockType> Types;

			const Vector3f BlockVertices[24] = {
				Vector3f(0.,0.,1.),
				Vector3f(1.,0.,1.),
				Vector3f(1.,1.,1.),
				Vector3f(0.,1.,1.),

				Vector3f(0.,0.,0.),
				Vector3f(1.,0.,0.),
				Vector3f(1.,1.,0.),
				Vector3f(0.,1.,0.),

				Vector3f(1.,0.,0.),
				Vector3f(1.,0.,1.),
				Vector3f(1.,1.,1.),
				Vector3f(1.,1.,0.),

				Vector3f(0.,0.,0.),
				Vector3f(0.,0.,1.),
				Vector3f(0.,1.,1.),
				Vector3f(0.,1.,0.),

				Vector3f(0.,1.,0.),
				Vector3f(1.,1.,0.),
				Vector3f(1.,1.,1.),
				Vector3f(0.,1.,1.),

				Vector3f(0.,0.,0.),
				Vector3f(1.,0.,0.),
				Vector3f(1.,0.,1.),
				Vector3f(0.,0.,1.),

			};

			const Vector2f BlockUVS[24] = {
				Vector2f(1.,1.),
				Vector2f(0.,1.),
				Vector2f(0.,0.),
				Vector2f(1.,0.),

				Vector2f(0.,1.),
				Vector2f(1.,1.),
				Vector2f(1.,0.),
				Vector2f(0.,0.),

				Vector2f(0.,1.),
				Vector2f(1.,1.),
				Vector2f(1.,0.),
				Vector2f(0.,0.),

				Vector2f(1.,1.),
				Vector2f(0.,1.),
				Vector2f(0.,0.),
				Vector2f(1.,0.),

				Vector2f(0.,0.),
				Vector2f(1.,0.),
				Vector2f(1.,1.),
				Vector2f(0.,1.),

				Vector2f(0.,0.),
				Vector2f(1.,0.),
				Vector2f(1.,1.),
				Vector2f(0.,1.),
			};

			Vector3f GetTexCoord(int ind, int BlockType) {

				return Vector3f(BlockUVS[ind], BlockType);
			}

			void AddBlock(BlockType Type)
			{
				if (!Type.IsNone)
					AddItemIconRequest(ItemRequestType::Block, Types.size(), Type.Name);
				Types.push_back(Type);
			}

			BlockType& GetBlock(unsigned char Idx)
			{
				return Types[Idx];
			}

			unsigned char GetBlockSize()
			{
				return static_cast<unsigned char>(Types.size());
			}

			std::vector<BlockType>& GetTypes() {
				return Types; 
			}
			
			std::vector<unsigned char> Chunk::ConstructMip(std::vector<unsigned char> Data, unsigned char Res)
			{
				
				auto NewData = std::vector<unsigned char>((Res * Res * Res) / 8, 0);

				for (int x = 0; x < Res / 2; x++) {
					for (int y = 0; y < Res / 2; y++) {

						for (int z = 0; z < Res / 2; z++) {

							//look at the sub-voxels and encode those as a bitmask 

							unsigned char BitMask = 0;

							int Importance = 1;

							for (int SubVoxel = 0; SubVoxel < 8; SubVoxel++) {

								Vector3i SubVoxelLocation = VIndicies[SubVoxel];

								Vector3i Voxel = Vector3i(x, y, z) * 2 + SubVoxelLocation;

								auto Type = Data[Voxel.x * Res * Res + Voxel.y * Res + Voxel.z];

								if (Type != 0) {
									BitMask |= 1 << SubVoxel;
								}

							}

							NewData[x * (Res / 2) * (Res / 2) + y * (Res / 2) + z] = BitMask;

						}

					}
				}


				return NewData;
			}

			void Chunk::Draw(Shader& RenderToShader, Camera& Camera)
			{
				
				Matrix4f Project = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, Camera.znear, Camera.zfar); 

				Frustum Frustum; 
				Frustum.Update(Camera.Project * Camera.View);

				if (!Frustum.InFrustum(MainChunkFrustum))
					return; 

				RenderToShader.SetUniform("IdentityMatrix", Camera.Project * Camera.View * ModelMatrix);

				for (int x = 0; x < 4; x++) {
					for (int y = 0; y < 4; y++) {
						if(Frustum.InFrustum(SecondaryFrustums[x][y]))
							MeshDataOpaque.Draw(x, y); 
					}
				}


			}

			void Chunk::DrawTransparent(Shader& RenderToShader, Camera& Camera)
			{
				RenderToShader.SetUniform("IdentityMatrix", Camera.Project * Camera.View * ModelMatrix);
				for (int x = 0; x < 4; x++) {
					for (int y = 0; y < 4; y++) {
						MeshDataTransparent.Draw(x, y);
					}
				}
			}

			void Chunk::Generate(std::vector<Chunk*> NeighbooringChunks)
			{

				auto noise = [](float x, float y, int seed) {


					return glm::simplex(Vector2f(x, y) + Vector2f(-seed, seed));

				};
				
				auto vecnoise = [](Vector2f p, int seed) {


					return glm::simplex(p + Vector2f(-seed, seed));

				};

				auto fractalnoise = [=](float x, float y, int seed, int octaves) {

					float TotalWeight = 0.5;
					float CurrentWeight = 0.5;

					float Noise = noise(x, y, seed) * CurrentWeight;

					Vector2f p = Vector2f(x, y) + Vector2f(123., 81.);

					for (int octave = 0; octave < octaves; octave++) {

						p *= 2.71;

						CurrentWeight *= 0.5;

						TotalWeight += CurrentWeight;
						Noise += vecnoise(p, seed) * CurrentWeight;

					}

					return Noise / TotalWeight;



				};

				auto GetType = [](int Height, int MyY, float BiomeNoise) {

					int Dist = Height - MyY;

					if (Dist == 1)
						return (BiomeNoise > 0.7 ? 20 : BiomeNoise > 0.3 ? 3 : 31); //snow
					else if (Dist < 4)
						return 2; //dirt
					return 1; //stone 

				};

				//definitely worth doing this on another thread in the future 

				Blocks = std::vector<unsigned char>(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, 0);
				BlockLighting = std::vector<Vector4f>(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, Vector4f(0.0));
				TallestBlock = std::vector<unsigned char>(CHUNK_SIZE * CHUNK_SIZE, 0);
				//Mask = std::vector<BlockMask>(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, BlockMask());
#ifdef SKYGRID 
				for (int x = 0; x < CHUNK_SIZE / 5; x++) {

					for (int z = 0; z < CHUNK_SIZE / 5; z++) {

						for (int y = 0; y < CHUNK_SIZE / 5; y++) {

							Blocks[x * CHUNK_SIZE * CHUNK_SIZE * 5 + y * CHUNK_SIZE * 5 + z * 5] = rand() % (Types.size() - 1) + 1;

						}

					}

				}
#else

#ifdef SKYBLOCK 


				int Types[3] = { 2,2,3 };

				for (int x = 0; x < 6; x++) {

					for (int z = 0; z < 6; z++) {

						for (int y = 0; y < 3; y++) {

							if (x <= 2 && z > 2) {

							}
							else {

								int _x = x + 64;
								int _y = y + 64;
								int _z = z + 64;

								Blocks[_x * CHUNK_SIZE * CHUNK_SIZE + _y * CHUNK_SIZE + _z] = Types[y];

							}


						}

					}

				}





#else 


				auto HeightMap = std::vector<unsigned char>(CHUNK_SIZE * CHUNK_SIZE); 

				for (int x = 0; x < CHUNK_SIZE; x++) {

					for (int z = 0; z < CHUNK_SIZE; z++) {
						HeightMap[x*CHUNK_SIZE+z] = (fractalnoise((x + X * CHUNK_SIZE) / 512., (z + Y * CHUNK_SIZE) / 512., -1238, 3) * 0.5 + 0.5) * 60 + 30;
					}

				}

				for (int x = 0; x < CHUNK_SIZE; x++) {

					for (int z = 0; z < CHUNK_SIZE; z++) {

						auto Height = HeightMap[x * CHUNK_SIZE + z]; 

						float BiomeNoise = noise((x + X * CHUNK_SIZE) / 512., (z + Y * CHUNK_SIZE) / 512., -1238) * 0.5 + 0.5;


						for (int y = 0; y < CHUNK_SIZE; y++) {

							unsigned char BlockType = 0; 

							srand(((x+X * CHUNK_SIZE) / 5)* CHUNK_SIZE + ((z+Y*CHUNK_SIZE) / 5));

							if (y < Height) {

								BlockType = GetType(Height, y, BiomeNoise); 

								//27, 43
							}
							else {

								/*if (rand() % 17 == 0) {
									//tree! 

									auto RelativeX = x % 5; 
									auto RelativeZ = z % 5; 

									auto HeightBaseTree = HeightMap[int(x / 5) * 5 * CHUNK_SIZE + int(z / 5) * 5]; 

									int HeightDiff = y - HeightBaseTree;

									if (RelativeX == 2 && RelativeZ == 2 && HeightDiff <= 3)
										BlockType = 27; 

									for (int TreeH = 0; TreeH < 3; TreeH++) {
										if ((((RelativeX == TreeH || RelativeX == 4-TreeH) && (RelativeZ >= TreeH && RelativeZ <= 4-TreeH)) || ((RelativeZ == TreeH || RelativeZ == 4 - TreeH) && (RelativeX >= TreeH && RelativeX <= 4 - TreeH))) && HeightDiff == 2+TreeH) {
											BlockType = 42; 
										}
									}


								}*/

							}


							Blocks[x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z] = BlockType; 
						}
						
						 
					}

				}

#endif
#endif 
				for (int x = 0; x < CHUNK_SIZE; x++) {
					for (int z = 0; z < CHUNK_SIZE; z++) {

						for (int y = 0; y < CHUNK_SIZE; y++) {

							if (!Rendering::Chunk::GetBlock(GetBlock(x, y, z)).IsEmpty) {
								TallestBlock[x * CHUNK_SIZE + z] = y;
							}

						}

					}
				}


				LoadFromFile(); 

				//UpdateMeshData(NeighbooringChunks);


			}

			void Chunk::UpdateMeshData(std::vector<Chunk*> NeighbooringChunks)
			{

				std::vector<Vector4f> Tris;
				std::vector<Vector3f> TexCoord;

				std::vector<unsigned int> Indicies;

				//for (auto& El : Mask) El = BlockMask();

				auto VisibleFaces = std::vector<BlockMask>(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, BlockMask());
				auto VisibleDiscardedFaces = VisibleFaces;


				int Indicie = 0;

				auto GetBlock = [&](Vector3i BlockPosition) {

					if (BlockPosition.x >= 0 && BlockPosition.x < CHUNK_SIZE &&
						BlockPosition.y >= 0 && BlockPosition.y < CHUNK_SIZE &&
						BlockPosition.z >= 0 && BlockPosition.z < CHUNK_SIZE) {


						return Types[Blocks[BlockPosition.x * CHUNK_SIZE * CHUNK_SIZE + BlockPosition.y * CHUNK_SIZE + BlockPosition.z]];


					}

					return Types[0];

				};

				auto GetIndex = [](Vector3i Position) {

					return Position.x * CHUNK_SIZE * CHUNK_SIZE + Position.y * CHUNK_SIZE + Position.z;

				};

				const int Dimensions[3] = {
					0,2,1
				};

				const Vector3i DimensionX[3] = {

					Vector3i(1,0,0),
					Vector3i(0,0,1),
					Vector3i(1,0,0)

				};

				const Vector3i DimensionY[3] = {

					Vector3i(0,1,0),
					Vector3i(0,1,0),
					Vector3i(0,0,1)

				};

				for (unsigned short x = 0; x < CHUNK_SIZE; x++) {
					for (unsigned short y = 0; y < CHUNK_SIZE; y++) {
						for (unsigned short z = 0; z < CHUNK_SIZE; z++) {

							unsigned char BlockIdx = Blocks[x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z];
							auto& Block = Types[BlockIdx];


							if (!Block.IsNone) {


								bool VisibleSides[6] = {

									!GetBlock(Vector3i(x, y, z + 1)).IsSolid,
									!GetBlock(Vector3i(x, y, z - 1)).IsSolid,
									!GetBlock(Vector3i(x + 1, y, z)).IsSolid,
									!GetBlock(Vector3i(x - 1, y, z)).IsSolid,
									!GetBlock(Vector3i(x, y + 1, z)).IsSolid,
									!GetBlock(Vector3i(x, y - 1, z)).IsSolid

								};

								for (int i = 0; i < 6; i++)
									VisibleFaces[GetIndex(Vector3i(x, y, z))].SetMask(i, VisibleSides[i]);






							}
						}
					}
				}

				int TriangleCountSaved = 0; 
				int TotalTriangles = 0; 


				for (unsigned short x = 0; x < CHUNK_SIZE; x++) {
					for (unsigned short y = 0; y < CHUNK_SIZE; y++) {
						for (unsigned short z = 0; z < CHUNK_SIZE; z++) {

							auto CurrentVisibleFaces = VisibleFaces[GetIndex(Vector3i(x, y, z))];
							auto& CurrentVisibleCulled = VisibleDiscardedFaces[GetIndex(Vector3i(x, y, z))];

							unsigned char BlockIdx = Blocks[x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z];
							auto& Block = Types[BlockIdx];

							if (!Block.IsNone) {

								for (int i = 0; i < 6; i++) {

									//has this already been greedily excluded ? 


									if (CurrentVisibleFaces.GetMask(i) && !CurrentVisibleCulled.GetMask(i)) {

										Vector2i BestGreedyRect = Vector2i(1, 1);

										int Furthest = MAX_GREEDY;

										for (int Greedyy = 0; Greedyy < MAX_GREEDY; Greedyy++) {

											for (int Greedyx = 0; Greedyx < Furthest; Greedyx++) {

												Vector3i NewCoordinate = Vector3i(x, y, z) + Greedyx * DimensionX[i / 2] + Greedyy * DimensionY[i / 2];

												if (NewCoordinate.x >= CHUNK_SIZE || NewCoordinate.y >= CHUNK_SIZE || NewCoordinate.z >= CHUNK_SIZE) {
													Furthest = Greedyx;
													break;
												}

												if (this->GetBlock(NewCoordinate.x, NewCoordinate.y, NewCoordinate.z) != BlockIdx || !VisibleFaces[GetIndex(NewCoordinate)].GetMask(i) || VisibleDiscardedFaces[GetIndex(NewCoordinate)].GetMask(i)) {
													Furthest = Greedyx;
													break;
												}

												Vector2i CurrentGreedyRect = Vector2i(Greedyx, Greedyy) + Vector2i(1);

												if (CurrentGreedyRect.x * CurrentGreedyRect.y > BestGreedyRect.x* BestGreedyRect.y) {
													BestGreedyRect = CurrentGreedyRect;
												}


											}

										}

										//iterate over the largest greedy rect ! 

										for (int rectX = 0; rectX < BestGreedyRect.x; rectX++) {

											for (int rectY = 0; rectY < BestGreedyRect.y; rectY++) {

												Vector3i NewCoordinate = Vector3i(x, y, z) + rectX * DimensionX[i / 2] + rectY * DimensionY[i / 2];

												VisibleDiscardedFaces[GetIndex(NewCoordinate)].SetMask(i, true);



											}

										}




										TriangleCountSaved += (BestGreedyRect.x * BestGreedyRect.y) - 1; 
										TotalTriangles += (BestGreedyRect.x * BestGreedyRect.y);

										Vector3f TriangleSize = Vector3i(1.0) + (BestGreedyRect.x - 1) * DimensionX[i / 2] + (BestGreedyRect.y - 1) * DimensionY[i / 2];
										Vector2f TexCoordSize = Vector2f(BestGreedyRect); 
										Vector3f ActualTexCoordSize = Vector3f(TexCoordSize, 1.0); 



										//triangle one
										Tris.push_back(Vector4f(BlockVertices[i * 4] * TriangleSize, 0.) + Vector4f(x, y, z, i)); Indicies.push_back(Indicie++); TexCoord.push_back(GetTexCoord(i * 4, BlockIdx) * ActualTexCoordSize);
										Tris.push_back(Vector4f(BlockVertices[i * 4 + 1] * TriangleSize, 0.) + Vector4f(x, y, z, i)); Indicies.push_back(Indicie++); TexCoord.push_back(GetTexCoord(i * 4 + 1, BlockIdx) * ActualTexCoordSize);
										Tris.push_back(Vector4f(BlockVertices[i * 4 + 2] * TriangleSize, 0.) + Vector4f(x, y, z, i)); Indicies.push_back(Indicie++); TexCoord.push_back(GetTexCoord(i * 4 + 2, BlockIdx) * ActualTexCoordSize);

										//triangle two
										Tris.push_back(Vector4f(BlockVertices[i * 4 + 2] * TriangleSize, 0.) + Vector4f(x, y, z, i)); Indicies.push_back(Indicie++); TexCoord.push_back(GetTexCoord(i * 4 + 2, BlockIdx) * ActualTexCoordSize);
										Tris.push_back(Vector4f(BlockVertices[i * 4 + 3] * TriangleSize, 0.) + Vector4f(x, y, z, i)); Indicies.push_back(Indicie++); TexCoord.push_back(GetTexCoord(i * 4 + 3, BlockIdx) * ActualTexCoordSize);
										Tris.push_back(Vector4f(BlockVertices[i * 4] * TriangleSize, 0.) + Vector4f(x, y, z, i)); Indicies.push_back(Indicie++); TexCoord.push_back(GetTexCoord(i * 4, BlockIdx) * ActualTexCoordSize);
										Indicies.push_back(Indicie++);

									}
								}



							}
						}
					}
				}


				Vertices = Tris.size();

				glBindVertexArray(ChunkVAOID);

				glBindBuffer(GL_ARRAY_BUFFER, ChunkVBOID[1]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(Tris[0]) * Tris.size(), Tris.data(), GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

				glBindBuffer(GL_ARRAY_BUFFER, ChunkVBOID[2]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoord[0]) * TexCoord.size(), TexCoord.data(), GL_STATIC_DRAW);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ChunkVBOID[0]);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indicies[0]) * Indicies.size(), Indicies.data(), GL_STATIC_DRAW);

				glBindVertexArray(0);

				//construct correct AC structure for the voxels! 

				

				UpdateTextureData(); 

				std::cout << "Saved triangles: " << TriangleCountSaved * 2 << '\n'; 
				std::cout << "Total triangles: " << TotalTriangles * 2 << '\n';

			}

			void Chunk::UpdateMeshData(std::vector<Chunk*> NeighbooringChunks, int SubX, int SubY, BLOCK_RENDER_TYPE RenderType)
			{
				
				int CHUNK_SIZE_SPLIT = CHUNK_SIZE / 4; 


				std::vector<Vector4f> Tris;
				std::vector<Vector3f> TexCoord;

				std::vector<unsigned int> Indicies;

				//for (auto& El : Mask) El = BlockMask();

				auto VisibleFaces = std::vector<BlockMask>(32 * CHUNK_SIZE * 32, BlockMask());
				auto VisibleDiscardedFaces = std::vector<BlockMask>(32 * CHUNK_SIZE * 32, BlockMask());


				int Indicie = 0;

				auto GetBlock = [&](Vector3i BlockPosition) {

					if (BlockPosition.x >= 0 && BlockPosition.x < CHUNK_SIZE &&
						BlockPosition.y >= 0 && BlockPosition.y < CHUNK_SIZE &&
						BlockPosition.z >= 0 && BlockPosition.z < CHUNK_SIZE) {

						return Types[Blocks[BlockPosition.x * CHUNK_SIZE * CHUNK_SIZE + BlockPosition.y * CHUNK_SIZE + BlockPosition.z]];

					}
					else if (BlockPosition.x >= CHUNK_SIZE && NeighbooringChunks[0] != nullptr) {

						Vector3i BlockPos = BlockPosition - Vector3i(CHUNK_SIZE, 0, 0); 
						return Types[NeighbooringChunks[0]->Blocks[BlockPos.x * CHUNK_SIZE * CHUNK_SIZE + BlockPos.y * CHUNK_SIZE + BlockPos.z]];

					}
					else if (BlockPosition.z >= CHUNK_SIZE && NeighbooringChunks[1] != nullptr) {

						Vector3i BlockPos = BlockPosition - Vector3i(0, 0, CHUNK_SIZE);
						return Types[NeighbooringChunks[1]->Blocks[BlockPos.x * CHUNK_SIZE * CHUNK_SIZE + BlockPos.y * CHUNK_SIZE + BlockPos.z]];

					}
					else if (BlockPosition.x < 0 && NeighbooringChunks[2] != nullptr) {

						Vector3i BlockPos = BlockPosition + Vector3i(CHUNK_SIZE, 0, 0);
						return Types[NeighbooringChunks[2]->Blocks[BlockPos.x * CHUNK_SIZE * CHUNK_SIZE + BlockPos.y * CHUNK_SIZE + BlockPos.z]];

					}
					else if (BlockPosition.z < 0 && NeighbooringChunks[3] != nullptr) {

						Vector3i BlockPos = BlockPosition + Vector3i(0, 0, CHUNK_SIZE);
						return Types[NeighbooringChunks[3]->Blocks[BlockPos.x * CHUNK_SIZE * CHUNK_SIZE + BlockPos.y * CHUNK_SIZE + BlockPos.z]];

					}

					return Types[0];

				};

				auto GetIndex = [](Vector3i Position) {

					if (Position.x >= 32 || Position.z >= 32) {
						std::cout << "Warning!\nAccessing data outside of subchunk\n";
						std::cout << Position.x << ' ' << Position.z << '\n'; 
						std::cin.get(); 
					}

					return Position.x * 32 * CHUNK_SIZE + Position.y * 32 + Position.z;

				};

				const int Dimensions[3] = {
					0,2,1
				};

				const Vector3i DimensionX[3] = {

					Vector3i(1,0,0),
					Vector3i(0,0,1),
					Vector3i(1,0,0)

				};

				const Vector3i DimensionY[3] = {

					Vector3i(0,1,0),
					Vector3i(0,1,0),
					Vector3i(0,0,1)

				};

				unsigned char StartX = 32 * SubX; 
				unsigned char StartZ = 32 * SubY; 



				for (unsigned short x = StartX; x < StartX+32; x++) {
					for (unsigned short y = 0; y < CHUNK_SIZE; y++) {
						for (unsigned short z = StartZ; z < StartZ+32; z++) {

							unsigned char BlockIdx = Blocks[x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z];
							auto& Block = Types[BlockIdx];


							if (!Block.IsNone && Block.RenderType == RenderType) {


								bool VisibleSides[6] = {

									GetBlock(Vector3i(x, y, z + 1)).IsEmpty,
									GetBlock(Vector3i(x, y, z - 1)).IsEmpty,
									GetBlock(Vector3i(x + 1, y, z)).IsEmpty,
									GetBlock(Vector3i(x - 1, y, z)).IsEmpty,
									GetBlock(Vector3i(x, y + 1, z)).IsEmpty,
									GetBlock(Vector3i(x, y - 1, z)).IsEmpty

								};

								int _x = x - StartX; 
								int _z = z - StartZ; 

								for (int i = 0; i < 6; i++)
									VisibleFaces[_x * 32 * CHUNK_SIZE + y * 32 + _z].SetMask(i, VisibleSides[i]);






							}
						}
					}
				}

				//^ assume this one works 




				for (unsigned short x = StartX; x < StartX+32; x++) {
					for (unsigned short y = 0; y < CHUNK_SIZE; y++) {
						for (unsigned short z = StartZ; z < StartZ+32; z++) {

							int _x = x - StartX;
							int _z = z - StartZ;

							auto CurrentVisibleFaces = VisibleFaces[GetIndex(Vector3i(_x, y, _z))];
							auto& CurrentVisibleCulled = VisibleDiscardedFaces[GetIndex(Vector3i(_x, y, _z))];

							unsigned char BlockIdx = Blocks[x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z];
							auto& Block = Types[BlockIdx];



							if (!Block.IsNone && Block.RenderType == RenderType) {

								for (int i = 0; i < 6; i++) {

									if (CurrentVisibleFaces.GetMask(i) && !CurrentVisibleCulled.GetMask(i)) {

										Vector2i BestGreedyRect = Vector2i(1, 1);

										int Furthest = MAX_GREEDY;

										for (int Greedyy = 0; Greedyy < MAX_GREEDY; Greedyy++) {

											for (int Greedyx = 0; Greedyx < Furthest; Greedyx++) {


												//are we exciting our current grid? 
												
												


												Vector3i NewCoordinate = Vector3i(x, y, z) + Greedyx * DimensionX[i / 2] + Greedyy * DimensionY[i / 2];
												Vector3i NewCoordinateSub = Vector3i(_x, y, _z) + Greedyx * DimensionX[i / 2] + Greedyy * DimensionY[i / 2];
												int CurrentGridX = NewCoordinate.x / 32; 
												int CurrentGridY = NewCoordinate.y / 32; 

												if (CurrentGridX != SubX || CurrentGridY != SubY || NewCoordinateSub.x >= 32 || NewCoordinateSub.z >= 32)
													break; 


												if (NewCoordinate.x >= CHUNK_SIZE || NewCoordinate.y >= CHUNK_SIZE || NewCoordinate.z >= CHUNK_SIZE) {
													Furthest = Greedyx;
													break;
												}

												if (this->GetBlock(NewCoordinate.x, NewCoordinate.y, NewCoordinate.z) != BlockIdx || !VisibleFaces[GetIndex(NewCoordinateSub)].GetMask(i) || VisibleDiscardedFaces[GetIndex(NewCoordinateSub)].GetMask(i)) {
													Furthest = Greedyx;
													break;
												}

												Vector2i CurrentGreedyRect = Vector2i(Greedyx, Greedyy) + Vector2i(1);

												if (CurrentGreedyRect.x * CurrentGreedyRect.y > BestGreedyRect.x * BestGreedyRect.y) {
													BestGreedyRect = CurrentGreedyRect;
												}


											}

										}

										//iterate over the largest greedy rect ! 

										for (int rectX = 0; rectX < BestGreedyRect.x; rectX++) {

											for (int rectY = 0; rectY < BestGreedyRect.y; rectY++) {

												Vector3i NewCoordinate = Vector3i(_x, y, _z) + rectX * DimensionX[i / 2] + rectY * DimensionY[i / 2];

												VisibleDiscardedFaces[GetIndex(NewCoordinate)].SetMask(i, true);



											}

										}

										Vector3f TriangleSize = Vector3i(1.0) + (BestGreedyRect.x - 1) * DimensionX[i / 2] + (BestGreedyRect.y - 1) * DimensionY[i / 2];
										Vector2f TexCoordSize = Vector2f(BestGreedyRect);
										Vector3f ActualTexCoordSize = Vector3f(TexCoordSize, 1.0);

										//triangle one
										Tris.push_back(Vector4f(BlockVertices[i * 4] * TriangleSize, 0.) + Vector4f(x, y, z, i)); Indicies.push_back(Indicie++); TexCoord.push_back(GetTexCoord(i * 4, BlockIdx) * ActualTexCoordSize);
										Tris.push_back(Vector4f(BlockVertices[i * 4 + 1] * TriangleSize, 0.) + Vector4f(x, y, z, i)); Indicies.push_back(Indicie++); TexCoord.push_back(GetTexCoord(i * 4 + 1, BlockIdx) * ActualTexCoordSize);
										Tris.push_back(Vector4f(BlockVertices[i * 4 + 2] * TriangleSize, 0.) + Vector4f(x, y, z, i)); Indicies.push_back(Indicie++); TexCoord.push_back(GetTexCoord(i * 4 + 2, BlockIdx) * ActualTexCoordSize);

										//triangle two
										Tris.push_back(Vector4f(BlockVertices[i * 4 + 2] * TriangleSize, 0.) + Vector4f(x, y, z, i)); Indicies.push_back(Indicie++); TexCoord.push_back(GetTexCoord(i * 4 + 2, BlockIdx) * ActualTexCoordSize);
										Tris.push_back(Vector4f(BlockVertices[i * 4 + 3] * TriangleSize, 0.) + Vector4f(x, y, z, i)); Indicies.push_back(Indicie++); TexCoord.push_back(GetTexCoord(i * 4 + 3, BlockIdx) * ActualTexCoordSize);
										Tris.push_back(Vector4f(BlockVertices[i * 4] * TriangleSize, 0.) + Vector4f(x, y, z, i)); Indicies.push_back(Indicie++); TexCoord.push_back(GetTexCoord(i * 4, BlockIdx) * ActualTexCoordSize);
										Indicies.push_back(Indicie++);

									}
								}



							}
						}
					}
				}
				
				ChunkMeshData* MeshData = nullptr; 
				switch (RenderType) {
				case BLOCK_RENDER_TYPE::OPAQUE: 
					MeshData = &MeshDataOpaque; 
					break; 

				case BLOCK_RENDER_TYPE::TRANSPARENT:
					MeshData = &MeshDataTransparent; 
					break; 

				case BLOCK_RENDER_TYPE::REFRACTIVE:
					MeshData = &MeshDataRefractive; 
					break; 

				}



				MeshData->Vertices[SubX][SubY] = Tris.size();

			//	std::cout << "Vertices: " << Tris.size() << '\n'; 

				glBindVertexArray(MeshData->ChunkVAOs[SubX][SubY]);

				glBindBuffer(GL_ARRAY_BUFFER, MeshData->ChunkVBOs[SubX][SubY][1]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(Tris[0])* Tris.size(), Tris.data(), GL_STATIC_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

				glBindBuffer(GL_ARRAY_BUFFER, MeshData->ChunkVBOs[SubX][SubY][2]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoord[0])* TexCoord.size(), TexCoord.data(), GL_STATIC_DRAW);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MeshData->ChunkVBOs[SubX][SubY][0]);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indicies[0])* Indicies.size(), Indicies.data(), GL_STATIC_DRAW);

				glBindVertexArray(0);


			}

			void Chunk::UpdateAllMeshData(std::vector<Chunk*> NeighbooringChunks, BLOCK_RENDER_TYPE RenderType)
			{
				for (int x = 0; x < 4; x++) {
					for (int y = 0; y < 4; y++) {
						UpdateMeshData(NeighbooringChunks, x, y, RenderType); 
					}
				}

				UpdateTextureData(); 

			}

			void Chunk::UpdateMeshDataSpecificBlock(std::vector<Chunk*> NeighbooringChunks, Vector3i BlockPos, BLOCK_RENDER_TYPE RenderType)
			{
				int FractX = BlockPos.x & 31; 
				int FractZ = BlockPos.z & 31; 

				int SubX = BlockPos.x / 32; 
				int SubZ = BlockPos.z / 32; 

				UpdateMeshData(NeighbooringChunks, SubX, SubZ, RenderType);

				if(FractX == 0 && SubX != 0)
					UpdateMeshData(NeighbooringChunks, SubX - 1, SubZ, RenderType);
				else if(FractX == 31 && SubX != 3)
					UpdateMeshData(NeighbooringChunks, SubX + 1, SubZ, RenderType);
				
				if(FractZ == 0 && SubZ != 0) 
					UpdateMeshData(NeighbooringChunks, SubX, SubZ - 1, RenderType);
				else if(FractZ == 31 && SubZ != 3)
					UpdateMeshData(NeighbooringChunks, SubX, SubZ + 1, RenderType);

				UpdateTextureData(); 
			}

			void Chunk::UpdateTextureData() //TODO: Remove this 
			{
				/*glBindTexture(GL_TEXTURE_3D, ChunkTexID);

				glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, Blocks.data());

				int lowestMip = 5;

				std::vector<unsigned char> Data[5];

				int Res = CHUNK_SIZE;

				glGenerateMipmap(GL_TEXTURE_3D);

				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

				for (int i = 0; i < lowestMip; i++) {


					if (i == 0)
						Data[i] = ConstructMip(Blocks, Res);
					else
						Data[i] = ConstructMip(Data[i - 1], Res);

					Res /= 2;

					glTexSubImage3D(GL_TEXTURE_3D, i + 1, 0, 0, 0, Res, Res, Res, GL_RED, GL_UNSIGNED_BYTE, Data[i].data());

				}



				glBindTexture(GL_TEXTURE_3D, 0);



				glBindTexture(GL_TEXTURE_3D, ChunkLightTexID);
				glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 0, GL_RGBA, GL_FLOAT, BlockLighting.data());
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glBindTexture(GL_TEXTURE_3D, 0);*/
			}

			void Chunk::SetBlock(unsigned char x, unsigned char y, unsigned char z, unsigned char type)
			{

				Blocks[x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z] = type;

			}

			Chunk::Chunk(long long X, long long Y) : X(X), Y(Y), MainChunkFrustum(Vector3f(X*CHUNK_SIZE, 0.0, Y*CHUNK_SIZE), Vector3f(X*CHUNK_SIZE+CHUNK_SIZE, CHUNK_SIZE, Y*CHUNK_SIZE+CHUNK_SIZE))
			{
				glGenVertexArrays(1, &ChunkVAOID);
				glBindVertexArray(ChunkVAOID);
				glGenBuffers(3, ChunkVBOID);
				glBindVertexArray(0);
				glGenTextures(1, &ChunkTexID);
				glGenTextures(1, &ChunkLightTexID);

				MeshDataOpaque.Create(); 
				MeshDataTransparent.Create(); 
				MeshDataRefractive.Create(); 

				ModelMatrix = Core::ModelMatrix(Vector3f(-X, 0, -Y) * Vector3f(CHUNK_SIZE), Vector3f(0.0), Vector3f(1.0)); 

				for (int x = 0; x < 4; x++) {
					for (int y = 0; y < 4; y++) {

						Vector3f Base = Vector3f(X, 0, Y) * float(CHUNK_SIZE); 

						Vector3f Min = Base + Vector3f(x * (CHUNK_SIZE / 4), 0, y * (CHUNK_SIZE / 4)); 
						Vector3f Max = Min + Vector3f(CHUNK_SIZE / 4, CHUNK_SIZE, CHUNK_SIZE / 4);

						SecondaryFrustums[x][y] = FrustumAABB(Min, Max); 

					}
				}



			}

			Chunk::~Chunk()
			{
				DumpToFile(); 

				MeshDataOpaque.Delete(); 
				MeshDataTransparent.Delete(); 
				MeshDataRefractive.Delete(); 
				//Mask.clear(); 
				TallestBlock.clear(); 
				Blocks.clear(); 
				BlockLighting.clear(); 
			}

			void Chunk::DumpToFile()
			{

				std::string Title = "World/Chunk_" + std::to_string(X) + '_' + std::to_string(Y) + ".bin"; 

				std::ofstream File; 
				File.open(Title, std::ios::out | std::ios::binary);
				File.write((char*)Blocks.data(), Blocks.size()); 
				File.close(); 

			}

			void Chunk::LoadFromFile()
			{

				std::string Title = "World/Chunk_" + std::to_string(X) + '_' + std::to_string(Y) + ".bin";

				std::ifstream File; 
				File.open(Title, std::ios::out | std::ios::binary);
				File.read((char*)Blocks.data(), Blocks.size()); 
				File.close(); 


			}

			void Chunk::ManageCollision(Vector3f& Position, Vector3f & Acceleration, Vector3f &Velocity)
			{
				
				//figure out the relevant blocks! 

				Vector3i PositionBlock = Vector3i(Position); 

				auto ManageCollision = [](Vector3i BlockPosition, Vector3f& Position, int BlockType) {
				
					Vector3f Positionf = Vector3f(BlockPosition); 

					Core::CollisionAABB CollisionBox;

					CollisionBox.Min = Vector3f(Positionf - Vector3f(0.125, 0.25, 0.125));
					CollisionBox.Max = Vector3f(Positionf + Vector3f(1.125, 2.5, 1.125));

					return CollisionBox.HandleCollision(Position); 
				
				}; 
				
				for (int x = -5; x <= 5; x++) {
					for (int y = -5; y <= 5; y++) {
						for (int z = -5; z <= 5; z++) {

							Vector3i NewPositionBlock = PositionBlock + Vector3i(x, y, z) - Vector3i(X, 0, Y) * CHUNK_SIZE; 

							if (NewPositionBlock.x >= 0 && NewPositionBlock.y >= 0 && NewPositionBlock.z >= 0 &&
								NewPositionBlock.x < CHUNK_SIZE && NewPositionBlock.y < CHUNK_SIZE && NewPositionBlock.z < CHUNK_SIZE) {

								int TypeIndex = GetBlock(NewPositionBlock.x, NewPositionBlock.y, NewPositionBlock.z); 

								auto& Data = Rendering::Chunk::GetBlock(TypeIndex); 

								if (Data.IsSolid) {

									if (ManageCollision(NewPositionBlock + Vector3i(X, 0, Y) * CHUNK_SIZE, Position, TypeIndex)) {
										Acceleration = Vector3f(0.0);
										Velocity = Vector3f(0.0); 
									}

								}

							}


						}
					}
				}




			}

			Vector3i Chunk::CastBlock(Vector3f Position, Vector3f RayDirection, BLOCK_ACTION Action, unsigned short Distance, int& Side)
			{

				Vector3f Clamped = Vector3f(RayDirection.x > 0. ? 1. : 0., RayDirection.y > 0. ? 1. : 0., RayDirection.z > 0. ? 1. : 0.);

				Vector3f NextPlane = glm::floor(Position + Clamped);

				auto GetBlock = [&](Vector3i BlockPosition) {

					if (BlockPosition.x >= 0 && BlockPosition.x < CHUNK_SIZE &&
						BlockPosition.y >= 0 && BlockPosition.y < CHUNK_SIZE &&
						BlockPosition.z >= 0 && BlockPosition.z < CHUNK_SIZE) {


						return Types[Blocks[BlockPosition.x * CHUNK_SIZE * CHUNK_SIZE + BlockPosition.y * CHUNK_SIZE + BlockPosition.z]];


					}

					return Types[1];

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

					if (CoordInt.x > -1 && CoordInt.x < CHUNK_SIZE &&
						CoordInt.y > -1 && CoordInt.y < CHUNK_SIZE &&
						CoordInt.z > -1 && CoordInt.z < CHUNK_SIZE) {

						auto Block = GetBlock(CoordInt);

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
							/*
							if (Action == BLOCK_ACTION::BREAK) {

								SetBlock(CoordInt.x, CoordInt.y, CoordInt.z, 0);
								UpdateMeshData({ nullptr, nullptr,nullptr,nullptr,nullptr });

								return ;
								break;

							}
							else if (Action == BLOCK_ACTION::PLACE) {

								int Side = ((SideHit + 1) * 2) - 1;

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

								CoordInt += BlockNormals[Side];



								if (CoordInt.x > -1 && CoordInt.x < CHUNK_SIZE &&
									CoordInt.y > -1 && CoordInt.y < CHUNK_SIZE &&
									CoordInt.z > -1 && CoordInt.z < CHUNK_SIZE) {

									SetBlock(CoordInt.x, CoordInt.y, CoordInt.z, Type);

									UpdateMeshData({ nullptr, nullptr,nullptr,nullptr,nullptr });

									return 1;
								}
							}

							*/
						}


					}
					else {
						return CoordInt;
					}
				}

				return Vector3i(-1,-1000,1);


			}

		}
	}
}
