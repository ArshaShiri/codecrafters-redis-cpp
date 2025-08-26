#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "redis.hpp"
#include "tcp_socket.hpp"

namespace {
constexpr std::size_t SIZE_OF_MESSAGE_QUEUE = 1024;
} // namespace

Redis::Redis(RedisConfig config)
  : server_{config.listening_port}, message_queue_{SIZE_OF_MESSAGE_QUEUE}, response_queue_{SIZE_OF_MESSAGE_QUEUE},
    data_manager_{}, rdb_handler_{config.rdb_config, data_manager_}, role_{config.role} {

    const auto server_receive_callback = [&](auto client_socket) {
        const auto &socket_receive_buffer = client_socket->receive_buffer;
        auto message = std::string(socket_receive_buffer.data(), client_socket->next_valid_receive_index);
        client_socket->next_valid_receive_index = 0;
        message_queue_.push({client_socket->file_descriptor, std::move(message)});
    };

    server_.receive_callback = server_receive_callback;
}

void Redis::run() {
    run_tcp_server();
    run_message_handler();
}

void Redis::stop() {
    running_ = false;
    tcp_server_thread_.request_stop();
}

void Redis::run_tcp_server() {
    tcp_server_thread_ = std::jthread([this](std::stop_token stop_token) {
        while (!stop_token.stop_requested()) {
            ResponseMessage response_message;
            if (response_queue_.pop(response_message)) {
                server_.enqueue_to_send_buffer(response_message.client_id, response_message.message);
            }
            server_.poll();
        }
    });
}

void Redis::run_message_handler() {
    auto message_handler = MessageHandler{*this, data_manager_};
    RequestMessage request_message;
    while (running_) {
        if (message_queue_.pop(request_message)) {
            std::cout << "message:\n" << request_message.message << std::endl;
            auto response = message_handler.genera_response(request_message.message);
            std::cout << "response:\n" << response << std::endl;
            response_queue_.push({request_message.client_id, std::move(response)});
        }
    }
}
