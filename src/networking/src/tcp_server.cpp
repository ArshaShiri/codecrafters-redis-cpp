#include <array>
#include <iostream>
#include <sys/epoll.h>

#include "tcp_server.hpp"

namespace {
constexpr auto LOCAL_HOST = "0";
constexpr auto LISTENING_SOCKET = true;
constexpr auto BLOCKING = false;
constexpr auto NO_TIME_STAMP = false;

std::array<epoll_event, 1024> events;

void add_to_epoll_list(TCPSocket *socket, int epoll_file_descriptor) {
    epoll_event ev{EPOLLET | EPOLLIN | EPOLLOUT, {reinterpret_cast<void *>(socket)}};

    if (epoll_ctl(epoll_file_descriptor, EPOLL_CTL_ADD, socket->file_descriptor, &ev)) {
        throw SocketException("Failed to add to epoll list");
    }
}
} // namespace

TCPServer::TCPServer(int listening_port)
  : listening_socket{TCPSocketConfig{LOCAL_HOST, listening_port, LISTENING_SOCKET, BLOCKING, NO_TIME_STAMP}} {
    epoll_file_descriptor = epoll_create(1);

    if (epoll_file_descriptor < 0) {
        throw SocketException("Epoll creation failed");
    }

    add_to_epoll_list(&listening_socket, epoll_file_descriptor);
}

void TCPServer::poll() {
    const auto max_number_of_events = sockets.size() + 1;
    const auto number_of_file_events = epoll_wait(epoll_file_descriptor, events.data(), max_number_of_events, 0);
    if (number_of_file_events < 0) {
        throw SocketException("Epoll wait failed");
    }

    for (int event_number{0}; event_number < number_of_file_events; ++event_number) {
        const auto &event = events[event_number];
        auto socket = reinterpret_cast<TCPSocket *>(event.data.ptr);

        // Available for read
        if (event.events & EPOLLIN) {
            if (socket == &listening_socket) {
                add_new_connections();
                continue;
            } else {
                const auto bytes_received = socket->receive();
                if (bytes_received <= 0) {
                    std::cout << "Connection is closed" << std::endl;
                    sockets.erase(
                      std::remove_if(sockets.begin(), sockets.end(), [&](const auto &s) { return s.get() == socket; }),
                      sockets.end());
                    continue;
                }
            }
        }
        if (event.events & EPOLLOUT) {
            socket->send();
        }
    }
}

void TCPServer::add_new_connections() {
    for (;;) {
        struct sockaddr_storage client_address;
        socklen_t client_len = sizeof(client_address);

        const auto client_file_descriptor =
          accept(listening_socket.file_descriptor, (struct sockaddr *)&client_address, &client_len);
        if (client_file_descriptor == INVALID_SOCKET) {
            break;
        }

        std::cout << "There is a new connection" << std::endl;
        if (!set_non_blocking(client_file_descriptor)) {
            throw SocketException("setNonBlocking() failed.");
        }

        auto socket = std::make_unique<TCPSocket>(client_file_descriptor);
        socket->recv_callback = recv_callback;
        add_to_epoll_list(socket.get(), epoll_file_descriptor);
        sockets.push_back(std::move(socket));
    }
}
