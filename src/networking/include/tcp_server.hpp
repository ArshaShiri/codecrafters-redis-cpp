#pragma once

#include <memory>

#include "tcp_socket.hpp"

class TCPServer {
  public:
    explicit TCPServer(int listening_port);

    TCPServer() = delete;
    TCPServer(const TCPServer &) = delete;
    TCPServer(const TCPServer &&) = delete;
    TCPServer &operator=(const TCPServer &) = delete;
    TCPServer &operator=(const TCPServer &&) = delete;

    /**
     * @brief Run the server
     *
     */
    void poll();


    // Function wrapper to call back when data is available.
    std::function<void(TCPSocket *socket)> recv_callback = nullptr;

  private:
    void add_new_connections();

    // Socket on which this server is listening for new connections on.
    TCPSocket listening_socket;
    int epoll_file_descriptor = -1;

    // Collection of all sockets.
    std::vector<std::unique_ptr<TCPSocket>> sockets;
};
