#include "Pipeline.h"
#include <iostream>
#include "Sound.h"
using namespace iTrace; 


void ComputeDofKernels(float RAD_SCALE, float MAX_BLUR_SIZE) {

	std::vector<Vector2f> Kernels; 


	float Radius = RAD_SCALE; 

	float CurrentRadius = Radius; 
	float CurrentAngle = 0.0; 
	int i = 0; 
	for (float Angle = 0.0; Radius < MAX_BLUR_SIZE; Angle += (2.39996323 / 4.0)) {
		if (i % 4 == 0) {
			i = 0; 
			CurrentRadius = Radius; 
			CurrentAngle = Angle; 
		}
		i++; 


		Kernels.push_back(Vector2f(cos(Angle) * (Radius / MAX_BLUR_SIZE), sin(Angle) * (Radius / MAX_BLUR_SIZE)));
		Radius += RAD_SCALE / Radius; 

	}

	std::cout << "vec2 Kernels[" << Kernels.size() << "] = vec2[" << Kernels.size() << "](" << '\n'; 

	int kernelIdx = 1; 

	for (auto& Kernel : Kernels) {

		std::cout << "vec2(" << Kernel.x << ',' << Kernel.y << (kernelIdx == Kernels.size() ? ")" : "),\n"); 
		kernelIdx++; 
	}
	std::cout << ");"; 



}


int main(int *argc, char*argv[]) {

	alutInit(argc, argv); 

	//ComputeDofKernels(.1, 5.0); 
	Window Screen = Window(Vector2i(1920, 1080), true); 

	Camera Camera = iTrace::Camera(90.0, 0.01, 250., Vector3f(64.,128.0,64.0), Vector3f(0.), Screen); 

	iTrace::Pipeline Pipeline; 
	Pipeline.PreparePipeline(Camera, Screen); 
	Pipeline.RunPipeline(Camera, Screen); 
	
}
