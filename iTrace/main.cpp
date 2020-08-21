#include "Pipeline.h"
#include <iostream>
#include "Sound.h"
using namespace iTrace; 




Vector3f LineLine(Vector2f p1, Vector2f p2, Vector2f p3, Vector2f p4) {
	
	float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
	float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;

	float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	// If d is zero, there is no intersection
	if (d == 0) return Vector3f(-100.0);

	// Get the x and y
	float pre = (x1 * y2 - y1 * x2), post = (x3 * y4 - y3 * x4);
	float x = (pre * (x3 - x4) - (x1 - x2) * post) / d;
	float y = (pre * (y3 - y4) - (y1 - y2) * post) / d;

	// Check if the x and y coordinates are within both lines
	if (x < glm::min(x1, x2) || x > glm::max(x1, x2) ||
		x < glm::min(x3, x4) || x > glm::max(x3, x4)) return Vector3f(-100.0);
	if (y < glm::min(y1, y2) || y > glm::max(y1, y2) ||
		y < glm::min(y3, y4) || y > glm::max(y3, y4)) return Vector3f(-100.0);

	// Return the point of intersection
	return Vector3f(x,y,1.0);
}

constexpr int NUM_BLADES = 3; 


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

		float AngleClamped = int((Angle/6.28) * NUM_BLADES); 


		float AngleNext = float(AngleClamped + 1) / NUM_BLADES; 
		AngleClamped /= float(NUM_BLADES);

		Vector2f OuterLine1 = Vector2f(cos(AngleClamped*6.28), sin(AngleClamped * 6.28));
		Vector2f OuterLine2 = Vector2f(cos(AngleNext * 6.28), sin(AngleNext * 6.28));

		Vector2f InnerLine1 = Vector2f(0.0); 
		Vector2f InnerLine2 = Vector2f(cos(Angle), sin(Angle)); 

		Vector2f Intersect = Vector2f(LineLine(OuterLine1, OuterLine2, InnerLine1, InnerLine2)); 

		Vector2f Point = InnerLine2 * length(Intersect); 

		Kernels.push_back(Vector2f(Point.x * (Radius / MAX_BLUR_SIZE), Point.y * (Radius / MAX_BLUR_SIZE)));
		Radius += (RAD_SCALE)/ Radius ;

	}

	std::cout << "vec2 Kernels[" << Kernels.size() << "] = vec2[" << Kernels.size() << "](" << '\n';

	int kernelIdx = 1; 
	int _s = 0; 
	for (auto& Kernel : Kernels) {

		std::cout << "vec2(" << Kernel.x << ',' << Kernel.y << (kernelIdx == Kernels.size() ? ")" : "),\n"); 
		kernelIdx++;

	}
	std::cout << ");"; 



}


float RRTAndODTFit(float v) {
	float a = v * (v + 0.0245786f) - 0.000090537f;
	float b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
	return a / b;
}

float ODTtoRRT(float v) {

	float f1 = 0.4329510f * v - 0.0245786f;
	float f2 = 4.0 * (0.983729f * v - 1) * (0.238081f * v + 0.000090537f);
	float f3 = 0.4329510f * v;
	float f4 = 0.0245786f;

	return (-sqrt(f1 * f1 - f2) - f3 + f4) / (2.0 * (0.983729f * v - 1));

}




using namespace glm; 



vec2 wavedx(vec2 position, vec2 direction, float speed, float frequency, float timeshift) {
	float x = dot(direction, position) * frequency + timeshift * speed;
	float wave = exp(sin(x) - 1.0);
	float dx = wave * cos(x);
	return vec2(wave, -dx);
}

float getwaves(vec2 position, int iterations, float Time) {
	float iter = 0.0;
	float phase = 6.0;
	float speed = 2.0;
	float weight = 1.0;
	float w = 0.0;
	float ws = 0.0;
	for (int i = 0; i < iterations; i++) {
		vec2 p = vec2(sin(iter), cos(iter));
		vec2 res = wavedx(position, p, speed, phase, Time);
		position += normalize(p) * res.y * weight * 0.05f;
		w += res.x * weight;
		iter += 12.0;
		ws += weight;
		weight = mix(weight, 0.0f, 0.2f);
		phase *= 1.18;
		speed *= 1.07;
	}
	return w / ws;
}



int main(int *argc, char*argv[]) {
	
	//std::cout << ODTtoRRT(0.75f) / AverageLum << '\n';
	//std::cin.get(); 



	//ComputeDofKernels(1.0/float(8.0), 8.0);


	alutInit(argc, argv); 

	Window Screen = Window(Vector2i(1920, 1080), true); 
	Camera Camera = iTrace::Camera(90.0, 0.01, 250., Vector3f(128.0 + 64.0,128.0,128.0 + 64.0), Vector3f(0.), Screen); 

	iTrace::Pipeline Pipeline; 
	Pipeline.PreparePipeline(Camera, Screen); 
	Pipeline.RunPipeline(Camera, Screen); 
}
