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
  : listening_socket_{TCPSocketConfig{LOCAL_HOST, listening_port, LISTENING_SOCKET, BLOCKING, NO_TIME_STAMP}} {
    epoll_file_descriptor_ = epoll_create(1);

    if (epoll_file_descriptor_ < 0) {
        throw SocketException("Epoll creation failed");
    }

    add_to_epoll_list(&listening_socket_, epoll_file_descriptor_);
}

TCPServer::~TCPServer() {
    close(epoll_file_descriptor_);
}

void TCPServer::poll() {
    const auto number_of_events = epoll_wait(epoll_file_descriptor_, events.data(), events.size(), 0);
    if (number_of_events < 0) {
        throw SocketException("Epoll wait failed");
    }

    for (int event_number{0}; event_number < number_of_events; ++event_number) {
        auto &event = events[event_number];
        auto socket = reinterpret_cast<TCPSocket *>(event.data.ptr);

        // Can be read
        if (event.events & EPOLLIN) {
            if (socket == &listening_socket_) {
                add_new_connections();
                continue;
            } else {
                const auto bytes_received = socket->receive();
                if (bytes_received <= 0) {
                    file_descriptor_to_socket_.erase(file_descriptor_to_socket_.find(socket->file_descriptor));
                    receive_sockets_.erase(socket);
                    epoll_ctl(epoll_file_descriptor_, EPOLL_CTL_DEL, socket->file_descriptor, &event);
                    continue;
                }
            }
        }
        // Can send
        if (event.events & EPOLLOUT) {
            receive_sockets_.insert(socket);
        }
    }

    std::for_each(receive_sockets_.begin(), receive_sockets_.end(), [](const auto &socket_ptr) { socket_ptr->send(); });
}

void TCPServer::enqueue_to_send_buffer(int client_file_descriptor, const std::string &message) {
    if (auto client = file_descriptor_to_socket_.find(client_file_descriptor);
        client != file_descriptor_to_socket_.end()) {
        client->second->enqueue_to_send_buffer(message);
    }
}

void TCPServer::add_new_connections() {
    for (;;) {
        struct sockaddr_storage client_address;
        socklen_t client_len = sizeof(client_address);

        const auto client_file_descriptor =
          accept(listening_socket_.file_descriptor, (struct sockaddr *)&client_address, &client_len);
        if (client_file_descriptor == INVALID_SOCKET) {
            break;
        }

        std::cout << "There is a new connection" << std::endl;
        if (!set_non_blocking(client_file_descriptor)) {
            throw SocketException("setNonBlocking() failed.");
        }

        auto socket = std::make_unique<TCPSocket>(client_file_descriptor);
        socket->receive_callback = receive_callback;
        add_to_epoll_list(socket.get(), epoll_file_descriptor_);
        file_descriptor_to_socket_.emplace(client_file_descriptor, std::move(socket));
    }
}
