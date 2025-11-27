#pragma once

#include <string>
#include "input_utils.h"   // COMMAND, MENU_INPUTS, ParseInput, Possible, USERNAME, COLOR
#include "profile.h"
#include "types.h"

class Menu {
public:
    explicit Menu(Profile& user);

    // Runs the interactive menu until the user issues
    // CREATE, CONNECT, or CLOSE. Returns that command.
    COMMAND Run();

private:
    COMMAND  CurrentCommand{};
    Profile& CurrentUser;

    void ExecuteCommand();
};
