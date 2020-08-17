#include "MeshBaker.h"

namespace iTrace {
	namespace Rendering {
		namespace Chunk {

			struct MeshEntry {
				unsigned int NumIndices;
				unsigned int BaseVertex;
				unsigned int BaseIndex;
				unsigned int MaterialIndex;
				inline MeshEntry() :
					NumIndices(0),
					BaseVertex(0),
					BaseIndex(0),
					MaterialIndex(0)
				{}
			};

			struct Vertex {
				Vector3f Normal, Tangent, Position;
				Vector2f UV;
				unsigned char Padding[4];
			};

			void ConvertToModel(const std::string& Directory, const std::string& ModelName)
			{


				Assimp::Importer Importer;

				std::vector<MeshEntry> MeshEntries;
				std::vector<Vertex> PackedVertices, ActualVertices;

				const aiScene* Scene = Importer.ReadFile(Directory, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

				MeshEntries.resize(Scene->mNumMeshes);

				auto InitMesh = [&](const aiMesh* Mesh) {
				
					for (unsigned int i = 0; i < Mesh->mNumVertices; i++) {
						const aiVector3D* VertexPos = &(Mesh->mVertices[i]);
						const aiVector3D* VertexNormal = &(Mesh->mNormals[i]);
						const aiVector3D* VertexTextureCoordinate = Mesh->HasTextureCoords(0) ? &(Mesh->mTextureCoords[0][i]) : nullptr;
						const aiVector3D* VertexTangent = Mesh->HasTangentsAndBitangents() ? &(Mesh->mTangents[i]) : nullptr;

						//if (!aiMesh->HasTangentsAndBitangents())
						//	std::cout << "Warning: model does not have proper tangents!\n"; 

						Vertex Vert; 

						

						Vert.Position = Vector3f(VertexPos->x, VertexPos->y, VertexPos->z);
						Vert.Normal = Vector3f(VertexNormal->x, VertexNormal->y, VertexNormal->z);


						if (VertexTextureCoordinate)
						{
							Vert.UV = Vector2f(VertexTextureCoordinate->x, VertexTextureCoordinate->y);
						}
						else
						{
							Vert.UV = Vector2f(0.0);
						}

						Vert.UV.y = 1.0 - Vert.UV.y; 

						if (VertexTangent) {
							Vert.Tangent = glm::vec3(VertexTangent->x, VertexTangent->y, VertexTangent->z);
						}
						else {
							
							Vert.Tangent = Vector3f(0.0);

						}

						PackedVertices.push_back(Vert); 

					}

				}; 

				unsigned int NumVertices = 0;
				unsigned int NumIndices = 0;

				// Count the number of vertices and indices
				for (unsigned int i = 0; i < MeshEntries.size(); i++) {
					MeshEntries[i].MaterialIndex = Scene->mMeshes[i]->mMaterialIndex;
					MeshEntries[i].NumIndices = Scene->mMeshes[i]->mNumFaces * 3;
					MeshEntries[i].BaseVertex = NumVertices;
					MeshEntries[i].BaseIndex = NumIndices;

					NumVertices += Scene->mMeshes[i]->mNumVertices;
					NumIndices += MeshEntries[i].NumIndices;
				}

				for (unsigned int i = 0; i < MeshEntries.size(); i++) {
					const aiMesh* aiMesh = Scene->mMeshes[i];
					InitMesh(aiMesh);
				}

				for (int MeshEntry = 0; MeshEntry < MeshEntries.size(); MeshEntry++) {

					for (int Indicie = 0; Indicie < MeshEntries[MeshEntry].NumIndices; Indicie++) {
						ActualVertices.push_back(PackedVertices[MeshEntries[MeshEntry].BaseIndex + Indicie]); 
					}

				}

				Core::StoreToFile("Models/" + ModelName + ".model", ActualVertices); 

			}
			void LoadModelData(CustomBlockModel& Model, const std::string& ModelName)
			{
				auto Vertices = Core::GrabFromFile<Vertex>("Models/" + ModelName + ".model"); 

				for (auto& Vertex : Vertices) {

					Model.BlockData[0].Normals.push_back(Vertex.Normal);
					Model.BlockData[0].Vertices.push_back(Vertex.Position);
					Model.BlockData[0].Tangents.push_back(Vertex.Tangent);
					Model.BlockData[0].UVs.push_back(Vertex.UV); 

				}

			}
		}
	}
}