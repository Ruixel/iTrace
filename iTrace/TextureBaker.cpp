#include "Chunk.h"
#include <fstream>
#include <filesystem>
#include "ParallaxBaker.h"
#include <sstream>

namespace iTrace {

	namespace Rendering {

		namespace Chunk {
			using namespace glm;

			std::vector<TextureDir> TextureDirectories;
			std::array<unsigned int, static_cast<int>(TextureType::SIZE)> TextureArrays;
			unsigned int BlockDataTexture, TextureExtensionData, BlockExtraDataTexture;


			TextureDir GetTextureData(unsigned char Idx)
			{
				return TextureDirectories[Idx];
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

							Pixel.r = glm::clamp((int)((Pixel.r - Min) * Multiplier), 0, 255);
							Pixel.g = glm::clamp((int)((Pixel.g - Min) * Multiplier), 0, 255);
							Pixel.b = glm::clamp((int)((Pixel.b - Min) * Multiplier), 0, 255);
							Pixel.a = 255;
							SaveImage.setPixel(x, y, Pixel);

						}
					}

					SaveImage.saveToFile(InputFile);
				}

			}
			void AddTexture(const std::string& Directory, float ParallaxStrenght)
			{
				TextureDirectories.push_back(TextureDir(Directory, ParallaxStrenght));
			}
			void GenerateBlockTextureData()
			{


				auto& Types = GetTypes(); 

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
				GenTextureArray(2, GL_RGB, 3, Vector4i(128, 128, 128, 255));
				GenTextureExtensionsArray(3, GL_RED, 1, Vector2i((BAKE_RESOLUTION + 2) * BAKE_DIRECTIONS, BAKE_RESOLUTION));
				GenTextureExtensionsArray(4, GL_RED, 1, Vector2i(TEXTURE_RES));
				GenTextureExtensionsArray(5, GL_RED, 1, Vector2i(TEXTURE_RES));
				GenTextureExtensionsArray(6, GL_RED, 1, Vector2i(TEXTURE_RES));


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
					GL_RGBA8
					, ExtensionPixelData.size(), 0,
					GL_RGBA, GL_UNSIGNED_BYTE, ExtensionPixelData.data());
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
			
			void CreateStylizedTextures() {
				Shader NormalMapCreationShader = Shader("Stylizer/NormalMapBake"); 
				Shader Stylizer = Shader("Stylizer/Stylizer"); 
				FrameBufferObject NormalMapCreationBuffer = FrameBufferObject(Vector2i(TEXTURE_RES), GL_RGB8, false); 

				//iterate over all regular textures. then, create stylized versions of these.
				//then, store these on the disk. 
				//stylizing is done in a shader to make it faster and easier to work with. don't clutter! 

				for (int SubImageIdx = 0; SubImageIdx < TextureDirectories.size(); SubImageIdx++) {

					for (auto Name : TextureNames) {

						if (Name == "Parallax.png")
							Name = "Height.png"; 

						auto& Directory = TextureDirectories[SubImageIdx];
						std::string FullPath = "Materials/" + Directory.BaseDirectory + "/";
						std::string FilePath = FullPath + Name;

						std::string FullPathStylized = "StylizedMaterials/" + Directory.BaseDirectory + "/lowres_" + Name; 

						ResizeAndSaveImage(FilePath, FullPathStylized, 32); 


					}

				}



			}



		}


	}

}