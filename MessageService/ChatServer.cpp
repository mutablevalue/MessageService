#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "ChatServer.h"

#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

static constexpr std::string_view HelloCommand = "HELLO";

ChatServer::ChatServer() {
    InitializeListeningSocket();
    PrintStartupInfo();
}

ChatServer::~ChatServer() {
    Stop();
}

void ChatServer::Run() {
    Accepting.store(true);
    AcceptLoop();
}

void ChatServer::Stop() {
    bool wasAccepting = true;
    if (!Accepting.compare_exchange_strong(wasAccepting, false)) {
        return;
    }

    if (ListeningSocket != INVALID_SOCKET) {
        ShutdownAndCloseSocket(ListeningSocket);
        ListeningSocket = INVALID_SOCKET;
    }

    std::lock_guard<std::mutex> lock(ClientsMutex);
    for (auto& client : Clients) {
        if (client && client->Socket != INVALID_SOCKET) {
            ShutdownAndCloseSocket(client->Socket);
            client->Socket = INVALID_SOCKET;
        }
    }
    Clients.clear();
}

void ChatServer::ShutdownAndCloseSocket(SOCKET socket) {
    constexpr int ShutdownBoth = SD_BOTH;
    shutdown(socket, ShutdownBoth);
    closesocket(socket);
}

void ChatServer::InitializeListeningSocket() {
    ListeningSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListeningSocket == INVALID_SOCKET) {
        throw std::runtime_error("ChatServer: socket() failed");
    }

    BOOL allowReuse = TRUE;
    const char* optionBuffer = reinterpret_cast<const char*>(&allowReuse);
    const int optionLength = static_cast<int>(sizeof(allowReuse));

    if (setsockopt(ListeningSocket,
        SOL_SOCKET,
        SO_REUSEADDR,
        optionBuffer,
        optionLength) == SOCKET_ERROR) {
        ShutdownAndCloseSocket(ListeningSocket);
        ListeningSocket = INVALID_SOCKET;
        throw std::runtime_error("ChatServer: setsockopt(SO_REUSEADDR) failed");
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(ServerPort);

    if (bind(ListeningSocket,
        reinterpret_cast<sockaddr*>(&address),
        static_cast<int>(sizeof(address))) == SOCKET_ERROR) {
        ShutdownAndCloseSocket(ListeningSocket);
        ListeningSocket = INVALID_SOCKET;
        throw std::runtime_error("ChatServer: bind() failed");
    }

    if (listen(ListeningSocket, ListenBacklog) == SOCKET_ERROR) {
        ShutdownAndCloseSocket(ListeningSocket);
        ListeningSocket = INVALID_SOCKET;
        throw std::runtime_error("ChatServer: listen() failed");
    }
}

void ChatServer::PrintStartupInfo() const {
    std::cout << "Server listening on port " << ServerPort << "\n";
}

std::string ChatServer::BuildColoredName(const std::string& name, u32 rgba) {
    constexpr u32 BitsPerComponent = 8U;
    constexpr u32 RedShift = 16U;
    constexpr u32 GreenShift = 8U;
    constexpr u32 BlueShift = 0U;
    constexpr u32 ComponentMask = 0xFFU;

    const u8 red = static_cast<u8>((rgba >> RedShift) & ComponentMask);
    const u8 green = static_cast<u8>((rgba >> GreenShift) & ComponentMask);
    const u8 blue = static_cast<u8>((rgba >> BlueShift) & ComponentMask);

    constexpr std::string_view EscapePrefix = "\x1b[38;2;";
    constexpr std::string_view EscapeSuffix = "m";
    constexpr std::string_view ResetCode = "\x1b[0m";

    std::ostringstream builder;
    builder << EscapePrefix
        << static_cast<unsigned int>(red) << ';'
        << static_cast<unsigned int>(green) << ';'
        << static_cast<unsigned int>(blue) << EscapeSuffix
        << name
        << ResetCode;
    return builder.str();
}

