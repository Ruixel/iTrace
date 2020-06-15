#include "text.h"
#include "FrameBuffer.h"
#include <iostream>

iTrace::Rendering::TextSystem::TextSystem(const std::string & FontFile)
{
	FontShader = Shader("Shaders/TextShader"); 
	Font = LoadTextureGL(FontFile.c_str(), GL_RED, true, false); 
}

void iTrace::Rendering::TextSystem::DisplayText(const std::string & Text, Window & Window, float Size, Vector2f Position, Vector3f Color)
{
	
	glUniform2f(glGetUniformLocation(FontShader.ShaderID, "Size"), Size, Size*1.4*(float(Window.GetResolution().x) / float(Window.GetResolution().y)));
	
	int i = 0;
	for (auto Character : Text) {
		glUniform2f(glGetUniformLocation(FontShader.ShaderID, "Position"), Position.x + Size*i*2, Position.y);
		glUniform1i(glGetUniformLocation(FontShader.ShaderID, "Character"), int(Character) - 32);
		glUniform3f(glGetUniformLocation(FontShader.ShaderID, "Color"), Color.x, Color.y, Color.z);

		DrawPostProcessQuad(); 
		i++; 
	}
}
