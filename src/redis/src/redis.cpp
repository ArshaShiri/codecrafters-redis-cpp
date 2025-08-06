#include <iostream>
#include <string>

#include "message_handler.hpp"
#include "redis.hpp"
#include "tcp_socket.hpp"

Redis::Redis(int listening_port) : server_{listening_port}, data_manager_{} {

    const auto server_callback = [&](auto client_socket) {
        const auto &socket_receive_buffer = client_socket->receive_buffer;
        const auto message = std::string(socket_receive_buffer.begin(),
                                         socket_receive_buffer.begin() + client_socket->next_valid_receive_index);

        std::cout << "message:\n" << message << std::endl;

        const auto message_handler =
          MessageHandler{socket_receive_buffer.data(), client_socket->next_valid_receive_index, data_manager_};
        const auto &response = message_handler.get_response();
        client_socket->next_valid_receive_index = 0;

        std::cout << "response:\n" << response << std::endl;

        client_socket->enqueue_to_send_buffer(response);
        client_socket->next_valid_send_index = response.size();
    };

    server_.recv_callback = server_callback;
}

void Redis::run() {
    while (running_) {
        server_.poll();
    }
}
