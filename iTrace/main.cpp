#include "Pipeline.h"
#include <iostream>
using namespace iTrace; 


int main() {
	
	Window Screen = Window(Vector2i(1920, 1080), true); 

	Camera Camera = iTrace::Camera(90.0, 0.01, 250., Vector3f(64.), Vector3f(0.), Screen); 

	iTrace::Pipeline Pipeline; 
	Pipeline.PreparePipeline(Camera, Screen); 
	Pipeline.RunPipeline(Camera, Screen); 
	
}