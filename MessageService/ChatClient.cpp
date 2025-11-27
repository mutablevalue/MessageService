#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "ChatClient.h"

#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <sstream>

static constexpr std::string_view HelloCommand = "HELLO";

ChatClient::ChatClient(const Profile& userProfile, const KArray& userKey)
    : Socket(INVALID_SOCKET),
    UserProfile(userProfile),
    UserKey(userKey) {
}

bool ChatClient::ConnectToServer(const CONNECT& connectionInfo) {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    std::string portString = std::to_string(connectionInfo.PORT);
    addrinfo* result = nullptr;

    const int res = getaddrinfo(
        connectionInfo.IP,
        portString.c_str(),
        &hints,
        &result
    );

    if (res != 0 || result == nullptr) {
        std::cerr << "getaddrinfo failed for "
            << connectionInfo.IP << ":" << connectionInfo.PORT
            << " (error " << res << ")\n";
        return false;
    }

    SOCKET connected = INVALID_SOCKET;

    for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        SOCKET candidate = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (candidate == INVALID_SOCKET) {
            continue;
        }

        const int connectResult = connect(
            candidate,
            ptr->ai_addr,
            static_cast<int>(ptr->ai_addrlen)
        );

        if (connectResult == SOCKET_ERROR) {
            closesocket(candidate);
            continue;
        }

        connected = candidate;
        break;
    }

    freeaddrinfo(result);

    if (connected == INVALID_SOCKET) {
        std::cerr << "Unable to connect to "
            << connectionInfo.IP << ":" << connectionInfo.PORT << "\n";
        return false;
    }

    Socket = connected;

    if (!SendHello()) {
        std::cerr << "Failed to send HELLO handshake\n";
        CloseSocket();
        return false;
    }

    std::cout << "Connected to "
        << connectionInfo.IP << ":" << connectionInfo.PORT << "\n";
    return true;
}

void ChatClient::Run() {
    if (Socket == INVALID_SOCKET) {
        return;
    }

    Running.store(true);
    ReceiveThread = std::thread(&ChatClient::ReceiveLoop, this);

    RunInputLoop();

    Running.store(false);
    CloseSocket();

    if (ReceiveThread.joinable()) {
        ReceiveThread.join();
    }
}

void ChatClient::CloseSocket() {
    if (Socket != INVALID_SOCKET) {
        constexpr int ShutdownBoth = SD_BOTH;
        shutdown(Socket, ShutdownBoth);
        closesocket(Socket);
        Socket = INVALID_SOCKET;
    }
}

bool ChatClient::SendHello() {
    std::ostringstream builder;
    builder << HelloCommand << ' '
        << UserProfile.Name << ' '
        << "0x" << std::hex << UserProfile.Color
        << LineDelimiter;

    const std::string helloMessage = builder.str();
    const int messageLength = static_cast<int>(helloMessage.size());

    const int bytesSent = send(
        Socket,
        helloMessage.data(),
        messageLength,
        0
    );

    return bytesSent != SOCKET_ERROR;
}

void ChatClient::ReceiveLoop() {
    char buffer[ReceiveBufferSize];

    while (Running.load()) {
        const int bytesReceived = recv(
            Socket,
            buffer,
            ReceiveBufferSize - 1,
            0
        );

        if (bytesReceived <= 0) {
            break;
        }

        buffer[bytesReceived] = '\0';
        std::cout << buffer;
    }
}

void ChatClient::RunInputLoop() {
    for (;;) {
        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }

        if (line == QuitCommandOne || line == QuitCommandTwo) {
            break;
        }

        line.push_back(LineDelimiter);

        const int messageLength = static_cast<int>(line.size());
        const int bytesSent = send(
            Socket,
            line.data(),
            messageLength,
            0
        );

        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "send() failed with error: "
                << WSAGetLastError() << "\n";
            break;
        }
    }
}
