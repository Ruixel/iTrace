#pragma once

#include "text.h"

#define DISABLE_PROFILING 

namespace iTrace {


	namespace Profiler {

		void SetPerformance(const std::string& Name, float ElapsedTime, bool Exclusive = false); 
		void DrawProfiling(Window & Window,Rendering::TextSystem & Text);
		void FlushTime(); 
		void SetPerformance(const std::string Name); 

	}

}


