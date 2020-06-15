#pragma once

#include <any>
#include <vector>
#include <string>
#include <queue>
#include <unordered_map>

namespace iTrace {

	struct CommandPush {

		std::string CommandID = "null";
		std::vector<std::any> CommandData = {};

	};

	struct CommandPusher {
		
		void* World;
		
		std::queue<CommandPush> Commands; 

		CommandPush PopCommand(); 

		void AddCommand(CommandPush Command); 

		std::unordered_map<std::string, std::any> GivenConstantData; 


	};

	CommandPusher& GetGlobalCommandPusher(); 

}

