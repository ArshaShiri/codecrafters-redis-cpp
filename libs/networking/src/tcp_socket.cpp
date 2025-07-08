#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>

#include "tcp_socket.hpp"

namespace {
// bool set_non_blocking(int fd) {
//     const auto flags = fcntl(fd, F_GETFL, 0);
//     if (flags & O_NONBLOCK)
//         return true;

//     return (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1);
// }

[[nodiscard]] int create_tcp_socket(const TCPSocketConfig &socket_config) {
    const int input_flags = AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = input_flags;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_addrlen = 0;
    hints.ai_addrlen = 0;
    hints.ai_addr = nullptr;
    hints.ai_canonname = nullptr;

    struct addrinfo *result = nullptr;
    const auto rc = getaddrinfo(socket_config.ip.c_str(), std::to_string(socket_config.port).c_str(), &hints, &result);

    if (rc != 0) {
        std::cerr << "getaddrinfo() failed. error:" << std::string(gai_strerror(rc))
                  << "errno:" + std::string(strerror(errno)) << std::endl;
        exit(EXIT_FAILURE);
    }

    int socket_fd = -1;
    int one = 1;
    for (auto *address_info = result; address_info;) {
        socket_fd = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);

        if (socket_fd == -1) {
            std::cerr << "socket() failed. errno:" + std::string(strerror(errno)) << std::endl;

            exit(EXIT_FAILURE);
        }

        // Allow re-using the address in the call to bind()
        // Bind to the specified port number.

        auto return_code =
          setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&one), sizeof(one));

        if (return_code == -1) {
            std::cerr << "setsockopt() SO_REUSEADDR failed. errno:" + std::string(strerror(errno)) << std::endl;

            exit(EXIT_FAILURE);
        }

        return_code = bind(socket_fd, address_info->ai_addr, address_info->ai_addrlen);

        if (return_code == -1) {
            std::cerr << "bind() failed. errno:%" + std::string(strerror(errno)) << std::endl;

            exit(EXIT_FAILURE);
        }

        // Enable software receive timestamps.
        if (socket_config.needs_so_timestamp) {
            const auto return_code =
              setsockopt(socket_fd, SOL_SOCKET, SO_TIMESTAMP, reinterpret_cast<void *>(&one), sizeof(one));

            if (return_code == -1) {
                std::cerr << "setSOTimestamp() failed. errno:" + std::string(strerror(errno)) << std::endl;

                exit(EXIT_FAILURE);
            }
        }

        address_info = address_info->ai_next;
        freeaddrinfo(address_info);
    }

    return socket_fd;
}
} // namespace

// Create TCPSocket with provided attributes to either listen-on / connect-to.
int TCPSocket::create(TCPSocketConfig socket_config) {
    socket_fd_ = create_tcp_socket(socket_config);

    socket_attrib_.sin_family = AF_INET;
    socket_attrib_.sin_addr.s_addr = INADDR_ANY;
    socket_attrib_.sin_port = htons(socket_config.port);

    return socket_fd_;
}

void TCPSocket::send_message(const std::string &message) noexcept {
    const auto sent_number = send(socket_fd_, message.c_str(), message.size(), 0);
    std::cout << "sent socket: " << socket_fd_ << sent_number << "bytes" << std::endl;
}