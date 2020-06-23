#include "Chunk.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include "glm-master/glm/gtc/noise.hpp"
#include "ItemRenderer.h"
#include <sstream>
#include "ParallaxBaker.h"

namespace iTrace {

	namespace Rendering {

		namespace Chunk {

			using namespace glm;

			std::vector<BlockType> Types;
			std::vector<TextureDir> TextureDirectories;
			unsigned int BlockDataTexture, TextureExtensionData, BlockExtraDataTexture;
			std::array<unsigned int, static_cast<int>(TextureType::SIZE)> TextureArrays;

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
				if (!Type.IsEmpty)
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

			void ResizeAndSaveImage(const std::string& InputFile, const std::string& OutPutFile, int OutPutResolution) {


				sf::Image BaseImage;
				BaseImage.loadFromFile(InputFile);

				bool SuperSample = BaseImage.getSize().x == BaseImage.getSize().y && (BaseImage.getSize().x % OutPutResolution == 0);

				auto Pixels = BaseImage.getPixelsPtr();

				std::vector<unsigned char> ImageData = std::vector<unsigned char>(OutPutResolution * OutPutResolution * 4);


				if (SuperSample) {

					std::cout << "Supersampling!\n";

					int PixelToSuperSample = BaseImage.getSize().x / OutPutResolution;

					for (int x = 0; x < OutPutResolution; x++) {

						for (int y = 0; y < OutPutResolution; y++) {

							Vector4f Average = Vector4f(0.0);

							for (int SubX = 0; SubX < PixelToSuperSample; SubX++) {

								for (int SubY = 0; SubY < PixelToSuperSample; SubY++) {

									for (int c = 0; c < 4; c++) {

										Average[c] += float(Pixels[((y * PixelToSuperSample + SubY) * BaseImage.getSize().x + (x * PixelToSuperSample + SubX)) * 4 + c]) / 255.f;

									}

								}


							}

							Average /= Vector4f(PixelToSuperSample * PixelToSuperSample);
							for (int i = 0; i < 4; i++) {
								ImageData[(y * OutPutResolution + x) * 4 + i] = unsigned char(Average[i] * 255);
							}


						}

					}





				}
				else {

					for (int x = 0; x < OutPutResolution; x++) {
						for (int y = 0; y < OutPutResolution; y++) {

							//grab the correct coords

							float TextureX = float(x) / float(OutPutResolution);
							float TextureY = float(y) / float(OutPutResolution);

							int TexelX = glm::clamp(int(TextureX * BaseImage.getSize().x), 0, (int)BaseImage.getSize().x);
							int TexelY = glm::clamp(int(TextureY * BaseImage.getSize().y), 0, (int)BaseImage.getSize().y);

							for (int i = 0; i < 4; i++) {
								ImageData[(y * OutPutResolution + x) * 4 + i] = Pixels[(TexelY * BaseImage.getSize().x + TexelX) * 4 + i];
							}



						}
					}

				}

				sf::Image Image;

				Image.create(OutPutResolution, OutPutResolution, ImageData.data());
				Image.saveToFile(OutPutFile);






			}
			void NormalizeImage(const std::string& InputFile) {

				unsigned char Min = 255, Max = 0;

				sf::Image Image;

				if (Image.loadFromFile(InputFile)) {


					for (int x = 0; x < Image.getSize().x; x++) {
						for (int y = 0; y < Image.getSize().y; y++) {

							auto Pixel = Image.getPixel(x, y); 

							Min = min(Pixel.r, Min); 
							Max = max(Pixel.r, Max); 

						}
					}

				}

				if (Min != 0 || Max != 255) {

					float Multiplier = float(255.f) / float(Max - Min); 
					
					sf::Image SaveImage; 

					SaveImage.create(Image.getSize().x, Image.getSize().y); 

					for (int x = 0; x < Image.getSize().x; x++) {
						for (int y = 0; y < Image.getSize().y; y++) {

							auto Pixel = Image.getPixel(x, y);

							Pixel.r = glm::clamp((int)((Pixel.r-Min) * Multiplier),0,255); 
							Pixel.g = glm::clamp((int)((Pixel.g-Min) * Multiplier), 0, 255);
							Pixel.b = glm::clamp((int)((Pixel.b-Min) * Multiplier), 0, 255);
							Pixel.a = 255; 
							SaveImage.setPixel(x, y, Pixel); 

						}
					}

					SaveImage.saveToFile(InputFile); 
				}

			}
			void AddTexture(const std::string& Directory, float ParallaxStrenght)
			{
				TextureDirectories.push_back(TextureDir( Directory, ParallaxStrenght ));
			}

