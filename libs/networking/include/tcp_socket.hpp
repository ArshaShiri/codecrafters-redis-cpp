#pragma once

#include <arpa/inet.h>

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

struct TCPSocketConfig {
    std::string ip;
    int port = -1;
    bool needs_so_timestamp = false;

    auto toString() const {
        std::stringstream ss;
        ss << "SocketCfg[ip:" << ip << " port:" << port << " needs_SO_timestamp:" << needs_so_timestamp << "]";

        return ss.str();
    }
};

class TCPSocket {
  public:
    TCPSocket() = default;

    TCPSocket(const TCPSocket &) = delete;
    TCPSocket(const TCPSocket &&) = delete;
    TCPSocket &operator=(const TCPSocket &) = delete;
    TCPSocket &operator=(const TCPSocket &&) = delete;

    // Create TCPSocket with provided attributes to either listen-on
    int create(TCPSocketConfig socket_config);

    void send_message(const std::string &message) noexcept;

    // Socket attributes.
    struct sockaddr_in socket_attrib_;

  private:
    int socket_fd_ = -1;

    std::string time_str_;
};