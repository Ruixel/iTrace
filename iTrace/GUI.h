#pragma once

#include "Shader.h"
#include "Window.h"
#include "Texture.h"

namespace iTrace {

	namespace Rendering {


		enum class ElementType {EMPTY, TEXTURE};

		enum class AlphaMode {ALPHA, COLORDISCARD};

		struct UIElement {
			virtual ElementType GetElementType() {
				return ElementType::EMPTY; 
			}
			virtual void Draw(Window& Window) = 0; 
		};

		struct TextureUI : UIElement {

			AlphaMode Mode = AlphaMode::ALPHA; 
			Vector3f DiscardColor = Vector3f(0.0); 

			TextureGL& Texture;

			TextureUI(TextureGL& Texture, Vector2f Size, Vector2f Position); 
			void SetAlphaMode(AlphaMode Mode, Vector3f DiscardColor = Vector3f(0.0)); 
			void Draw(Window& Window); 
			ElementType GetElementType() {
				return ElementType::TEXTURE; 
			}

		};



	}

}