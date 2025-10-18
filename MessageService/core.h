#pragma once
#include "menu.h"
#include "profile.h"
#include "Key.h"

class Core {
public:
	Core() {
		Profile UserProfile;
		Key Key;
		Menu Menu(UserProfile);
	}

};