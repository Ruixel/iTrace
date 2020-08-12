#include "BlockModels.h"

void iTrace::Rendering::Chunk::AddBlockModels()
{

	auto Foliage = GenerateModelInstance(); 

	auto& FoliageModel = GetModelData(Foliage); 
	
	FoliageModel.AddPlane(CustomBlockModelPlane(Vector3f(0.0, -0.40200681803, 0.533432 * 0.5), Vector3f(0.0,0.0,0.0f), Vector2f(0.862211, 0.31094),Vector2f(2.772, 1.0f), -1));
	FoliageModel.AddPlane(CustomBlockModelPlane(Vector3f(-0.211105 * 0.5, -0.40200681803, 0.29546 * 0.5), Vector3f(0.0, 46.0553, 0.0f), Vector2f(0.862211, 0.31094), Vector2f(2.772, 1.0f), -1));
	FoliageModel.AddPlane(CustomBlockModelPlane(Vector3f(0.376151 * 0.5, -0.40200681803, 0.038295 * 0.5), Vector3f(0.0, 57.2927, 0.0f), Vector2f(0.862211, 0.31094), Vector2f(2.772, 1.0f), -1));
	FoliageModel.AddPlane(CustomBlockModelPlane(Vector3f(0.184237 * 0.5, -0.40200681803, 0.115061 * 0.5), Vector3f(0.0, -43.0325, 0.0f), Vector2f(0.862211, 0.31094), Vector2f(2.772, 1.0f), -1));
	FoliageModel.AddPlane(CustomBlockModelPlane(Vector3f(-0.11131 * 0.5, -0.40200681803, -0.049985 * 0.5), Vector3f(0.0, -51.5044, 0.0f), Vector2f(0.862211, 0.31094), Vector2f(2.772, 1.0f), -1));

	FoliageModel.GenerateModelData(); 
}