void ChatServer::AcceptLoop() {
    while (Accepting.load()) {
        sockaddr_in clientAddress{};
        int clientLength = static_cast<int>(sizeof(clientAddress));

        SOCKET clientSocket = accept(
            ListeningSocket,
            reinterpret_cast<sockaddr*>(&clientAddress),
            &clientLength
        );

        if (clientSocket == INVALID_SOCKET) {
            if (!Accepting.load()) {
                break;
            }
            std::cerr << "ChatServer: accept() failed with error "
                << WSAGetLastError() << "\n";
            continue;
        }

        auto session = std::make_unique<ServerClientSession>();
        session->Socket = clientSocket;
        ServerClientSession* rawSession = session.get();

        {
            std::lock_guard<std::mutex> lock(ClientsMutex);
            Clients.push_back(std::move(session));
        }

        std::thread handler(&ChatServer::HandleClient, this, rawSession);
        handler.detach();
    }
}

bool ChatServer::ReceiveLine(ServerClientSession* session, std::string& outLine) {
    outLine.clear();

    while (true) {
        const std::size_t delimiterPosition =
            session->PendingBuffer.find(LineDelimiter);

        if (delimiterPosition != std::string::npos) {
            outLine = session->PendingBuffer.substr(0U, delimiterPosition);
            const std::size_t eraseCount = delimiterPosition + 1U;
            session->PendingBuffer.erase(0U, eraseCount);
            return true;
        }

        char receiveBuffer[ReceiveChunkSize];
        const int bytesReceived = recv(
            session->Socket,
            receiveBuffer,
            ReceiveChunkSize,
            0
        );

        if (bytesReceived <= 0) {
            return false;
        }

        session->PendingBuffer.append(
            receiveBuffer,
            receiveBuffer + bytesReceived
        );
    }
}

void ChatServer::HandleClient(ServerClientSession* session) {
    if (!HandleHello(session)) {
        CloseSession(session);
        return;
    }

    {
        const std::string joinMessage =
            BuildColoredName(session->Name, session->Color) +
            " joined.\n";
        Broadcast(joinMessage);
    }

    while (true) {
        std::string messageLine;
        if (!ReceiveLine(session, messageLine)) {
            break;
        }

        const std::string taggedMessage =
            BuildColoredName(session->Name, session->Color) +
            ' ' +
            messageLine +
            '\n';

        Broadcast(taggedMessage);
    }

    {
        const std::string leaveMessage =
            BuildColoredName(session->Name, session->Color) +
            " left.\n";
        Broadcast(leaveMessage);
    }

    CloseSession(session);
}

bool ChatServer::HandleHello(ServerClientSession* session) {
    std::string helloLine;
    if (!ReceiveLine(session, helloLine)) {
        return false;
    }

    std::istringstream parser(helloLine);
    std::string helloToken;
    std::string nameToken;
    std::string colorToken;

    parser >> helloToken >> nameToken >> colorToken;

    if (helloToken != HelloCommand ||
        nameToken.empty() ||
        colorToken.empty()) {
        return false;
    }

    session->Name = nameToken;
    const unsigned long parsedColor =
        std::strtoul(colorToken.c_str(), nullptr, 0);
    session->Color = static_cast<u32>(parsedColor);
    return true;
}

void ChatServer::Broadcast(const std::string& text) {
    std::cout << text;

    std::lock_guard<std::mutex> lock(ClientsMutex);

    for (auto& client : Clients) {
        if (!client || client->Socket == INVALID_SOCKET) {
            continue;
        }

        const int totalLength = static_cast<int>(text.size());
        const int bytesSent = send(
            client->Socket,
            text.data(),
            totalLength,
            0
        );

        if (bytesSent == SOCKET_ERROR) {
            // Receiver will eventually fail; nothing else to do here.
        }
    }
}

void ChatServer::CloseSession(ServerClientSession* targetSession) {
    std::lock_guard<std::mutex> lock(ClientsMutex);

    auto removeBegin = std::remove_if(
        Clients.begin(),
        Clients.end(),
        [targetSession](const std::unique_ptr<ServerClientSession>& session) {
            if (session.get() == targetSession) {
                if (session->Socket != INVALID_SOCKET) {
                    ChatServer::ShutdownAndCloseSocket(session->Socket);
                    session->Socket = INVALID_SOCKET;
                }
                return true;
            }
            return false;
        }
    );

    Clients.erase(removeBegin, Clients.end());
}
