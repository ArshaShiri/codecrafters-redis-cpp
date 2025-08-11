#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include "tcp_socket.hpp"

namespace {
// Size of our send and receive buffers in bytes.
constexpr std::size_t TCP_BUFFER_SIZE = 64 * 1024 * 1024;
} // namespace

TCPSocket::TCPSocket(const TCPSocketConfig &socket_config)
  : receive_buffer(TCP_BUFFER_SIZE), send_buffer(TCP_BUFFER_SIZE) {
    file_descriptor = create_socket(socket_config);
}

TCPSocket::TCPSocket(int file_descriptor)
  : file_descriptor{file_descriptor}, receive_buffer(TCP_BUFFER_SIZE), send_buffer(TCP_BUFFER_SIZE) {
}

int TCPSocket::receive() noexcept {
    const auto bytes_received = recv(
      file_descriptor, receive_buffer.data() + next_valid_receive_index, TCP_BUFFER_SIZE - next_valid_receive_index, 0);

    if (bytes_received <= 0) {
        return bytes_received;
    }

    // std::cout << bytes_received << " bytes received:\n"
    //           << std::string{receive_buffer.data() + next_valid_receive_index,
    //                          receive_buffer.data() + next_valid_receive_index + bytes_received}
    //           << std::endl;

    next_valid_receive_index += bytes_received;

    if (receive_callback != nullptr) {
        receive_callback(this);
    } else {
        std::cout << "No callback is set printing the received message:" << std::endl;
    }

    return bytes_received;
}

void TCPSocket::send() noexcept {
    auto message = std::string(send_buffer.data(), next_valid_send_index);
    ::send(file_descriptor, send_buffer.data(), next_valid_send_index, MSG_DONTWAIT | MSG_NOSIGNAL);
    next_valid_send_index = 0;
}

void TCPSocket::enqueue_to_send_buffer(const std::string &message) {
    const auto buffer_size = send_buffer.size() - next_valid_send_index;
    if (message.size() > buffer_size) {
        throw std::overflow_error("Send buffer is full");
    }

    // TODO Extra copy (message can be written to the send buffer directly)
    std::copy(message.begin(), message.end(), send_buffer.begin() + next_valid_send_index);
    next_valid_send_index = message.size();
    auto message_ = std::string(send_buffer.data(), next_valid_send_index);
}

TCPSocket::~TCPSocket() {
    if (file_descriptor != -1) {
        close(file_descriptor);
    }
}
