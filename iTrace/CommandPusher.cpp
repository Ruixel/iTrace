#include "CommandPusher.h"
#include "WorldManager.h"

namespace iTrace {

	CommandPusher* GlobalPusher = nullptr; 

	CommandPush CommandPusher::PopCommand()
	{
		auto Last = Commands.front(); 
		Commands.pop(); 

		return Last;
	}

	void CommandPusher::AddCommand(CommandPush Command)
	{
		Commands.push(Command); 
	}

	CommandPusher& GetGlobalCommandPusher()
	{
		if (GlobalPusher == nullptr)
			GlobalPusher = new CommandPusher();
		return *GlobalPusher; 
	}

}
