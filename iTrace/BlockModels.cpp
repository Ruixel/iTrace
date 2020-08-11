#include "BlockModels.h"

void iTrace::Rendering::Chunk::AddBlockModels()
{

	auto Foliage = GenerateModelInstance(); 

	auto& FoliageModel = GetModelData(Foliage); 
	
	FoliageModel.AddPlane(CustomBlockModelPlane(Vector3f(0.0), Vector3f(0.0), Vector2f(1.0), -1)); 

}
