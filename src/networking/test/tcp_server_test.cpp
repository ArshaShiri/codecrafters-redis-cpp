#include <chrono>
#include <iostream>
#include <queue>
#include <stop_token>
#include <syncstream>
#include <thread>

#include <gtest/gtest.h>

#include "tcp_server.hpp"

namespace {

using namespace std::chrono_literals;
constexpr int LISTENING_PORT = 6030;

} // namespace

class ServerTest : public ::testing::Test {
  protected:
    ServerTest() : server_{LISTENING_PORT} {
        const auto server_callback = [this](auto tcp_socket) {
            const auto &socket_receive_buffer = tcp_socket->receive_buffer;
            auto message = std::string(socket_receive_buffer.begin(),
                                       socket_receive_buffer.begin() + tcp_socket->next_valid_receive_index);
            std::cout << "callback: " << message.size() << std::endl;
            received_messages_.push_back(std::move(message));
            tcp_socket->next_valid_receive_index = 0;
        };

        server_.receive_callback = server_callback;
    }

    void SetUp() override {
        server_thread_ = std::jthread([this](std::stop_token stop_token) {
            while (!stop_token.stop_requested()) {
                server_.poll();
            }
        });
    }

    void TearDown() override {
        if (server_thread_.joinable()) {
            server_thread_.request_stop();
            server_thread_.join();
        }
    }

    const std::vector<std::string> &get_received_messages() {
        return received_messages_;
    }

    TCPServer server_;
    std::jthread server_thread_;
    std::vector<std::string> received_messages_;
};

class Client {
  public:
    Client() {
        TCPSocketConfig client_tcp_socket_config;
        client_tcp_socket_config.ip = '0';
        client_tcp_socket_config.is_listening = false;
        client_tcp_socket_config.is_blocking = true;
        client_tcp_socket_config.needs_so_timestamp = false;
        client_tcp_socket_config.port = LISTENING_PORT;

        socket_fd_ = create_socket(client_tcp_socket_config);
        if (socket_fd_ < 0) {
            throw std::runtime_error("Failed to create client socket");
        }
    }

    void enqueueMessage(const std::string &message) {
        messages_.push(message);
    }

    void run() {
        while (!messages_.empty()) {
            std::this_thread::sleep_for(100ms);
            const auto message = messages_.front();
            messages_.pop();
            std::cout << "sending: " << message.size() << std::endl;
            const auto size = send(socket_fd_, message.c_str(), message.size(), 0);
            std::cout << size << " sent" << std::endl;
        }
    }

  private:
    int socket_fd_;
    std::queue<std::string> messages_;
};

// This test runs a server. Subsequently, two clients are connected to it and send some data.
// The server captures the data via a callback.
TEST_F(ServerTest, clientMessagesAreReceivedInServer) {
    const auto create_client = [](const std::vector<std::string> &client_messages, std::string client_name) {
        Client client;

        std::for_each(client_messages.begin(), client_messages.end(), [&client, &client_name](const auto &message) {
            const auto client_message = client_name + "_" + message;
            client.enqueueMessage(client_message);
        });

        return client;
    };

    const std::string message_one = "message_one";
    const std::string message_two = "message_two";
    const std::string message_three = "message_three";
    const std::vector<std::string> messages_to_be_sent{message_one, message_two, message_three};

    auto client_one = create_client(messages_to_be_sent, "client_one");
    auto client_two = create_client(messages_to_be_sent, "client_two");

    std::jthread client_one_thread([&client_one]() { client_one.run(); });
    client_two.run();

    client_one_thread.join();

    std::vector<std::string> expected_messages{"client_one_message_one",
                                               "client_one_message_two",
                                               "client_one_message_three",
                                               "client_two_message_one",
                                               "client_two_message_two",
                                               "client_two_message_three"};
    const auto received_messages = get_received_messages();

    EXPECT_EQ(received_messages.size(), expected_messages.size());
    for (const auto &expected_message : expected_messages) {
        EXPECT_TRUE(std::find(received_messages.begin(), received_messages.end(), expected_message) !=
                    received_messages.end());
    }
}

TEST_F(ServerTest, largeMessage) {
    const auto create_client = [](const std::string &client_messages) {
        Client client;
        client.enqueueMessage(client_messages);
        return client;
    };

    std::string large_message = "";
    for (std::size_t idx = 0; idx < 1024 * 1024 * 256; ++idx) {
        large_message += "Hello ";
    }
    const std::vector<std::string> messages_to_be_sent{large_message};
    auto client_one = create_client(large_message);

    client_one.run();
    server_thread_.request_stop();
    server_thread_.join();

    const auto received_messages = get_received_messages();
    std::string received_string = "";
    std::for_each(received_messages.begin(), received_messages.end(), [&received_string](const auto &message) {
        received_string += message;
    });

    EXPECT_EQ(received_string.size(), large_message.size());
}
