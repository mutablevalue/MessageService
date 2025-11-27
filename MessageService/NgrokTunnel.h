#pragma once

#include <string>
#include "types.h"

struct NgrokEndpoint {
    std::string Host;
    u16         Port;
};

// Throws std::runtime_error on failure.
NgrokEndpoint StartNgrokTcp(u16 localPort, const std::string& ngrokPath);
