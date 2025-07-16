#pragma once

#include <arpa/inet.h>

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

struct TCPSocketConfig {
    std::string ip;
    int port = -1;
    bool is_listening = false;
    bool is_blocking = false;
    bool needs_so_timestamp = false;

    auto toString() const {
        std::stringstream ss;
        ss << "SocketCfg[ip: " << ip << " port:" << port << " is listening: " << is_listening
           << " is blocking: " << is_blocking << " needs_SO_timestamp: " << needs_so_timestamp << "]";

        return ss.str();
    }
};

// Create TCPSocket with provided attributes to either listen-on / connect-to.
[[nodiscard]] int create_socket(const TCPSocketConfig &socket_config);
