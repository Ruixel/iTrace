#pragma once

#include <string>
#include <map>

namespace iTrace {

	namespace Rendering {

		const enum class ShaderType {
			VERTEX, FRAGMENT
		};

		void PrepareShaderLoading(); 
		void UpdateShaderBinaryStructure(); //<- updates the packedshader header file 
		void LoadShader(std::string Path, std::string Inject);
		std::string GetShader(std::string Path); 


	}
	
}


