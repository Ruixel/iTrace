#pragma once

#include <string>
#include <vector>

namespace iTrace {

	void RequestBoolean(const std::string& BooleanName, bool DefaultValue); 
	bool GetBoolean(const std::string& BooleanName, int * OutPut = nullptr); 
	std::vector<std::string> BooleanCommand(std::vector<std::string> Input); 



}