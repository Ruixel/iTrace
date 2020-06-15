#include "BooleanCommands.h"
#include <unordered_map>

std::unordered_map<std::string, bool> Booleans; 

void iTrace::RequestBoolean(const std::string& BooleanName, bool DefaultValue)
{
	Booleans[BooleanName] = DefaultValue; 
}

bool iTrace::GetBoolean(const std::string& BooleanName, int* OutPut)
{
	if (OutPut != nullptr)
		*OutPut = 0;
	
	if (Booleans.find(BooleanName) == Booleans.end()) {
		if (OutPut != nullptr)
			*OutPut = -1; 
		return false; 
	}
	return Booleans[BooleanName]; 

}

std::vector<std::string> iTrace::BooleanCommand(std::vector<std::string> Input)
{

	if (Input.size() == 1 && Input[0] == "help") {

		return { "boolean command. used to set/get global booleans.", 
			"useage: boolean [name] [set/get] [value, only for set]", 
			"example: boolean set raytracing true", 
			"for a list of all booleans, use boolean list" }; 


	}
	else if (Input.size() == 1 && Input[0] == "list") {

		std::vector<std::string> OutPut; 

		int i = 1; 

		OutPut.push_back("All booleans: "); 

		for (auto& Boolean : Booleans) {

			OutPut[OutPut.size() - 1] += (Boolean.first + " "); 

			i++; 

			if (i == 3) {
				OutPut.push_back(""); 
			}

		}
		return OutPut; 

	}
	

	if (Input.size() < 2) {
		return { "Not enough input provided for boolean command." }; 
	}


	if (Booleans.find(Input[0]) == Booleans.end()) {

		return { "No boolean called" + Input[0] + " found, use boolean list for a complete list of all booleans" }; 

	}
	
	if (Input[1] == "set") {

		if (Input.size() < 3) {
			return { "Not enough input provided for boolean command." };
		}

		bool Value = false; 
		if (Input[2] == "true") {
			Value = true; 
		}
		else if (Input[2] != "false") {
			return { "Wrong parameter for boolean index 3 in boolean command, only options are true/false" };
		}

		Booleans[Input[0]] = Value; 

		return { "Boolean: " + Input[0] + " set to " + Input[2] }; 

	}
	else if (Input[1] == "get") {

		return { "Boolean: " + Input[0] + (Booleans[Input[0]] ? " is true" : " is false") }; 

	}
	else {
		return { "Wrong parameter for boolean index 2 in boolean command, only options are set/get" }; 
	}


}
