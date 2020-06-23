#pragma once 
#include "Texture.h"
#include <vector>
#include <unordered_map>
#include "Camera.h"
#include "FrameBuffer.h"
#include <stdint.h>
#include <array>

/*

*/




namespace iTrace {
	namespace Rendering {
		namespace Chunk {

#define CHUNK_SIZE 128
#define TEXTURE_RES 512
#define LONGESTLIGHT 10
#define MAX_GREEDY 128

			const Vector3f BlockNormals[6] = {
				Vector3f(1.0, 0.0, 0.0),
				Vector3f(-1., 0.0, 0.0),
				Vector3f(0.0, 1.0, 0.0),
				Vector3f(0.0, -1., 0.0),
				Vector3f(0.0, 0.0, 1.0),
				Vector3f(0.0, 0.0, -1.)
			};

			const enum class TextureType : std::uint8_t { ALBEDO, NORMAL, ROUGHNESS, DISPLACEMENT, EMISSION, METALNESS, SIZE };
			const enum class TextureExtension : std::uint8_t {DISPLACEMENT, EMISSION, METALNESS, SIZE};
			const enum class BLOCK_ACTION : std::uint8_t {BREAK, PLACE};
			const std::array<std::string, static_cast<int>(TextureType::SIZE)> TextureNames = { "Albedo.png", "Normal.png", "Roughness.png", "Parallax.png","Emission.png", "Metalness.png"}; 
			
			const std::vector<std::vector<std::string>> Keywords = { {"col", "color", "alb", "albedo", "diffuse", "diff","_a","_c"}, {"norm", "normal", "_n"},{"rough", "roughness", "_r", "_s"} , {"displace", "disp", "height", "_h"}, {"emission", "emissive","emis","_e", "em"}, {"met", "metalness", "metallic", "spec", "specular", "_m"} };


			struct TextureDir {

				std::string BaseDirectory = "";
				float ParallaxStrenght = 0.0f; 
				std::array<int, static_cast<int>(TextureExtension::SIZE)> Extensions = {-1,-1, -1};
				Vector3f EmissiveAverage = Vector3f(0.0); 

				TextureDir(std::string BaseDir, float ParallaxStrenght) :
					BaseDirectory(BaseDir), ParallaxStrenght(ParallaxStrenght) {
					BaseDirectory = BaseDir; 
				}

				TextureDir operator=(const std::string& s) {
					return TextureDir(s,0.f); 
				}

			};

			struct BlockType {
				std::string Name = "Air"; 
				unsigned char TexIds[6] = { 0,0,0,0,0,0 }; 
				bool IsSolid = false; 
				bool IsEmpty = true; //i;e "empty" blocks get treated as air in the ray tracing process.
				//these blocks also have the effect 
				bool IsNone = true; //is just nothing
				bool IsEmissive = false; 
				bool IsMetallic = false; 
				float EmissiveStrength = 0.0; 
				BlockType() {}
				BlockType(std::string Name, std::vector<unsigned char> TexIds, bool IsSolid, bool IsEmpty, bool IsNone, float EmissiveStrength=0.0) : 
					Name(Name),
					
					IsSolid(IsSolid),
					IsEmpty(IsEmpty),
					IsNone(IsNone),
					EmissiveStrength(EmissiveStrength),
					IsEmissive(EmissiveStrength>0.1)
				{
					for (int i = 0; i < 6; i++) {
						this->TexIds[i] = TexIds[glm::min(i, (int)(TexIds.size() - 1))]; 
					}


				}

			};



			void AddBlock(BlockType Type); 
			BlockType &GetBlock(unsigned char Idx); 
			unsigned char GetBlockSize(); 
			void AddTexture(const std::string& Directory, float ParallaxStrenght = 1.0f); 
			void GenerateBlockTextureData(); 
			unsigned int GetTextureArrayList(unsigned char Idx); 
			unsigned int GetBlockDataTexture(); 
			unsigned int GetTextureExtensionData(); 
			unsigned int GetBlockExtraDataTexture();

			TextureDir GetTextureData(unsigned char Idx);
		
			struct BlockMask {

				std::array<bool, 6> Mask = { false,false,false,false,false,false };

				void SetMask(int idx, bool flag) {
					Mask[idx] = flag; 
				}
				bool GetMask(int idx) {
					return Mask[idx];
				}



			};

			struct Chunk {

				std::vector<unsigned char> Blocks; //size is CHUNK_SIZE^3
				std::vector<Vector4f> BlockLighting; 
				
				std::vector<unsigned char> TallestBlock; 
				std::vector<BlockMask> Mask;

				unsigned int ChunkTexID, ChunkLightTexID; //the 3D texture used for tracing. Weighs ~2.62 mb of vram 
				unsigned int ChunkVAOID, ChunkVBOID[3]; //VBO IDs -> Triangles, Normals, Texture Coordinates (z component stores material), Tangents, Indicies 
				long long X, Y; 
				unsigned int Vertices = 0; 
				
				void Draw(Camera& Camera); //draws chunk data to  
				void Generate(std::vector<Chunk*> NeighbooringChunks);
				void UpdateMeshData(std::vector<Chunk*> NeighbooringChunks); 
				void SetBlock(unsigned char x, unsigned char y, unsigned char z, unsigned char type); 
				unsigned char GetBlock(unsigned char x, unsigned char y, unsigned char z) {
					return Blocks[x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z]; 
				}
				unsigned char GetTallestBlock(unsigned char x, unsigned char z) {
					return TallestBlock[x * CHUNK_SIZE + z]; 
				}
				void SetTallestBlock(unsigned char x, unsigned char y, unsigned char z) {
					TallestBlock[x * CHUNK_SIZE + z] = y; 
				}
				

				//returns 0 if the distance was too short, 1 if the action was carried out and -1 if it exited the chunk
				Vector3i CastBlock(Vector3f Origin, Vector3f Direction, BLOCK_ACTION Action, unsigned short Distance, int & side);


				Chunk(long long X, long long Y); 

				void DumpToFile(); 
				void LoadFromFile(); 
				void ManageCollision(Vector3f& Position, Vector3f & Acceleration, Vector3f & Velocity); 
			
			};
			
			
		}
	}
}