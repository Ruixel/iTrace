#include "BlockModels.h"
#include "MeshBaker.h"

void iTrace::Rendering::Chunk::AddBlockModels()
{
	ConvertToModel("Models/Raw/Flower.obj", "Flower");
	ConvertToModel("Models/Raw/Wheat.obj", "Wheat");
	ConvertToModel("Models/Raw/Stump.obj", "Stump");
	ConvertToModel("Models/Raw/Bush.obj", "Bush");
	ConvertToModel("Models/Raw/Vines.obj", "Vines");

	LoadModelData(GetModelData(GenerateModelInstance()), "Flower");
	LoadModelData(GetModelData(GenerateModelInstance()), "Wheat");
	LoadModelData(GetModelData(GenerateModelInstance()), "Stump");
	LoadModelData(GetModelData(GenerateModelInstance()), "Bush");
	LoadModelData(GetModelData(GenerateModelInstance()), "Vines");


}
