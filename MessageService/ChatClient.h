#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <thread>
#include <atomic>
#include <string>
#include "types.h"
#include "profile.h"
#include "Key.h"
#include "input_utils.h"

class ChatClient {
public:
    static constexpr int  ReceiveBufferSize = 1024;
    static constexpr char LineDelimiter = '\n';

    ChatClient(const Profile& userProfile, const KArray& userKey);

    bool ConnectToServer(const CONNECT& connectionInfo);
    void Run();

private:
    SOCKET Socket = INVALID_SOCKET;
    Profile UserProfile{};
    KArray UserKey{};
    std::atomic<bool> Running{ false };
    std::thread ReceiveThread;

    static constexpr const char* QuitCommandOne = "/quit";
    static constexpr const char* QuitCommandTwo = "/exit";

    void CloseSocket();
    bool SendHello();
    void ReceiveLoop();
    void RunInputLoop();
};
