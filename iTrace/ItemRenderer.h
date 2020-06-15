#pragma once

#include "FrameBuffer.h"
#include "Texture.h"
#include "Camera.h"


namespace iTrace {
	
	namespace Rendering {
		
#define ITEM_ICON_RESOLUTION 2048

		const enum class ItemRequestType : int {
			Block
		};

		void AddItemIconRequest(ItemRequestType Type, int ID, std::string Title); 
		void RenderItemRequests(); 

		struct ItemIconRequest {

			ItemRequestType Type = ItemRequestType::Block;
			int ID; 
			std::string Title = "None"; 

		};

		struct ItemRenderer {




		};


	}

}


