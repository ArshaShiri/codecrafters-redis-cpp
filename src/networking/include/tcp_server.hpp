#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "tcp_socket.hpp"

class TCPServer {
  public:
    explicit TCPServer(int listening_port);

    TCPServer() = delete;
    TCPServer(const TCPServer &) = delete;
    TCPServer(const TCPServer &&) = delete;
    TCPServer &operator=(const TCPServer &) = delete;
    TCPServer &operator=(const TCPServer &&) = delete;
    ~TCPServer();

    /**
     * @brief Run the server
     *
     */
    void poll();

    /**
     * @brief Enqueue a message to the send buffer of a clinet.
     *
     * @param client_file_descriptor Socket file descriptor of the client.
     *
     * @param message The message to be sent.
     */
    void enqueue_to_send_buffer(int client_file_descriptor, const std::string &message);

    // Function wrapper to call back when data is available.
    std::function<void(TCPSocket *socket)> receive_callback = nullptr;

  private:
    void add_new_connections();

    // Socket on which this server is listening for new connections on.
    TCPSocket listening_socket_;
    int epoll_file_descriptor_{-1};

    // Collection of all sockets.
    std::unordered_map<int, std::unique_ptr<TCPSocket>> file_descriptor_to_socket_;
    std::unordered_set<TCPSocket *> receive_sockets_;
};
