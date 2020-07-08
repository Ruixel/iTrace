#include "RainDropBaker.h"
#include <fstream>
#include <iostream>

void iTrace::Rendering::RainBaker::BakeRain()
{

	glDisable(GL_DEPTH_TEST); 
	glClearColor(1.0, 0.0, 0.0, 0.0); 

	//check if all frames are here! (assume res is ok!) 


	for (int i = RAINDROP_FRAMES-1; i >= 0; i--) {

		std::string Title = "Resources/BakeData/RainDrop_" + std::to_string(i) + ".png"; 

		std::ifstream stream(Title); 

		if (stream.good())
			return; //<- we have all the textures, no need to bake 

	}

	Shader RainBakingShader = Shader("Shaders/RainBaker"); 
	FrameBufferObject RainBakingFBO = FrameBufferObject(Vector2i(RAINDROP_TEX), GL_RGBA8, false); 
	
	//auto Pixels = std::vector<unsigned char>(RAINDROP_TEX * RAINDROP_TEX * 4, 255); 
	auto Pixels = new unsigned char[RAINDROP_TEX * RAINDROP_TEX * 4]; 

	for (int i = 0; i < RAINDROP_FRAMES; i++) {

		std::string Title = "Resources/BakeData/RainDrop_" + std::to_string(i) + ".png";

		RainBakingShader.Bind();

		RainBakingFBO.Bind();

		RainBakingShader.SetUniform("Time", float(i) / float(RAINDROP_FRAMES)); 

		DrawPostProcessQuad(); 

		RainBakingFBO.UnBind();

		RainBakingShader.UnBind();

		glFinish(); 

		glBindTexture(GL_TEXTURE_2D, RainBakingFBO.ColorBuffer); 
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels); 

		glFinish(); 

		
		sf::Image SaveImage; 

		SaveImage.create(RAINDROP_TEX, RAINDROP_TEX, Pixels); 
		SaveImage.saveToFile(Title); 

	}

	delete[] Pixels; 

	//Pixels.clear(); 

}
