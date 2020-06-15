#include "CommandManager.h"
#include <sstream>
#include <iostream>

namespace iTrace {

	bool CommandManager::PollCommands(sf::Event& KeyBoardEvent)
	{

		//if its not a keyboard event, don't bother 
		if (KeyBoardEvent.type != sf::Event::KeyPressed && KeyBoardEvent.type != sf::Event::TextEntered)
			return false;

		if (Active) {


			auto CopyCommand = [&]() {



				if (OldCommands.size() == 0)
					return; 

				if (CommandCopyIndex == -1) {
					CurrentCommandText = "";
					return; 
				}
				
			
				int TrueIndex = OldCommands.size() - 1 - CommandCopyIndex;

				CurrentCommandText = std::get<0>(OldCommands[TrueIndex]);


			};

			if (KeyBoardEvent.key.code == sf::Keyboard::Return && KeyBoardEvent.type == sf::Event::KeyPressed) {

				if (CurrentCommandText.size() == 0) {
					//No command text, so cannot get parsed 
					Active = false;
					CurrentCommandText = "";
					CommandCopyIndex = -1;
					return true;

				}


				//Command is now being requested to get parsed, so do that 

				std::vector<std::string> CommandData = {};
				std::string CommandName = "";

				std::stringstream CommandStream(CurrentCommandText);

				CommandStream >> CommandName;

				if (Commands.find(CommandName) == Commands.end()) {
					Active = false;
					CurrentCommandText = "";
					CommandCopyIndex = -1;
					OldOutPut.push_back({ "Failed to find command: " + CommandName,0.0 });
					return true;
				}

				std::string Temp = "";

				while (CommandStream >> Temp) {
					CommandData.push_back(Temp);
				}

				OldCommands.push_back({ CurrentCommandText, 0.0f });

				auto OutPut = Commands[CommandName].CommandFunction(CommandData);

				if (OutPut.size() > 0) {
					for(auto & OutPut : OutPut)
						OldOutPut.push_back({ OutPut, 0.0f });
				}

				Active = false;
				CurrentCommandText = "";
				CommandCopyIndex = -1;

				return true;


			}
			else if (KeyBoardEvent.key.code == sf::Keyboard::Escape && KeyBoardEvent.type == sf::Event::KeyPressed) {
				Active = false;
				CurrentCommandText = "";
				CommandCopyIndex = -1;
				return true;
			}
			else if (KeyBoardEvent.key.code == sf::Keyboard::BackSpace && KeyBoardEvent.type == sf::Event::KeyPressed) {

				CommandCopyIndex = 0; //The command has now been changed, reset to most recent command 

				if (CurrentCommandText.size() > 0) {
					CurrentCommandText.pop_back();
				}
				//Assumes the cursor is at the very last character, which may not always be the case 

			}
			else if (KeyBoardEvent.key.code == sf::Keyboard::Up && KeyBoardEvent.type == sf::Event::KeyPressed) {
				//copy a command, and set the index for the copied command 


				CommandCopyIndex++;
				CommandCopyIndex = glm::clamp(CommandCopyIndex, 0, int(OldCommands.size()-1));
				CopyCommand();

			}
			else if (KeyBoardEvent.key.code == sf::Keyboard::Down && KeyBoardEvent.type == sf::Event::KeyPressed) {
				//copy a command, and set the index for the copied command 


				CommandCopyIndex--;
				CommandCopyIndex = glm::clamp(CommandCopyIndex, -1, int(OldCommands.size()-1));
				CopyCommand();

			}
			else if (KeyBoardEvent.type == sf::Event::TextEntered) {
				//No specific key is used, so the user likely typed something 

				std::cout << "yeet\n";

				if (KeyBoardEvent.text.unicode <= 126 && KeyBoardEvent.text.unicode >= 32)
					CurrentCommandText += char(KeyBoardEvent.text.unicode);

			}

			return true;
		}
		else {
			//Active via the 'T' key (will probably be reassignable in the future) 


			if (KeyBoardEvent.key.code == sf::Keyboard::T && KeyBoardEvent.type == sf::Event::KeyPressed) {
				Active = true;
				return true;
			}

		}

		return false;



	}

	void CommandManager::DrawCommandText(Rendering::TextSystem* TextSystem, Window& Window)
	{
		int x = 0;
		if (Active) {
			TextSystem->DisplayText(CurrentCommandText + (int(floor(Window.GetTimeOpened() * 2.)) % 2 == 0 && Active ? "_" : ""), Window, 0.005, Vector2f(-.9, -0.5), Vector3f(1.0));
		}
		for (auto& OldCommand : OldCommands) {
			std::get<1>(OldCommand) += Window.GetFrameTime();

			if (std::get<1>(OldCommand) > 900.0) {
				OldCommands.erase(OldCommands.begin() + x);
			}

			x++;
		}
		x = 0;

		for (auto& OutPut : OldOutPut) {
			std::get<1>(OutPut) += Window.GetFrameTime();

			if (std::get<1>(OutPut) > 80.0 || OldOutPut.size() > 24) {
				OldOutPut.erase(OldOutPut.begin() + x);
			}
			x++;
		}
		x = 0; 
		for (int i = OldOutPut.size() - 1; i != -1; i--) {


			TextSystem->DisplayText("> " +std::get<0>(OldOutPut[i]), Window, 0.005, Vector2f(-.9, -0.5 + 0.0175 * (x + 1) * (float(Window.GetResolution().x) / float(Window.GetResolution().y))), Vector3f(1.0));
			x++; 
		}

	}

}