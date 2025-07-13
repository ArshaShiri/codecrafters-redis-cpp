#include <iostream>
#include <netdb.h>
#include <unistd.h>

#include "common.hpp"
#include "socket_utils.hpp"
#include "tcp_server.hpp"

TCPServer::TCPServer(int listening_port) {
    TCPSocketConfig config;
    config.ip = "0";
    config.port = listening_port;
    config.is_listening = true;
    config.is_blocking = true;
    config.needs_so_timestamp = false;

    listening_socket_ = create_socket(config);
}

TCPServer::~TCPServer() {
    close(listening_socket_);
}

void TCPServer::run() {
    fd_set sockets;
    FD_ZERO(&sockets);
    FD_SET(listening_socket_, &sockets);
    auto max_socket = listening_socket_;

    for (;;) {
        fd_set reads = sockets;

        if (select(max_socket + 1, &reads, 0, 0, 0) < 0) {
            throw SocketException("select failed.");
        }

        for (int socket_fd = 1; socket_fd <= max_socket; ++socket_fd) {
            if (!FD_ISSET(socket_fd, &reads)) {
                continue;
            }

            // Handle a new connection
            if (socket_fd == listening_socket_) {
                struct sockaddr_storage client_address;
                socklen_t client_len = sizeof(client_address);
                auto client_socket = accept(listening_socket_, (struct sockaddr *)&client_address, &client_len);

                if (client_socket == INVALID_SOCKET) {
                    throw SocketException("Cannot get clinet socket");
                }

                FD_SET(client_socket, &sockets);
                max_socket = std::max(max_socket, client_socket);

                // Print clinet address info
                char address_buffer[100];
                getnameinfo((struct sockaddr *)&client_address, client_len, address_buffer, 0, 0, 0, NI_NUMERICHOST);
                std::cout << "New connectio from: " << address_buffer << std::endl;
            } else {
                char read[1024];
                const auto bytes_received = recv(socket_fd, read, sizeof(read), 0);

                const auto connection_closed = bytes_received < 1;
                if (connection_closed) {
                    FD_CLR(socket_fd, &sockets);
                    close(socket_fd);
                    std::cout << "connection closed: " << socket_fd << std::endl;
                    continue;
                }

                // For now ignore the incoming message

                const std::string TEST_MESSAGE = "pong";
                const auto bytes_sent = send(socket_fd, TEST_MESSAGE.c_str(), TEST_MESSAGE.size(), 0);
                std::cout << "sent: " << bytes_sent << " bytes" << std::endl;
            }
        }
    }
}