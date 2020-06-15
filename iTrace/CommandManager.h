#pragma once

#include "text.h"
#include <functional>
#include <unordered_map>

namespace iTrace {

	struct Command {

		std::string CommandName; 
		std::function<std::vector<std::string>(std::vector<std::string> & CommandData)> CommandFunction; 

		Command(const std::string & CommandName, std::function<std::vector<std::string>(std::vector<std::string> & CommandData)> CommandFunction) :
			CommandName(CommandName), CommandFunction(CommandFunction) {}
		Command() {}
	};

	struct CommandManager {

		std::unordered_map<std::string, Command> Commands = {};
		std::vector<std::tuple<std::string, float>> OldCommands = {};
		std::vector<std::tuple<std::string, float>> OldOutPut = {};

		bool Active = false; //<- is the command manager actually active? 

		
		int CommandCopyIndex = -1; 
		std::string CurrentCommandText = ""; 

		void AddCommands(Command Command) {
			Commands[Command.CommandName] = Command; 
		}

		bool PollCommands(sf::Event& KeyBoardEvent); 
		void DrawCommandText(Rendering::TextSystem* TextSystem, Window& Window); 
		CommandManager() {}
	};




}