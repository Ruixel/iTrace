#pragma once
#include "Core.h"
#include <array>
#include <vector>

namespace iTrace {

	namespace Rendering {

		namespace Chunk {


			struct CustomBlockModelPlane {
				Vector3f Position, Rotation;
				Vector2f Scale; 
				//Other data: 
				Vector3f Normal, Tangent, Bitangent; 

				int Face = -1; 
				//for some models, you may want certain planes to go away if a block from a certain location is there 


				CustomBlockModelPlane(Vector3f Position, Vector3f Rotation, Vector2f Scale, int Face);

				void AddToModelData(std::vector<Vector3f>& Tangents, std::vector<Vector3f>& Normals, std::vector<Vector3f>& Vertices, std::vector<Vector2f>& UVs); 

			};

			struct CustomBlockModelAABB {
				Vector3f Scale, Position;
				std::array<bool, 6> WillBeDiscarded = { false,false,false,false,false,false }; //<- notifies if the sides of the AABB can be discarded based on sorrounding blocks 

				void AddToModelData(std::vector<Vector3f>& Tangents, std::vector<Vector3f>& Normals, std::vector<Vector3f>& Vertices, std::vector<Vector2f>& UVs, std::vector<unsigned int> & Indicies, unsigned int & Index, std::array<bool, 6> & Visible);

			};


			struct CustomBlockModelData {
				std::vector<Vector3f> Tangents = {}, Normals = {}, Vertices = {};
				std::vector<Vector2f>  UVs = {};

				void AddToModelData(std::vector<Vector3f>& Tangents, std::vector<Vector3f>& Normals, std::vector<Vector3f>& Vertices, std::vector<Vector2f>& UVs, std::vector<unsigned int>& Indicies, unsigned int& Index);

			};

			struct CustomBlockModel {
				std::array<bool, 6> CanDiscard = { false,false,false,false,false,false }; //notes if this model can indeed cut of block faces 
				std::vector<CustomBlockModelAABB> AABBs; 
				std::vector<CustomBlockModelPlane> Planes;

				std::array< CustomBlockModelData, 7> BlockData; 
				


				std::vector<Vector3f> GetBlockTangents(); 
				std::vector<Vector3f> GetBlockNormals(); 
				std::vector<Vector3f> GetBlockVertices(); 
				std::vector<Vector2f> GetBlockUVs(); 

				void GenerateModelData(); //<- -1 if its a solid block, otherwise represents the model indicies at the locations 

				void AddToModelData(std::vector<Vector3f>& Tangents, std::vector<Vector3f>& Normals, std::vector<Vector3f>& Vertices, std::vector<Vector2f>& UVs, std::vector<unsigned int>& Indicies, unsigned int& Index, std::array<int, 6> & ModelIndicies); 
				void AddPlane(CustomBlockModelPlane Plane); 

			protected: 

				std::vector<Vector3f> Tangents = {}, Normals = {}, Vertices = {};
				std::vector<Vector2f> UVs = {};
				


				bool HasRT = false;
			};

			int GenerateModelInstance();
			CustomBlockModel& GetModelData(int Index); 
			std::string GetModelInjectionCode(); //<- gives the shader injection code required to traverse custom block models 



		}

	}

}