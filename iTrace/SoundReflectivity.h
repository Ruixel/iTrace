#pragma once
#include <string>
#include <array>


namespace iTrace {

	const enum class SoundType { STONE, DIRT, GRASS, GRAVEL, WOOD, METAL, FABRIC, SNOW, SAND, SIZE, NONE };

	struct SoundMaterialPropertie {

		std::string ExtensionName;
		int StepCount;
		float Reflectivity;

	};

	const std::array<SoundMaterialPropertie, static_cast<int>(SoundType::SIZE)> SoundMaterialTypes = {
		SoundMaterialPropertie{"Stone", 5, 1.0},
		SoundMaterialPropertie{"Dirt", 5,0.3},
		SoundMaterialPropertie{"Grass", 5, 0.3},
		SoundMaterialPropertie{"Gravel", 5, 0.5},
		SoundMaterialPropertie{"Wood", 5, 0.4},
		SoundMaterialPropertie{"Metal", 5, 1.0},
		SoundMaterialPropertie{"Default", 5, 0.05},
		SoundMaterialPropertie{"Snow", 5, 0.3},
		SoundMaterialPropertie{"Sand", 5, 0.3}
	};


}

