#include "CrosshairDrawer.h"
#include "FrameBuffer.h"

void iTrace::Rendering::CrosshairDrawer::PrepareCrosshairDrawer(Window& Window)
{
	UIShader = Shader("Shaders/UIShader"); 
	CrosshairTexture = LoadTextureGL("Resources/UI/Crosshair.png", GL_RGBA, false, false);

	Vector2f Aspect = Vector2f(float(Window.GetResolution().y) / float(Window.GetResolution().x), 1.0); 

	UIShader.Bind(); 

	UIShader.SetUniform("Position", Vector2f(0.0)); 
	UIShader.SetUniform("Scale", Vector2f(0.05) * Aspect);
	UIShader.SetUniform("Texture", 0);

	UIShader.UnBind(); 


}

void iTrace::Rendering::CrosshairDrawer::DrawCrosshair()
{

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

	UIShader.Bind(); 

	CrosshairTexture.Bind(0); 

	DrawPostProcessQuad(); 

	UIShader.UnBind(); 

	glDisable(GL_BLEND); 

}
