#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "NgrokTunnel.h"

#include <stdexcept>
#include <string>
#include <vector>
#include <cstdlib>

static constexpr std::size_t MaxNgrokCmdLength = 512U;
static constexpr std::size_t NgrokReadBufferSize = 1024U;

static std::pair<std::string, u16> SplitHostPort(const std::string& hostPort) {
    const std::size_t colonPos = hostPort.rfind(':');
    if (colonPos == std::string::npos) {
        throw std::runtime_error("NgrokTunnel: invalid tcp URL (no colon)");
    }

    std::string host = hostPort.substr(0U, colonPos);
    const std::string portString = hostPort.substr(colonPos + 1U);

    const unsigned long portLong =
        std::strtoul(portString.c_str(), nullptr, 10);
    const u16 port = static_cast<u16>(portLong);

    return { host, port };
}

NgrokEndpoint StartNgrokTcp(u16 localPort, const std::string& ngrokPath) {
    HANDLE readPipe = nullptr;
    HANDLE writePipe = nullptr;

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    if (!CreatePipe(&readPipe, &writePipe, &sa, 0U)) {
        throw std::runtime_error("NgrokTunnel: CreatePipe failed");
    }

    if (!SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0U)) {
        CloseHandle(readPipe);
        CloseHandle(writePipe);
        throw std::runtime_error("NgrokTunnel: SetHandleInformation failed");
    }

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    si.hStdOutput = writePipe;
    si.hStdError = writePipe;
    si.hStdInput = nullptr;
    si.dwFlags = STARTF_USESTDHANDLES;

    std::string commandLine;
    commandLine.reserve(256);
    commandLine.push_back('"');
    commandLine.append(ngrokPath);
    commandLine.push_back('"');
    commandLine.append(" tcp ");
    commandLine.append(std::to_string(localPort));
    commandLine.append(" --log=stdout");

    if (commandLine.size() >= MaxNgrokCmdLength) {
        CloseHandle(readPipe);
        CloseHandle(writePipe);
        throw std::runtime_error("NgrokTunnel: command too long");
    }

    char cmdBuffer[MaxNgrokCmdLength]{};
    std::snprintf(cmdBuffer, MaxNgrokCmdLength, "%s", commandLine.c_str());

    const BOOL created = CreateProcessA(
        nullptr,
        cmdBuffer,
        nullptr,
        nullptr,
        TRUE,
        0U,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    CloseHandle(writePipe);

    if (!created) {
        CloseHandle(readPipe);
        throw std::runtime_error("NgrokTunnel: CreateProcessA failed");
    }

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    std::string output;
    output.reserve(NgrokReadBufferSize * 4U);

    CHAR buffer[NgrokReadBufferSize];

    const std::string needle = "tcp://";
    while (true) {
        DWORD bytesRead = 0U;
        const BOOL ok = ReadFile(
            readPipe,
            buffer,
            static_cast<DWORD>(NgrokReadBufferSize - 1U),
            &bytesRead,
            nullptr
        );

        if (!ok || bytesRead == 0U) {
            break;
        }

        buffer[bytesRead] = '\0';
        output.append(buffer, buffer + bytesRead);

        const std::size_t pos = output.find(needle);
        if (pos != std::string::npos) {
            const std::size_t lineEnd = output.find('\n', pos);
            const std::size_t end =
                (lineEnd == std::string::npos) ? output.size() : lineEnd;

            const std::string hostPort =
                output.substr(pos + needle.size(), end - (pos + needle.size()));

            CloseHandle(readPipe);

            auto [host, port] = SplitHostPort(hostPort);
            return NgrokEndpoint{ host, port };
        }
    }

    CloseHandle(readPipe);
    throw std::runtime_error("NgrokTunnel: could not find tcp:// line in ngrok output");
}
