#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <string_view>  
#include <cstdlib>    
#include "types.h"



struct CONNECT {
	char IP[16]; // 16 bytes MAX
	u16 PORT;
};

struct USERNAME {
	char NAME[16]; // 16 bytes 
};

struct COLOR {
	u32 RGBA; // simple 32bit
};

struct CREATE {};


enum class MENU_INPUTS : uint8_t {
	CONNECT,
	CREATE, //
	USERNAME,
	COLOR,
	BAD,
	CLOSE,
};

using Possible = std::variant<CREATE, COLOR, USERNAME, CONNECT>;

static const char* ToString(MENU_INPUTS Possible) {
	switch (Possible) {
	case MENU_INPUTS::CONNECT:  return "CONNECT";
	case MENU_INPUTS::CREATE:   return "CREATE";
	case MENU_INPUTS::USERNAME: return "USERNAME";
	case MENU_INPUTS::COLOR:    return "COLOR";
	case MENU_INPUTS::BAD:      return "BAD";
	case MENU_INPUTS::CLOSE:      return "CLOSE";
	default:                    return "UNKNOWN";
	}
}
	
struct COMMAND {
	MENU_INPUTS INPUT; // our input
	Possible DATA; 

	void PrintCommand() const {
		std::cout << "Command Type: " << ToString(INPUT) << "\n";

		// stream flags so hex doesn’t leak to later prints
		std::ios::fmtflags old = std::cout.flags();

		std::visit([](const auto& data) {
			using T = std::decay_t<decltype(data)>;

			if constexpr (std::is_same_v<T, CONNECT>) {
				std::cout << "  IP: " << data.IP << "\n";
				std::cout << "  PORT: " << data.PORT << "\n";
			}
			else if constexpr (std::is_same_v<T, USERNAME>) {
				std::cout << "  NAME: " << data.NAME << "\n";
			}
			else if constexpr (std::is_same_v<T, COLOR>) {
				std::cout << "  RGBA: 0x" << std::hex << data.RGBA << std::dec << "\n";
			}
			else if constexpr (std::is_same_v<T, CREATE>) {
				std::cout << "  (no data)\n";
			}
			}, DATA);

		std::cout.flags(old);
	}
};

// input example CONNECT 127.0.0.1 6953

inline COMMAND Fail(COMMAND& Input) {
	Input.INPUT = MENU_INPUTS::BAD; return Input;
}
constexpr const char* DELIMS = " \t\r\n";

static const std::unordered_map<std::string, MENU_INPUTS> Lookup = {
  {"CONNECT",  MENU_INPUTS::CONNECT},
  {"CREATE",   MENU_INPUTS::CREATE},
  {"USERNAME", MENU_INPUTS::USERNAME},
  {"NAME",     MENU_INPUTS::USERNAME}, 
  {"COLOR",    MENU_INPUTS::COLOR},
  {"CLOSE",    MENU_INPUTS::CLOSE},     
};


// The way this works is it uses strtok starting at a base pointer to retrieve places of data
// all str toks used are assumed

inline COMMAND ParseInput(std::string_view input) {
	COMMAND Command{}; // our base command

	char buffer[64]; // creating a new buffer
	if (input.size() >= sizeof(buffer)) return Fail(Command); // reject too-long lines

	size_t Length = std::min(input.size(), sizeof(buffer) - 1);

	std::memcpy(buffer, input.data(), Length); buffer[Length] = '\0'; // Making sure null term is there
	char* context = nullptr;
	char* Token = strtok_s(buffer, DELIMS, &context);

	auto it = (Token ? Lookup.find(Token) : Lookup.end());
	Command.INPUT = (it != Lookup.end()) ? it->second : MENU_INPUTS::BAD;
	// we have our menu input now we must handle subdata create has none so move on
	if (Command.INPUT == MENU_INPUTS::CREATE
		|| Command.INPUT == MENU_INPUTS::BAD
		|| Command.INPUT == MENU_INPUTS::CLOSE) return Command;
	char* NextToken = strtok_s(nullptr, DELIMS, &context); // get the next token

	if (!NextToken) return Fail(Command); // bad because for all other menu inputs needs more params
	switch (Command.INPUT) {

	case MENU_INPUTS::CONNECT: {
		char* PortToken = strtok_s(nullptr, DELIMS, &context);
		char* extra = strtok_s(nullptr, DELIMS, &context); // cant use the same nullptr
		if (!PortToken || extra) return Fail(Command);
		std::cout << PortToken << "\n";
		CONNECT TEMP{};
		if (std::strlen(NextToken) >= sizeof(TEMP.IP)) return Fail(Command);
		strncpy_s(TEMP.IP, sizeof(TEMP.IP), NextToken, sizeof(TEMP.IP) - 1);
		long port = std::strtol(PortToken, nullptr, 10); // making sure there isnt any leading data
		TEMP.PORT = static_cast<u16>(port);
		Command.DATA = TEMP; break;
	}
	case MENU_INPUTS::USERNAME: {
		char* extra = strtok_s(nullptr, DELIMS, &context);
		if (extra) return Fail(Command);
		USERNAME TEMP {};
		if (std::strlen(NextToken) >= sizeof(TEMP.NAME)) return Fail(Command);
		strncpy_s(TEMP.NAME, sizeof(TEMP.NAME), NextToken, sizeof(TEMP.NAME) - 1); // to username
		Command.DATA = TEMP; break;
	}
	case MENU_INPUTS::COLOR: {
		char* extra = strtok_s(nullptr, DELIMS, &context);
		if (extra) return Fail(Command);
		COLOR TEMP{};
		unsigned long color = std::strtoul(NextToken, nullptr, 0);
		TEMP.RGBA = static_cast<u32>(color); // needed cause of conversion
		Command.DATA = TEMP; break;
	}
	default: {
		Command.INPUT = MENU_INPUTS::BAD;
		return Command; break;
	}
}

	//we now have a fully setup command that can be executed
	return Command;
}
