#include <iostream>
#include <string>

#include "redis.hpp"
#include "response_generator.hpp"
#include "tcp_socket.hpp"

namespace {
const auto server_callback = [](auto client_socket) {
    const auto &socket_receive_buffer = client_socket->receive_buffer;
    const auto message = std::string(socket_receive_buffer.begin(),
                                     socket_receive_buffer.begin() + client_socket->next_valid_receive_index);
    std::cout << "message:\n" << message << std::endl;
    const auto response = generate_response(socket_receive_buffer.data(), client_socket->next_valid_receive_index);
    client_socket->next_valid_receive_index = 0;

    client_socket->enqueue_to_send_buffer(response);
    client_socket->next_valid_send_index = response.size();
};
} // namespace

Redis::Redis(int listening_port) : server_{listening_port} {
    server_.recv_callback = server_callback;
}

void Redis::run() {
    for (;;) {
        server_.poll();
    }
}
