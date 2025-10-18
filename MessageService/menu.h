#pragma once
#include <string>
#include "input_utils.h"
#include "profile.h"


class Menu {

protected:
	COMMAND CurrentCommand;
	Profile CurrentUser;


	inline const COMMAND GetCurrentCommand() const {
		return CurrentCommand;
	}

private:
	inline void ExecuteCommand() {
		switch (CurrentCommand.INPUT) {

		case MENU_INPUTS::USERNAME: {
			Possible Data = CurrentCommand.DATA;
			char* Username = std::get<USERNAME>(Data).NAME;
			CurrentUser.SetName(Username);
			break;
		}
		case MENU_INPUTS::COLOR: {
			Possible Data = CurrentCommand.DATA;
			u32 Color = std::get<COLOR>(Data).RGBA;
			CurrentUser.SetColor(Color);
			break;
		}
		default: break;

		}
	}

public:
	Menu(const Profile& User) : CurrentUser(User) {
		while (CurrentCommand.INPUT != MENU_INPUTS::CLOSE) {
			std::cout << "Type in your command \n";
			std::string input;
			std::getline(std::cin, input);
			CurrentCommand = ParseInput(input);
			CurrentCommand.PrintCommand();
			ExecuteCommand();
		}
		std::cout << "Closing Menu";
	}
	Menu() = default;
};