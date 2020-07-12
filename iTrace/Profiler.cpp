#include "Profiler.h"
#include <map>

//#undef DISABLE_PROFILING 

struct PerformanceData {
	float Time; 
	bool Exclusive; 
};

std::map<std::string, PerformanceData> Performance;

sf::Clock InternalClock; 

void iTrace::Profiler::SetPerformance(const std::string& Name, float ElapsedTime, bool Exclusive)
{




	if (Performance.find(Name) == Performance.end()) {
		Performance[Name] = { ElapsedTime, Exclusive };
	}
	else {

		Performance[Name].Time = glm::mix(Performance[Name].Time, ElapsedTime, 0.05); 
		Performance[Name].Exclusive = Exclusive; 

	}

}

void iTrace::Profiler::DrawProfiling(Window& Window, Rendering::TextSystem& Text)
{

#ifndef DISABLE_PROFILING 

	float TotalTime = 0.0f; 
	float Offset = .95; 
	float Height = 0.05; 

	for (auto& Perf : Performance) {

		std::string Name = Perf.first + " time = " + std::to_string(Perf.second.Time); 

		Text.DisplayText(Name, Window, 0.005, Vector2f(-.97, Offset), Vector3f(1.0)); 

		if (!Perf.second.Exclusive)
			TotalTime += Perf.second.Time;

		Offset -= Height;

	}

	std::string Name = "Total time = " + std::to_string(TotalTime); 

	Text.DisplayText(Name, Window, 0.005, Vector2f(-.97, Offset), Vector3f(1.0)); 
#endif

}

void iTrace::Profiler::FlushTime()
{
#ifndef DISABLE_PROFILING 
	glFinish(); 
	InternalClock.restart(); 
#endif

}

void iTrace::Profiler::SetPerformance(const std::string Name)
{
#ifndef DISABLE_PROFILING 
	glFinish(); 
	SetPerformance(Name, InternalClock.getElapsedTime().asSeconds() * 1000.0f); 
	InternalClock.restart(); 
#endif
}
