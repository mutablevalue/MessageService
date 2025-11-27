#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include "Core.h"

int main() {
    WSADATA wsaData{};
    constexpr WORD RequestedVersion = MAKEWORD(2, 2);

    const int startupResult = WSAStartup(RequestedVersion, &wsaData);
    if (startupResult != 0) {
        std::cerr << "WSAStartup failed: " << startupResult << "\n";
        return 1;
    }

    {
        Core core;
    }

    WSACleanup();
    return 0;
}
