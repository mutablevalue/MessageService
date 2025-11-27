#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Core.h"

#include <iostream>
#include <string>

#include "menu.h"
#include "profile.h"
#include "Key.h"
#include "file.h"
#include "ChatServer.h"
#include "ChatClient.h"
#include "NgrokTunnel.h"
#include "input_utils.h"
#include "types.h"

static constexpr const char* NgrokExecutablePath = "C:\\Tools\\ngrok\\ngrok.exe";

Core::Core() {
    Run();
}

static Profile LoadOrCreateProfile() {
    FileSystem fileSystem;

    if (FileSystem::ProfileExists()) {
        return fileSystem.LoadProfile();
    }

    constexpr char DefaultName[] = "User";
    constexpr u32 DefaultColor = 0xFFFFFFFFU;

    const Profile defaultProfile{ DefaultName, DefaultColor };
    fileSystem.CreateProfile(defaultProfile);
    return defaultProfile;
}

static KArray LoadOrCreateKey() {
    Key key;
    return key.GetKey();
}

static void SaveProfile(const Profile& profile) {
    FileSystem fs;
    fs.OpenProfileFile();
    fs.WriteProfile(profile);
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

void Core::Run() {
    Profile userProfile = LoadOrCreateProfile();
    const KArray keyValue = LoadOrCreateKey();

    Menu menu(userProfile);
    const COMMAND command = menu.Run();

    SaveProfile(userProfile);

    switch (command.INPUT) {
    case MENU_INPUTS::CREATE: {
        ChatServer server;

        try {
            const NgrokEndpoint endpoint =
                StartNgrokTcp(ChatServer::ServerPort,
                    std::string(NgrokExecutablePath));

            std::cout << "Ngrok public endpoint:\n"
                << "  Host: " << endpoint.Host << "\n"
                << "  Port: " << endpoint.Port << "\n"
                << "Use: CONNECT " << endpoint.Host
                << " " << endpoint.Port << "\n";
        }
        catch (const std::exception& ex) {
            std::cerr << "Ngrok error: " << ex.what() << "\n";
            std::cerr << "You can still connect locally with:\n"
                << "  CONNECT 127.0.0.1 "
                << ChatServer::ServerPort << "\n";
        }

        server.Run();
        break;
    }
    case MENU_INPUTS::CONNECT: {
        const CONNECT info = std::get<CONNECT>(command.DATA);
        RunClient(userProfile, keyValue, info);
        break;
    }
    case MENU_INPUTS::CLOSE:
    default:
        break;
    }
}
