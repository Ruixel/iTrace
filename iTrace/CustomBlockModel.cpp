#include "CustomBlockModel.h"

namespace iTrace {

	namespace Rendering {

		namespace Chunk {


			std::vector<CustomBlockModel> Models; 

			CustomBlockModelPlane::CustomBlockModelPlane(Vector3f Position, Vector3f Rotation, Vector2f Scale, Vector2f UVScale, int Face) : 
				Position(Position), Rotation(Rotation), Scale(Scale), UVScale(UVScale), Face(Face)
			{
				

			}

			void CustomBlockModelPlane::AddToModelData(std::vector<Vector3f>& Tangents, std::vector<Vector3f>& Normals, std::vector<Vector3f>& Vertices, std::vector<Vector2f>& UVs)
			{
				
				auto BlockModelDataUVSpace = std::vector<Vector3f>{
					Vector3f(0.0,0.0,0.0), 
					Vector3f(1.0,0.0,0.0),
					Vector3f(1.0,1.0,0.0),
					Vector3f(1.0,1.0,0.0),
					Vector3f(0.0,1.0,0.0),
					Vector3f(0.0,0.0,0.0)
				}; 

				auto BlockModelDataLocalSpace = std::vector<Vector3f>{
					Vector3f(-0.5,-0.5,0.0),
					Vector3f(0.5,-0.5,0.0),
					Vector3f(0.5,0.5,0.0),
					Vector3f(0.5,0.5,0.0),
					Vector3f(-0.5,0.5,0.0),
					Vector3f(-0.5,-0.5,0.0)
				};

				auto Normal = Vector3f(0.0, 0.0, 1.0); 
				auto Tangent = Vector3f(0.0, 1.0, 0.0); 
				

				auto BlockModelDataWorldSpace = BlockModelDataLocalSpace; 



				Matrix4f Matrix = Core::ModelMatrix(-Position, Rotation, Vector3f(Scale, 1.0f)); 
				Matrix3f NormalMatrix = glm::inverse(glm::transpose(Matrix3f(Matrix)));
				//see: https://computergraphics.stackexchange.com/questions/7570/how-do-i-build-my-tangent-space-matrix
				Matrix3f TangentMatrix = Matrix3f(Matrix); 

				for (auto& Comp : BlockModelDataWorldSpace) {
					Comp = Vector3f(Matrix * Vector4f(Comp, 1.0f)); 
				}

				Normal = glm::normalize(NormalMatrix * Normal);
				Tangent = glm::normalize(NormalMatrix * Tangent); 

				//add all the faces to the 

				for (int FaceIdx = 0; FaceIdx < 6; FaceIdx++) {
					Tangents.push_back(Tangent); 
					Normals.push_back(Normal); 
					UVs.push_back(Vector2f(BlockModelDataUVSpace[FaceIdx].x, 1.0-BlockModelDataUVSpace[FaceIdx].y)*UVScale);
					Vertices.push_back(BlockModelDataWorldSpace[FaceIdx]); 
				}

			}

			int GenerateModelInstance()
			{
				Models.push_back(CustomBlockModel()); 
				return Models.size() - 1; 
			}

			CustomBlockModel& GetModelData(int Index)
			{
				return Models[Index]; 
			}

			std::string GetModelInjectionCode()
			{
				return std::string();
			}

			std::vector<Vector3f> CustomBlockModel::GetBlockTangents()
			{
				return Tangents; 
			}

			std::vector<Vector3f> CustomBlockModel::GetBlockNormals()
			{
				return Normals; 
			}

			std::vector<Vector3f> CustomBlockModel::GetBlockVertices()
			{
				return Vertices; 
			}

			std::vector<Vector2f> CustomBlockModel::GetBlockUVs()
			{
				return UVs; 
			}

			void CustomBlockModel::GenerateModelData()
			{

				for (auto& Plane : Planes) {

					auto& Data = BlockData[Plane.Face + 1]; 

					Plane.AddToModelData(Data.Tangents, Data.Normals, Data.Vertices, Data.UVs);
				}

			}

			void CustomBlockModel::AddToModelData(Vector3f OriginShift, std::vector<Vector3f>& Tangents, std::vector<Vector3f>& Normals, std::vector<Vector4f>& Vertices, std::vector<Vector3f>& UVs, std::vector<unsigned int>& Indicies, unsigned int& Index, std::array<int, 6> & ModelIndicies, int BlockIndex)
			{
				std::array<bool, 6> Cull; 
				bool FullyCulled = true; 
				for (int i = 0; i < 6; i++) {

					bool Culled = false; 

					if (ModelIndicies[i] == -1) {
						Culled = true; 
					}
					else {
						Culled = GetModelData(ModelIndicies[i]).CanDiscard[(i / 2) * 2 + !(i % 2)]; 
					}

					Cull[i] = Culled; 

					FullyCulled = FullyCulled && Culled; 

				}

				//if (FullyCulled)
				//	return; 

				for (int i = 0; i < 7; i++) {

					bool Visible = true; 
					//if (i > 0)
					//	Visible = Cull[i - 1]; 

					auto& ModelData = BlockData[i]; 

					ModelData.AddToModelData(OriginShift, Tangents, Normals, Vertices, UVs, Indicies, Index,BlockIndex); 

				}

			}

			void CustomBlockModel::AddPlane(CustomBlockModelPlane Plane)
			{
				Planes.push_back(Plane); 
			}

			void CustomBlockModelAABB::AddToModelData(std::vector<Vector3f>& Tangents, std::vector<Vector3f>& Normals, std::vector<Vector3f>& Vertices, std::vector<Vector2f>& UVs, std::vector<unsigned int>& Indicies, unsigned int& Index, std::array<bool, 6> & Visible)
			{

			}

			void CustomBlockModelData::AddToModelData(Vector3f OriginShift, std::vector<Vector3f>& Tangents, std::vector<Vector3f>& Normals, std::vector<Vector4f>& Vertices, std::vector<Vector3f>& UVs, std::vector<unsigned int>& Indicies, unsigned int& Index, int BlockIndex)
			{

				for (int VertexIdx = 0; VertexIdx < this->Tangents.size(); VertexIdx++) {

					Tangents.push_back(this->Tangents[VertexIdx]); 
					Normals.push_back(this->Normals[VertexIdx]);
					Vertices.push_back(Vector4f(this->Vertices[VertexIdx] + OriginShift,0));
					UVs.push_back(Vector3f(this->UVs[VertexIdx], BlockIndex));

					Indicies.push_back(Index++);


				}



			}

		}
	}
}