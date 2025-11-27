#pragma once

#include <string>
#include <iostream>
#include "input_utils.h"
#include "profile.h"
#include "types.h"

class Menu {
public:
    explicit Menu(Profile& user)
        : CurrentUser(user) {
    }

    COMMAND Run() {
        for (;;) {
            std::cout << "Type in your command\n";

            std::string input;
            if (!std::getline(std::cin, input)) {
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

private:
    COMMAND CurrentCommand{};
    Profile& CurrentUser;

    static constexpr std::size_t MaxNameLength = 16U;

    void ExecuteCommand() {
        switch (CurrentCommand.INPUT) {
        case MENU_INPUTS::USERNAME: {
            Possible data = CurrentCommand.DATA;
            const USERNAME& usernameData = std::get<USERNAME>(data);
            CurrentUser.SetName(usernameData.NAME);
            break;
        }
        case MENU_INPUTS::COLOR: {
            Possible data = CurrentCommand.DATA;
            const COLOR& colorData = std::get<COLOR>(data);
            CurrentUser.SetColor(colorData.RGBA);
            break;
        }
        default:
            break;
        }
    }
};