			void GenerateBlockTextureData()
			{




				auto Data = std::vector<unsigned char>(Types.size() * 6, 0);

				for (int TypeIndex = 0; TypeIndex < Types.size(); TypeIndex++) {

					auto& Type = Types[TypeIndex];

					for (int i = 0; i < 6; i++) {
						Data[TypeIndex * 6 + i] = Type.TexIds[i];
					}

				}

				glGenTextures(1, &TextureExtensionData);
				glGenTextures(1, &BlockExtraDataTexture);

				glGenTextures(1, &BlockDataTexture);
				glBindTexture(GL_TEXTURE_1D, BlockDataTexture);
				glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB8, Types.size() * 2, 0, GL_RGB, GL_UNSIGNED_BYTE, Data.data());
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glBindTexture(GL_TEXTURE_1D, 0);


				auto FileExists = [](const std::string FileName) {
					std::ifstream f(FileName.c_str());
					return f.good();
				};

				auto ToLower = [](const std::string& Input) {

					std::string New = Input;

					for (auto& Char : New) {

						Char = tolower(Char);

					}

					return New;

				};

				auto GetFileName = [](const std::string& PathName) {

					std::string FileNameReverse = "";

					bool FoundDot = false;

					for (int x = PathName.size(); x != -1 && PathName[x] != '/'; x--) {
						FileNameReverse += PathName[x];
					}

					std::string FileName = "";

					for (int x = 0; x < FileNameReverse.size(); x++) {

						FileName += FileNameReverse[(FileNameReverse.size() - 1) - x];

					}


					std::string FileNameNoDot = "";

					for (auto character : FileName) {

						FileNameNoDot += character;
						if (character == '.')
							break;
					}


					return FileNameNoDot;

				};

				auto GenTextureArray = [&](int idx, int format, int bytesperpixel, Vector4i FillingColor) {

					int InternalFormat =
						format == GL_RGBA ? GL_RGBA8 :
						format == GL_RGB ? GL_RGB8 :
						format == GL_RG ? GL_RG8 :
						GL_R8;


					glBindTexture(GL_TEXTURE_2D_ARRAY, TextureArrays[idx]);

					glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, InternalFormat, TEXTURE_RES, TEXTURE_RES, TextureDirectories.size(), 0, format, GL_UNSIGNED_BYTE, nullptr);

					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D_ARRAY,
						GL_TEXTURE_MIN_FILTER,
						GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D_ARRAY,
						GL_TEXTURE_MAG_FILTER,
						GL_LINEAR_MIPMAP_LINEAR);




					for (int SubImageIdx = 0; SubImageIdx < TextureDirectories.size(); SubImageIdx++) {

						auto& Directory = TextureDirectories[SubImageIdx];
						std::string FullPath = "Materials/" + Directory.BaseDirectory + "/";
						std::string FilePath = FullPath + TextureNames[idx];


						if (!FileExists(FilePath)) {

							//try to find a corresponding texture. If not, just create one with the default filling color


							std::string FoundPath = "null";

							for (const auto& Entry : std::filesystem::directory_iterator(FullPath)) {

								std::string PathName = Entry.path().generic_string();

								std::string FileName = GetFileName(PathName);

								std::cout << "Pathname: " << PathName << '\n';

								for (auto& Name : Keywords[idx]) {


									if (ToLower(FileName).find(ToLower(Name)) != std::string::npos) {
										FoundPath = PathName;
										std::cout << "Found: " << FoundPath << '\n';
									}

								}

								if (FoundPath != "null")
									break;

							}



							if (FoundPath != "null") {
								//resize. then save

								ResizeAndSaveImage(FoundPath, FilePath, TEXTURE_RES);

							}
							else {
								//create an empty with fill color. then save 


								auto Colors = std::vector<sf::Color>(TEXTURE_RES * TEXTURE_RES, sf::Color(FillingColor.x, FillingColor.y, FillingColor.z, FillingColor.w));

								sf::Image Image;

								Image.create(TEXTURE_RES, TEXTURE_RES, reinterpret_cast<sf::Uint8*>(Colors.data()));
								Image.saveToFile(FilePath);

							}


						}


						sf::Image LoadingImage;
						LoadingImage.loadFromFile(FilePath);

						auto RawPixelData = LoadingImage.getPixelsPtr();
						auto PixelData = std::vector<unsigned char>(LoadingImage.getSize().x * LoadingImage.getSize().y * bytesperpixel, 255);

						for (int pixel = 0; pixel < LoadingImage.getSize().x * LoadingImage.getSize().y; pixel++) {
							for (int color = 0; color < bytesperpixel; color++)
								PixelData[pixel * bytesperpixel + color] = RawPixelData[pixel * 4 + color];
						}

						glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
							0,
							0, 0, SubImageIdx,
							TEXTURE_RES, TEXTURE_RES, 1,
							format,
							GL_UNSIGNED_BYTE,
							PixelData.data());

					}

					glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

					glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

				};

				auto GenTextureExtensionsArray = [&](int idx, int format, int bytesperpixel, Vector2i Resolution) {

					bool IsEmmisive = (idx == static_cast<int>(TextureType::EMISSION));

					struct TextureExtensionData {

						std::string FullPath;
						int Idx;
						std::string ExtraDataPath = "";

					};

					std::vector<TextureExtensionData> ExtensionPaths;

					for (int SubImageIdx = 0; SubImageIdx < TextureDirectories.size(); SubImageIdx++) {




						auto Directory = TextureDirectories[SubImageIdx];

						std::cout << Directory.BaseDirectory << '\n'; 
						

						std::string FullPath = "Materials/" + Directory.BaseDirectory + "/";
						std::string FilePath = FullPath + TextureNames[idx];





						if (!FileExists(FilePath)) {

							std::string FoundPath = "null";

							for (const auto& Entry : std::filesystem::directory_iterator(FullPath)) {

								std::string PathName = Entry.path().generic_string();

								std::string FileName = GetFileName(PathName);

								//std::cout << "Pathname: " << PathName << '\n';

								for (auto& Name : Keywords[idx]) {


									if (ToLower(FileName).find(ToLower(Name)) != std::string::npos) {
										FoundPath = PathName;
										std::cout << "Found: " << FoundPath << "On keyword: " << Name << '\n';
									}

								}

								if (FoundPath != "null")
									break;

							}


							if (FoundPath != "null") {
								//resize. then save

								ResizeAndSaveImage(FoundPath, FilePath, TEXTURE_RES);



							}
						}

						if (FileExists(FilePath)) {

							std::string EmissiveBlockData = FullPath + "data.em";

							ExtensionPaths.push_back({ FilePath, SubImageIdx, IsEmmisive ? EmissiveBlockData : "" });

							if (IsEmmisive) {

								if (!FileExists(EmissiveBlockData)) {

									//find average of albedo * emissive 

									Vector3d Average = Vector3d(0.0);

									sf::Image AlbedoImage, EmissiveImage;

									AlbedoImage.loadFromFile(FullPath + TextureNames[static_cast<int>(TextureType::ALBEDO)]);
									EmissiveImage.loadFromFile(FilePath);

									auto AlbedoPixelPtr = AlbedoImage.getPixelsPtr();
									auto EmissivePixelPtr = EmissiveImage.getPixelsPtr();

									for (int x = 0; x < TEXTURE_RES * TEXTURE_RES; x++) {

										Vector3f AlbedoColor, EmissiveColor;

										for (int c = 0; c < 3; c++) {
											AlbedoColor[c] = AlbedoPixelPtr[x * 4 + c] / 255.f;
											EmissiveColor[c] = EmissivePixelPtr[x * 4 + c] / 255.f;
										}
										AlbedoColor = glm::pow(AlbedoColor, Vector3f(2.2));

										Average += (AlbedoColor * EmissiveColor.x);

									}





									Average /= Vector3d(TEXTURE_RES * TEXTURE_RES);

									std::ofstream OutPutData;
									OutPutData.open(EmissiveBlockData);
									OutPutData << Average.x << " " << Average.y << " " << Average.z << '\n';
									OutPutData.close();



								}





							}


						}



					}

					int InternalFormat =
						format == GL_RGBA ? GL_RGBA8 :
						format == GL_RGB ? GL_RGB8 :
						format == GL_RG ? GL_RG8 :
						GL_R8;


					glBindTexture(GL_TEXTURE_2D_ARRAY, TextureArrays[idx]);

					glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, InternalFormat, Resolution.x, Resolution.y, ExtensionPaths.size(), 0, format, GL_UNSIGNED_BYTE, nullptr);

					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D_ARRAY,
						GL_TEXTURE_MIN_FILTER,
						GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D_ARRAY,
						GL_TEXTURE_MAG_FILTER,
						GL_LINEAR_MIPMAP_LINEAR);

					for (int i = 0; i < ExtensionPaths.size(); i++) {

						auto Data = ExtensionPaths[i];

						sf::Image LoadingImage;
						LoadingImage.loadFromFile(Data.FullPath);

						std::cout << "Loading: " << Data.FullPath << " At index: " << i << '\n';

						auto RawPixelData = LoadingImage.getPixelsPtr();
						auto PixelData = std::vector<unsigned char>(LoadingImage.getSize().x * LoadingImage.getSize().y * bytesperpixel, 255);

						for (int pixel = 0; pixel < LoadingImage.getSize().x * LoadingImage.getSize().y; pixel++) {
							for (int color = 0; color < bytesperpixel; color++)
								PixelData[pixel * bytesperpixel + color] = RawPixelData[pixel * 4 + color];
						}

						glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
							0,
							0, 0, i,
							Resolution.x, Resolution.y, 1,
							format,
							GL_UNSIGNED_BYTE,
							PixelData.data());

						int ExtensionIdx = idx - (static_cast<int>(TextureType::SIZE) - static_cast<int>(TextureExtension::SIZE));

						TextureDirectories[Data.Idx].Extensions[ExtensionIdx] = i;

						if (IsEmmisive) {

							std::ifstream File;
							File.open(Data.ExtraDataPath);

							std::string Line = "";

							std::getline(File, Line);

							Vector3f Average;

							std::istringstream LineStream(Line);

							LineStream >> Average.x >> Average.y >> Average.z;

							TextureDirectories[Data.Idx].EmissiveAverage = Average;

						}


					}





					glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

					glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

				};

				
				//used for generating parallax map data. 
				{
					ParallaxBaker Parallax;
					bool NeededParallax = false;

					for (int SubImageIdx = 0; SubImageIdx < TextureDirectories.size(); SubImageIdx++) {

						auto& Directory = TextureDirectories[SubImageIdx];
						std::string FullPath = "Materials/" + Directory.BaseDirectory + "/";
						std::string FilePath = FullPath + "Height.png";
						std::string FinalPath = FullPath + "Parallax.png";





						if (!FileExists(FilePath)) {

							std::string FoundPath = "null";

							for (const auto& Entry : std::filesystem::directory_iterator(FullPath)) {

								std::string PathName = Entry.path().generic_string();

								std::string FileName = GetFileName(PathName);

								//std::cout << "Pathname: " << PathName << '\n';

								for (auto& Name : Keywords[3]) {


									if (ToLower(FileName).find(ToLower(Name)) != std::string::npos) {
										FoundPath = PathName;
										std::cout << "Found: " << FoundPath << "On keyword: " << Name << '\n';
									}

								}

								if (FoundPath != "null")
									break;

							}


							if (FoundPath != "null") {
								//resize. then save
								NormalizeImage(FoundPath);
								ResizeAndSaveImage(FoundPath, FilePath, BAKE_RESOLUTION);

								

							}


							

						}

						if (!FileExists(FinalPath) && FileExists(FilePath)) {

							if (!NeededParallax) {

								Parallax.PrepareParallaxBaker();

								NeededParallax = true;

							}

							Parallax.BakeParallaxMap(FilePath, FinalPath, Directory.ParallaxStrenght);


						}



					}




				}

				glGenTextures(static_cast<int>(TextureType::SIZE), TextureArrays.data());

				GenTextureArray(0, GL_RGB, 3, Vector4i(255));
				GenTextureArray(1, GL_RGB, 3, Vector4i(128, 128, 255, 255));
				GenTextureArray(2, GL_RGB, 3, Vector4i(128,128,128,255));
				GenTextureExtensionsArray(3, GL_RED, 1, Vector2i((BAKE_RESOLUTION+2)*BAKE_DIRECTIONS, BAKE_RESOLUTION));
				GenTextureExtensionsArray(4, GL_RED, 1, Vector2i(TEXTURE_RES));
				GenTextureExtensionsArray(5, GL_RED, 1, Vector2i(TEXTURE_RES));

				auto ExtensionPixelData = std::vector<unsigned char>(TextureDirectories.size() * static_cast<int>(TextureExtension::SIZE));

				for (int x = 0; x < TextureDirectories.size(); x++) {

					for (int ex = 0; ex < static_cast<int>(TextureExtension::SIZE); ex++) {

						std::cout << TextureDirectories[x].Extensions[ex] << ' ';

						ExtensionPixelData[x * static_cast<int>(TextureExtension::SIZE) + ex] = TextureDirectories[x].Extensions[ex] + 1;

					}
					std::cout << '\n';

				}

				int AmountOfExtensions = static_cast<int>(TextureType::SIZE);

				glBindTexture(GL_TEXTURE_1D, TextureExtensionData);
				glTexImage1D(GL_TEXTURE_1D, 0,
					GL_RGB8
					, ExtensionPixelData.size(), 0,
					GL_RGB, GL_UNSIGNED_BYTE, ExtensionPixelData.data());
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glBindTexture(GL_TEXTURE_1D, 0);


				auto BlockExtraData = std::vector<float>(Types.size());

				for (int TypeIdx = 0; TypeIdx < Types.size(); TypeIdx++) {

					BlockExtraData[TypeIdx] = Types[TypeIdx].EmissiveStrength;

				}

				glBindTexture(GL_TEXTURE_1D, BlockExtraDataTexture);
				glTexImage1D(GL_TEXTURE_1D, 0,
					GL_R16F
					, Types.size(), 0,
					GL_RED, GL_FLOAT, BlockExtraData.data());
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glBindTexture(GL_TEXTURE_1D, 0);

				//Identify and generate parallax maps! 

				
			}

			unsigned int GetTextureArrayList(unsigned char Idx)
			{
				return TextureArrays[Idx];
			}

			unsigned int GetBlockDataTexture()
			{
				return BlockDataTexture;
			}

			unsigned int GetTextureExtensionData()
			{
				return TextureExtensionData;
			}

			unsigned int GetBlockExtraDataTexture()
			{
				return BlockExtraDataTexture;
			}

			TextureDir GetTextureData(unsigned char Idx)
			{
				return TextureDirectories[Idx];
			}

			void Chunk::Draw(Camera& Camera)
			{

				glBindVertexArray(ChunkVAOID);

			//	if(sf::Keyboard::isKeyPressed(sf::Keyboard::T))
				//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

				glDrawElements(GL_TRIANGLES, Vertices, GL_UNSIGNED_INT, nullptr);


			//	if (sf::Keyboard::isKeyPressed(sf::Keyboard::T))
			//		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

				glBindVertexArray(0);

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

				auto GetType = [](int Height, int MyY) {

					int Dist = Height - MyY;

					if (Dist == 1)
						return 3; //grass
					else if (Dist < 4)
						return 2; //dirt
					return 1; //stone 

				};

				//definitely worth doing this on another thread in the future 

				Blocks = std::vector<unsigned char>(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, 0);
				BlockLighting = std::vector<Vector4f>(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, Vector4f(0.0));
				TallestBlock = std::vector<unsigned char>(CHUNK_SIZE * CHUNK_SIZE, 0);
				Mask = std::vector<BlockMask>(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, BlockMask());
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

				for (int x = 0; x < CHUNK_SIZE; x++) {

					for (int z = 0; z < CHUNK_SIZE; z++) {

						srand((x / 4) * (z / 4));

						unsigned char Height = fractalnoise(x / 128., z / 128., 0, 3) * 16 + 40;

						Height = 40; 

						float RigidNoise = 1.0 - abs(noise(x / 16.f, z / 16.f, 12381));

						unsigned char CaveBaseY = (noise(z / 16.f, x / 16.f, 1237) * 0.5 + 0.5) * 16;
						unsigned char CaveHeight = (noise(x / 32.f, z / 32.f, 1231) * 0.5 + 0.5) * 8;


						//Height = glm::max(Height, unsigned char(60)); 

						for (int y = 0; y < Height; y++) {

							Blocks[x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z] = GetType(Height, y);

							//if (RigidNoise > 0.8 && y > CaveBaseY && y < CaveBaseY + CaveHeight)
							//	Blocks[x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z] = 0; 



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

				UpdateMeshData(NeighbooringChunks);


			}

			void Chunk::UpdateMeshData(std::vector<Chunk*> NeighbooringChunks)
			{

				std::vector<Vector4f> Tris;
				std::vector<Vector3f> TexCoord;

				std::vector<unsigned int> Indicies;

				for (auto& El : Mask) El = BlockMask();

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

				Vector3i VIndicies[8] = {
					Vector3i(0,0,0),
					Vector3i(0,0,1),
					Vector3i(0,1,0),
					Vector3i(0,1,1),
					Vector3i(1,0,0),
					Vector3i(1,0,1),
					Vector3i(1,1,0),
					Vector3i(1,1,1)
				}; 



				auto ConstructMip = [&](std::vector<unsigned char> Data, unsigned char Res) { //<- 0 = not opaque, !0 = opaque 

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

				}; 


				glBindTexture(GL_TEXTURE_3D, ChunkTexID);

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

					glTexSubImage3D(GL_TEXTURE_3D, i + 1, 0,0,0, Res, Res, Res, GL_RED, GL_UNSIGNED_BYTE, Data[i].data()); 

				}

				

				glBindTexture(GL_TEXTURE_3D, 0);

				glBindTexture(GL_TEXTURE_3D, ChunkLightTexID);
				glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 0, GL_RGBA, GL_FLOAT, BlockLighting.data());
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glBindTexture(GL_TEXTURE_3D, 0);

				std::cout << "Saved triangles: " << TriangleCountSaved * 2 << '\n'; 
				std::cout << "Total triangles: " << TotalTriangles * 2 << '\n';

			}

			void Chunk::SetBlock(unsigned char x, unsigned char y, unsigned char z, unsigned char type)
			{

				Blocks[x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z] = type;

			}

			Chunk::Chunk(long long X, long long Y) : X(X), Y(Y)
			{
				glGenVertexArrays(1, &ChunkVAOID);
				glBindVertexArray(ChunkVAOID);
				glGenBuffers(3, ChunkVBOID);
				glBindVertexArray(0);
				glGenTextures(1, &ChunkTexID);
				glGenTextures(1, &ChunkLightTexID);

			}

			void Chunk::DumpToFile()
			{

				std::ofstream File; 
				File.open("World.bin", std::ios::out | std::ios::binary); 
				File.write((char*)Blocks.data(), Blocks.size()); 
				File.close(); 



			}

			void Chunk::LoadFromFile()
			{

				std::ifstream File; 
				File.open("World.bin", std::ios::out | std::ios::binary); 
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

							Vector3i NewPositionBlock = PositionBlock + Vector3i(x, y, z); 

							if (NewPositionBlock.x >= 0 && NewPositionBlock.y >= 0 && NewPositionBlock.z >= 0 &&
								NewPositionBlock.x < CHUNK_SIZE && NewPositionBlock.y < CHUNK_SIZE && NewPositionBlock.z < CHUNK_SIZE) {

								int TypeIndex = GetBlock(NewPositionBlock.x, NewPositionBlock.y, NewPositionBlock.z); 

								auto& Data = Rendering::Chunk::GetBlock(TypeIndex); 

								if (Data.IsSolid) {

									if (ManageCollision(NewPositionBlock, Position, TypeIndex)) {
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
						return Vector3i(-10);
					}
				}

				return Vector3i(-1);


			}


		}

	}

}
	