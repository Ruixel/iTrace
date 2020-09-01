#include "BlockModels.h"
#include "MeshBaker.h"

void iTrace::Rendering::Chunk::AddBlockModels()
{
	ConvertToModel("Models/Raw/Flower.obj", "Flower");
	ConvertToModel("Models/Raw/Wheat.obj", "Wheat");
	ConvertToModel("Models/Raw/Stump.obj", "Stump");
	ConvertToModel("Models/Raw/Bush.obj", "Bush");
	ConvertToModel("Models/Raw/Vines.obj", "Vines");
	ConvertToModel("Models/Raw/Melon.obj", "Melon");
	ConvertToModel("Models/Raw/Seaweed.obj", "Seaweed");
	ConvertToModel("Models/Raw/Coral.obj", "Coral");
	ConvertToModel("Models/Raw/Chest.obj", "Chest");


	ConvertToModel("Models/Raw/Fences/Fence_Side_1.obj", "Fence_Side1");
	ConvertToModel("Models/Raw/Fences/Fence_Side_2.obj", "Fence_Side2");

	ConvertToModel("Models/Raw/Fences/Fence_Full.obj", "Fence_Full");

	ConvertToModel("Models/Raw/Fences/Fence_Corner_1.obj", "Fence_Corner1");
	ConvertToModel("Models/Raw/Fences/Fence_Corner_2.obj", "Fence_Corner2");
	ConvertToModel("Models/Raw/Fences/Fence_Corner_3.obj", "Fence_Corner3");
	ConvertToModel("Models/Raw/Fences/Fence_Corner_4.obj", "Fence_Corner4");

	ConvertToModel("Models/Raw/Fences/Fence_Empty.obj", "Fence_Empty");

	ConvertToModel("Models/Raw/Pot.obj", "Pot");


	LoadModelData(GetModelData(GenerateModelInstance()), "Flower");
	LoadModelData(GetModelData(GenerateModelInstance()), "Wheat");
	LoadModelData(GetModelData(GenerateModelInstance()), "Stump");
	LoadModelData(GetModelData(GenerateModelInstance()), "Bush");
	LoadModelData(GetModelData(GenerateModelInstance()), "Vines");
	LoadModelData(GetModelData(GenerateModelInstance()), "Melon");
	LoadModelData(GetModelData(GenerateModelInstance()), "Seaweed");
	LoadModelData(GetModelData(GenerateModelInstance()), "Coral");
	LoadModelData(GetModelData(GenerateModelInstance()), "Chest");



	//Fences: 

	LoadModelData(GetModelData(GenerateModelInstance()), "Fence_Side1");
	LoadModelData(GetModelData(GenerateModelInstance()), "Fence_Side2");

	LoadModelData(GetModelData(GenerateModelInstance()), "Fence_Full");

	LoadModelData(GetModelData(GenerateModelInstance()), "Fence_Corner1");
	LoadModelData(GetModelData(GenerateModelInstance()), "Fence_Corner2");
	LoadModelData(GetModelData(GenerateModelInstance()), "Fence_Corner3");
	LoadModelData(GetModelData(GenerateModelInstance()), "Fence_Corner4");
	
	LoadModelData(GetModelData(GenerateModelInstance()), "Fence_Empty");

	LoadModelData(GetModelData(GenerateModelInstance()), "Pot");



}
