#include <chrono>
#include <iostream>
#include <syncstream>
#include <thread>

#include <gtest/gtest.h>

#include "tcp_server.hpp"

namespace {
using namespace std::chrono_literals;
} // namespace


// This test runs a server. Subsequently, two clients are connected to it and send some data.
// The server captures the data via a callback.
TEST(TCPServer, serverAndClient) {
    const auto server_port = 6030;
    std::vector<std::string> received_messages{};

    const auto server_callback = [&received_messages](auto tcp_socket) {
        const auto &socket_receive_buffer = tcp_socket->receive_buffer;
        auto message = std::string(socket_receive_buffer.begin(),
                                   socket_receive_buffer.begin() + tcp_socket->next_valid_receive_index);
        std::cout << "message: " << message << std::endl;
        received_messages.push_back(std::move(message));
        tcp_socket->next_valid_receive_index = 0;
    };

    TCPServer tcp_server{server_port};
    tcp_server.recv_callback = server_callback;

    TCPSocketConfig client_tcp_socket_config;
    client_tcp_socket_config.ip = '0';
    client_tcp_socket_config.is_listening = false;
    client_tcp_socket_config.is_blocking = true;
    client_tcp_socket_config.needs_so_timestamp = false;
    client_tcp_socket_config.port = server_port;

    const auto client_one_socket_fd = create_socket(client_tcp_socket_config);
    const auto client_two_socket_fd = create_socket(client_tcp_socket_config);

    const std::string message_one = "message_one";
    const std::string message_two = "message_two";
    const std::string message_three = "message_three";
    const auto messages_to_be_sent = std::vector<std::string>{message_one, message_two, message_three};

    const auto run_client = [&](const auto client_fd, std::string client_name) {
        for (const auto &message : messages_to_be_sent) {
            std::this_thread::sleep_for(1000ms);
            const auto client_message = client_name + "_" + message;
            send(client_fd, client_message.c_str(), client_message.size(), 0);
        }
    };

    const auto run_server = [&](std::stop_token stop_token) {
        while (!stop_token.stop_requested()) {
            tcp_server.poll();
        }
    };

    auto server_thread = std::jthread(run_server);
    auto client_one_thread = std::jthread(run_client, client_one_socket_fd, "client_one");
    auto client_two_thread = std::jthread(run_client, client_two_socket_fd, "client_two");

    client_one_thread.join();
    close(client_one_socket_fd);

    client_two_thread.join();
    close(client_two_socket_fd);

    std::this_thread::sleep_for(2000ms);
    server_thread.request_stop();
    server_thread.join();

    std::vector<std::string> expected_messages{"client_one_message_one",
                                               "client_one_message_two",
                                               "client_one_message_three",
                                               "client_two_message_one",
                                               "client_two_message_two",
                                               "client_two_message_three"};

    EXPECT_EQ(received_messages.size(), expected_messages.size());
    for (const auto &expected_message : expected_messages) {
        EXPECT_TRUE(std::find(received_messages.begin(), received_messages.end(), expected_message) !=
                    received_messages.end());
    }
}
