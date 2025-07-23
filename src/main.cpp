#include <iostream>

#include "tcp_server.hpp"

int main() {
    const auto server_callback = [](auto client_socket) {
        const auto &socket_receive_buffer = client_socket->receive_buffer;
        const auto message = std::string(socket_receive_buffer.begin(),
                                         socket_receive_buffer.begin() + client_socket->next_valid_receive_index);
        std::cout << "message:\n" << message << std::endl;
        client_socket->next_valid_receive_index = 0;

        std::string response = "+PONG\r\n";
        client_socket->enqueue_to_send_buffer(response);
        client_socket->next_valid_send_index = response.size();
    };

    TCPServer server{6379};
    server.recv_callback = server_callback;

    for (;;) {
        server.poll();
    }

    return 0;
}
