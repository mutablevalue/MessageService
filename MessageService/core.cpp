#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Core.h"

#include <iostream>
#include <string>

#include <ws2tcpip.h>      // getaddrinfo, inet_ntop
#pragma comment(lib, "Ws2_32.lib")

#include "menu.h"
#include "profile.h"
#include "Key.h"
#include "file.h"
#include "ChatServer.h"
#include "ChatClient.h"
#include "input_utils.h"
#include "types.h"

Core::Core() {
    Run();
}

static Profile LoadOrCreateProfile() {
    FileSystem fileSystem;

    if (FileSystem::ProfileExists()) {
        return fileSystem.LoadProfile();
    }

    constexpr char DefaultName[] = "User";
    constexpr u32  DefaultColor = 0xFFFFFFFFU;

    const Profile defaultProfile{ DefaultName, DefaultColor };
    fileSystem.CreateProfile(defaultProfile);
    return defaultProfile;
}

static KArray LoadOrCreateKey() {
    Key key;
    return key.GetKey();
}

static void SaveProfile(const Profile& profile) {
    FileSystem fileSystem;
    fileSystem.OpenProfileFile();
    fileSystem.WriteProfile(profile);
}

static void RunClient(const Profile& userProfile,
    const KArray& keyValue,
    const CONNECT& connectionInfo) {
    ChatClient client(userProfile, keyValue);
    if (!client.ConnectToServer(connectionInfo)) {
        std::cerr << "Failed to connect to server\n";
        return;
    }
    client.Run();
}

// Print local endpoints the host can give to other users.
static void PrintLocalEndpoints() {
    constexpr u16 Port = ChatServer::ServerPort;

    std::cout << "You can connect from this machine with:\n"
        << "  CONNECT 127.0.0.1 " << Port << "\n";

    char hostName[256]{};

    if (gethostname(hostName, static_cast<int>(sizeof(hostName))) == SOCKET_ERROR) {
        return;
    }

    addrinfo hints{};
    hints.ai_family = AF_INET;      // IPv4 only to keep it simple
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    addrinfo* result = nullptr;
    const int res = getaddrinfo(hostName, nullptr, &hints, &result);
    if (res != 0 || result == nullptr) {
        return;
    }

    std::cout << "On the same LAN/Wi-Fi, other clients can connect with:\n";

    char addrString[INET_ADDRSTRLEN]{};
    for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(ptr->ai_addr);

        // Skip loopback 127.x.x.x
        const u32 raw = ntohl(addr->sin_addr.s_addr);
        const u8 firstOctet = static_cast<u8>((raw >> 24) & 0xFFU);
        if (firstOctet == 127U) {
            continue;
        }

        if (!inet_ntop(AF_INET, &addr->sin_addr, addrString, sizeof(addrString))) {
            continue;
        }

        std::cout << "  CONNECT " << addrString << " " << Port << "\n";
    }

    freeaddrinfo(result);
}

void Core::Run() {
    Profile userProfile = LoadOrCreateProfile();
    const KArray keyValue = LoadOrCreateKey();

    Menu menu(userProfile);
    const COMMAND command = menu.Run();

    SaveProfile(userProfile);

    switch (command.INPUT) {
    case MENU_INPUTS::CREATE: {
        ChatServer server;
        std::cout << "Server listening on port " << ChatServer::ServerPort << "\n";
        PrintLocalEndpoints();
        server.Run();
        break;
    }

    case MENU_INPUTS::CONNECT: {
        const CONNECT connectionInfo = std::get<CONNECT>(command.DATA);
        RunClient(userProfile, keyValue, connectionInfo);
        break;
    }

    case MENU_INPUTS::CLOSE:
    default:
        break;
    }
}
