#pragma once

#include <assimp/cimport.h>
#include <assimp/mesh.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Core.h"
#include "CustomBlockModel.h"

namespace iTrace {

	namespace Rendering {
	
		namespace Chunk {

			void ConvertToModel(const std::string& Directory, const std::string& ModelName);
			void LoadModelData(CustomBlockModel& Model, const std::string& ModelName); 
		
		}
	}

}