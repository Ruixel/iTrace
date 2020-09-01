#include "ShaderPacker.h"
#include "PackedShaders.h"
#include <fstream>
#include <iostream>

namespace iTrace {

	namespace Rendering {


		std::map<std::string, std::string> InternalOutPut; 

		void PrepareShaderLoading()
		{
			//LoadShaderBinaries(); 
		}

		void UpdateShaderBinaryStructure()
		{

			std::ofstream File("PackedShaders.h"); 

			File << R"(/*
AUTO-GENERATED HEADER FILE. 
NO CHANGES HERE WILL BE SAVED. 
PLEASE NOTE: 
ALL OF THE FOLLOWING SHADERS ARE ALL RIGHTS RESERVED 
MODIFICATION, REDISTRIBUTION, COPYING and/or EXTRACTION of ANY part of ANY of these shaders 
without EXPLICIT and CLEAR PERMISSION is STRICTLY prohibited. 
Thread carefully!
*/
#include <map>
#include <string>
std::map<std::string, std::string> ShaderOutPut;
void LoadShaderBinaries() {
)"; 

			for (auto& OutPut : InternalOutPut) {
			//	File << "ShaderOutPut[" << '"' << OutPut.first << '"' << "] = R" << '"' << "(\n" << OutPut.second << "\n)" << '"' << ";\n"; 
			}

			File << "\n}"; 
			
			File.close(); 

		}

		void LoadShader(std::string Path, std::string Inject)
		{
			std::ifstream Source(Path); 

			std::string SourceString = ""; 

			if (!Source.is_open()) {

				if (ShaderOutPut.find(Path) == ShaderOutPut.end())
					std::cout << "Failed to load shader: " << Path << '\n';
				else
					InternalOutPut[Path] = ShaderOutPut[Path]; 
			}
			else {
				
				std::string Line;

				while (std::getline(Source, Line)) {

					if (SourceString.size() == 0) {

						SourceString += Line + '\n'; 
						SourceString += Inject + '\n'; 

					}
					else {
						SourceString += Line + '\n';
					}
					
				}

				InternalOutPut[Path] = SourceString; 

			}
			



		}

		std::string GetShader(std::string Path)
		{
			return InternalOutPut[Path]; 
		}

	}

}