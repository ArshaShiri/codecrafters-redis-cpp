#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include "socket_utils.hpp"

struct TCPSocket {
    TCPSocket(const TCPSocketConfig &socket_config);
    TCPSocket(int file_descriptor);

    TCPSocket() = delete;
    TCPSocket(const TCPSocket &) = delete;
    TCPSocket(const TCPSocket &&) = delete;
    TCPSocket &operator=(const TCPSocket &) = delete;
    TCPSocket &operator=(const TCPSocket &&) = delete;
    ~TCPSocket();

    /**
     * @brief Receive bytes from the socket and return the number of bytes received.
     *
     * @return int number of bytes received. (Returns the value from the recv system call)
     */
    int receive() noexcept;

    /**
     * @brief Send the content of send buffer
     *
     */
    void send() noexcept;

    /**
     * @brief Enqueue a message to the send buffer.
     *
     * @param message The message to be sent.
     */
    void enqueue_to_send_buffer(const std::string &message);

    // File descriptor for the socket.
    int file_descriptor = -1;

    // Send and receive buffers and trackers for read/write indices.
    std::vector<char> receive_buffer;
    std::vector<char> send_buffer;
    std::size_t next_valid_receive_index = 0;
    std::size_t next_valid_send_index = 0;

    // Function wrapper to callback when there is data to be processed.
    std::function<void(TCPSocket *socket)> receive_callback = nullptr;
};
