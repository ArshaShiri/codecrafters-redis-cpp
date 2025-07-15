#include <chrono>
#include <cstring>
#include <syncstream>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "include/socket_utils.hpp"

namespace {

using namespace std::chrono_literals;
std::osyncstream syncStream(std::cout);

} // namespace

TEST(TCPSocket, sendAndReceiveSocket) {
    TCPSocketConfig server_tcp_socket_config;
    server_tcp_socket_config.ip = '0';
    server_tcp_socket_config.is_listening = true;
    server_tcp_socket_config.is_blocking = true;
    server_tcp_socket_config.needs_so_timestamp = false;
    server_tcp_socket_config.port = 6379;

    const auto server_socket_fd = create_socket(server_tcp_socket_config);


    const std::string message_one = "message_one";
    const std::string message_two = "message_two";
    const std::string message_three = "message_three";
    const auto messages_to_be_sent = std::vector<std::string>{message_one, message_two, message_three};

    std::atomic<bool> is_done{false};

    auto run_server = [&]() {
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        syncStream << "Waiting for a client to connect...\n";

        const auto client_fd = accept(server_socket_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
        syncStream << "Client connected " << client_fd << " server_socket_fd: " << server_socket_fd
                   << " (errno: " + std::to_string(errno) + " - " + std::strerror(errno) + ")" << '\n';

        for (const auto &message : messages_to_be_sent) {
            std::this_thread::sleep_for(1000ms);
            const auto bytes_sent = send(client_fd, message.c_str(), message.size(), 0);
            syncStream << "Sent socket: " << client_fd << ", sending: " << bytes_sent << "bytes" << std::endl;
        }

        is_done.store(true);
    };

    TCPSocketConfig client_tcp_socket_config;
    client_tcp_socket_config.ip = '0';
    client_tcp_socket_config.is_listening = false;
    client_tcp_socket_config.is_blocking = true;
    client_tcp_socket_config.needs_so_timestamp = false;
    client_tcp_socket_config.port = 6379;

    const auto client_socket_fd = create_socket(client_tcp_socket_config);
    std::vector<std::string> received_messages{};

    auto run_client = [&]() {
        for (std::size_t idx{0}; idx < messages_to_be_sent.size(); ++idx) {
            char received[1024];
            const auto bytes_received = recv(client_socket_fd, received, 1024, 0);
            syncStream << "Received socket: " << client_socket_fd << ", received: " << bytes_received << "bytes"
                       << std::endl;
            received_messages.push_back(received);
        }
    };

    auto server = std::jthread(run_server);
    auto clinet = std::jthread(run_client);
    server.join();
    clinet.join();

    close(client_socket_fd);
    close(server_socket_fd);

    EXPECT_EQ(messages_to_be_sent, received_messages);
}
