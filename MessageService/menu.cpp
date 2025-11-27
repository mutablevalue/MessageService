#include "menu.h"

#include <iostream>

Menu::Menu(Profile& user)
    : CurrentUser(user) {
}

// Handle commands that modify the current user profile.
void Menu::ExecuteCommand() {
    switch (CurrentCommand.INPUT) {
    case MENU_INPUTS::USERNAME: {
        const USERNAME& data = std::get<USERNAME>(CurrentCommand.DATA);
        CurrentUser.SetName(data.NAME);
        break;
    }
    case MENU_INPUTS::COLOR: {
        const COLOR& data = std::get<COLOR>(CurrentCommand.DATA);
        CurrentUser.SetColor(data.RGBA);
        break;
    }
    default:
        break;
    }
}

COMMAND Menu::Run() {
    for (;;) {
        std::cout << "Type in your command\n";

        std::string input;
        if (!std::getline(std::cin, input)) {
            // EOF or error -> treat as CLOSE so Core can exit.
            CurrentCommand.INPUT = MENU_INPUTS::CLOSE;
            return CurrentCommand;
        }

        CurrentCommand = ParseInput(input);
        CurrentCommand.PrintCommand();

        ExecuteCommand();

        if (CurrentCommand.INPUT == MENU_INPUTS::CREATE ||
            CurrentCommand.INPUT == MENU_INPUTS::CONNECT ||
            CurrentCommand.INPUT == MENU_INPUTS::CLOSE) {
            return CurrentCommand;
        }
    }
}
