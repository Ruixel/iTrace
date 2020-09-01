#pragma once

#include "CustomBlockModel.h"

namespace iTrace {
	namespace Rendering {

		namespace Chunk {

			const enum class BLOCK_MODEL {BASE=-1,FLOWER,WHEAT, STUMP, BUSH, VINES, MELON, SEAWEED, CORAL, CHEST, FENCE_SIDE1, FENCE_SIDE2, FENCE_FULL, FENCE_CORNER_1, FENCE_CORNER_2, FENCE_CORNER_3, FENCE_CORNER_4, FENCE_EMPTY, POT};

			void AddBlockModels(); 

		}

	}
}

