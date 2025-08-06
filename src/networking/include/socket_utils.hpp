#pragma once

#include <arpa/inet.h>

#include <cstddef>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

constexpr auto INVALID_SOCKET = -1;

class SocketException : public std::runtime_error {
  public:
    SocketException(const std::string &message)
      : std::runtime_error(message + " (errno: " + std::to_string(errno) + " - " + std::strerror(errno) + ")") {
    }
};

/**
 * @brief Use to configure a socket. Depending on the use of the socket (client or server) each field should be
 * configured differently. (e.g. On a listening socket, port means the port that is listening on but on a client socket
 * it means the port that it connects to.)
 *
 */
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

bool set_non_blocking(int fd);

// Create TCPSocket with provided attributes to either listen-on / connect-to.
[[nodiscard]] int create_socket(const TCPSocketConfig &socket_config);
