#pragma once

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <mutex>
#include "types.h"

class ChatServer {
public:
    static constexpr u16 ServerPort = 8080U;
    static constexpr int ListenBacklog = 16;
    static constexpr int ReceiveChunkSize = 1024;
    static constexpr char LineDelimiter = '\n';
    static constexpr u32 DefaultColor = 0xFFFFFFFFU;

    ChatServer();
    ~ChatServer();

    void Run();
    void Stop();

private:
    struct ServerClientSession {
        SOCKET      Socket = INVALID_SOCKET;
        std::string Name;
        u32         Color = DefaultColor;
        std::string PendingBuffer;
    };

    SOCKET ListeningSocket = INVALID_SOCKET;
    std::atomic<bool> Accepting{ false };

    std::mutex ClientsMutex;
    std::vector<std::unique_ptr<ServerClientSession>> Clients;

    static void ShutdownAndCloseSocket(SOCKET socket);
    void InitializeListeningSocket();
    void PrintStartupInfo() const;
    void AcceptLoop();
    static bool ReceiveLine(ServerClientSession* session, std::string& outLine);
    void HandleClient(ServerClientSession* session);
    bool HandleHello(ServerClientSession* session);
    void Broadcast(const std::string& text);
    void CloseSession(ServerClientSession* targetSession);

    static std::string BuildColoredName(const std::string& name, u32 rgba);
};
