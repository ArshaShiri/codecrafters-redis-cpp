#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <stdexcept>
#include <string.h>
#include <sys/socket.h>

#include "common.hpp"
#include "socket_utils.hpp"

namespace {
/// Represents the maximum number of pending / unaccepted TCP connections.
constexpr int MAX_TCP_SERVER_BACKLOG = 1024;
} // namespace

bool set_non_blocking(int fd) {
    const auto flags = fcntl(fd, F_GETFL, 0);
    if (flags & O_NONBLOCK)
        return true;

    return (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1);
}

// Create TCPSocket with provided attributes to either listen-on / connect-to.
[[nodiscard]] int create_socket(const TCPSocketConfig &socket_config) {
    const int input_flags = (socket_config.is_listening ? AI_PASSIVE : 0) | AI_NUMERICHOST | AI_NUMERICSERV;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = input_flags;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_addr = nullptr;
    hints.ai_canonname = nullptr;

    struct addrinfo *result = nullptr;
    const auto rc = getaddrinfo(socket_config.ip.c_str(), std::to_string(socket_config.port).c_str(), &hints, &result);

    if (rc != 0) {
        throw SocketException("getaddrinfo() failed.");
    }

    int socket_fd = -1;
    int one = 1;

    socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (socket_fd == INVALID_SOCKET) {
        throw SocketException("socket() failed.");
    }

    if (!socket_config.is_blocking) {
        if (!set_non_blocking(socket_fd)) {
            throw SocketException("setNonBlocking() failed.");
        }
    }

    if (socket_config.is_listening) {
        // Allow re-using the address in the call to bind()
        // Bind to the specified port number.
        auto return_code =
          setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&one), sizeof(one));

        if (return_code == -1) {
            throw SocketException("setsockopt() SO_REUSEADDR failed.");
        }

        return_code = bind(socket_fd, result->ai_addr, result->ai_addrlen);
        if (return_code == -1) {
            throw SocketException("bind() failed.");
        }

        return_code = listen(socket_fd, MAX_TCP_SERVER_BACKLOG);
        if (return_code != 0) {
            throw SocketException("listen() failed.");
        }

    } else {
        connect(socket_fd, result->ai_addr, result->ai_addrlen);
    }


    // Enable software receive timestamps.
    if (socket_config.needs_so_timestamp) {
        const auto return_code =
          setsockopt(socket_fd, SOL_SOCKET, SO_TIMESTAMP, reinterpret_cast<void *>(&one), sizeof(one));

        if (return_code == -1) {
            throw SocketException("setSOTimestamp() failed.");
        }
    }

    freeaddrinfo(result);

    return socket_fd;
}
