#pragma once

#include "Shader.h"
#include "Texture.h"

namespace iTrace {
	namespace Rendering {
		class TextSystem {
			TextureGL Font; 
			Shader FontShader; 
		public: 
			TextSystem() {}
			TextSystem(const std::string &FontFile);
			void DisplayText(const std::string & Text, Window & Window,float Size = 0.01f, Vector2f Position = Vector2f(0.), Vector3f Color = Vector3f(0.2));
			void PrepareTextDrawing() { FontShader.Bind(); glActiveTexture(GL_TEXTURE31); Font.Bind(31); glUniform1i(glGetUniformLocation(FontShader.ShaderID, "Font"), 31); }
			void EndTextDrawing() { FontShader.UnBind(); }

		};
	}
}
