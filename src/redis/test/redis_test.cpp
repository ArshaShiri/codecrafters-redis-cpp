#include <chrono>
#include <iostream>
#include <mutex>
#include <queue>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "redis.hpp"

namespace {
static constexpr int LISTENING_PORT = 6000;
using namespace std::chrono_literals;

class Client {
  public:
    Client() {
        TCPSocketConfig client_tcp_socket_config;
        client_tcp_socket_config.ip = '0';
        client_tcp_socket_config.is_listening = false;
        client_tcp_socket_config.is_blocking = false;
        client_tcp_socket_config.needs_so_timestamp = false;
        client_tcp_socket_config.port = LISTENING_PORT;

        socket_fd_ = create_socket(client_tcp_socket_config);
        if (socket_fd_ < 0) {
            throw std::runtime_error("Failed to create client socket");
        }
    }

    void enqueueMessage(const std::string &message) {
        std::unique_lock<std::mutex> lock(message_mutex_);
        messages_.push(message);
    }

    void run(std::stop_token stop_token,
             std::queue<std::string> &expected_responses,
             std::queue<std::chrono::milliseconds> delays = {}) {
        while (!stop_token.stop_requested()) {
            std::unique_lock<std::mutex> lock(message_mutex_);
            if (!messages_.empty()) {
                const auto message = messages_.front();
                messages_.pop();
                lock.unlock();

                send(socket_fd_, message.c_str(), message.size(), 0);

                bool received_response = false;
                while (!received_response && !stop_token.stop_requested()) {
                    char buffer[1024];
                    const auto bytes_received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);

                    if (bytes_received > 0) {
                        received_response = true;
                        buffer[bytes_received] = '\0'; // Null-terminate the received data

                        // Check if the response matches the current expected response
                        if (!expected_responses.empty()) {
                            const auto expected_response = expected_responses.front();
                            EXPECT_STREQ(buffer, expected_response.c_str());
                            expected_responses.pop();
                        }
                    }
                }
                // Simulate some delay in requests from the client
                if (!delays.empty()) {
                    const auto delay = delays.front();
                    std::this_thread::sleep_for(delay);
                    delays.pop();
                }
            } else {
                lock.unlock();
            }
        }
    }

  private:
    int socket_fd_;
    std::mutex message_mutex_;
    std::queue<std::string> messages_;
};

} // namespace

class RedisTest : public ::testing::Test {
  protected:
    void SetUp() override {
        server_thread_ = std::jthread([this]() { redis_server_.run(); });
    }

    void TearDown() override {
        redis_server_.stop();
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }

  private:
    Redis redis_server_{LISTENING_PORT};
    std::jthread server_thread_;
};

TEST_F(RedisTest, pingCommand) {
    std::queue<std::string> expected_responses;

    const auto ping_command = "*1\r\n$4\r\nPING\r\n";
    expected_responses.push("+PONG\r\n");

    Client client;
    client.enqueueMessage(ping_command);

    std::jthread client_thread(
      [&client, &expected_responses](std::stop_token stop_token) { client.run(stop_token, expected_responses); });
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST_F(RedisTest, echoCommand) {
    std::queue<std::string> expected_responses;

    const auto echo_command = "*2\r\n$4\r\nECHO\r\n$10\r\nmy_message\r\n";
    expected_responses.push("$10\r\nmy_message\r\n");

    Client client;
    client.enqueueMessage(echo_command);

    std::jthread client_thread(
      [&client, &expected_responses](std::stop_token stop_token) { client.run(stop_token, expected_responses); });
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST_F(RedisTest, setAndGetCommand) {
    std::queue<std::string> expected_responses;

    const auto set_command_one = "*3\r\n$3\r\nSET\r\n$7\r\nkey_one\r\n$9\r\nvalue_one\r\n";
    const auto set_command_two = "*3\r\n$3\r\nSET\r\n$14\r\nlonger_key_two\r\n$16\r\nlonger_value_two\r\n";
    expected_responses.push("+OK\r\n");
    expected_responses.push("+OK\r\n");

    const auto get_command_one = "*2\r\n$3\r\nGET\r\n$7\r\nkey_one\r\n";
    expected_responses.push("$9\r\nvalue_one\r\n");

    const auto get_command_two = "*2\r\n$3\r\nGET\r\n$14\r\nlonger_key_two\r\n";
    expected_responses.push("$16\r\nlonger_value_two\r\n");

    Client client;
    client.enqueueMessage(set_command_one);
    client.enqueueMessage(set_command_two);
    client.enqueueMessage(get_command_one);
    client.enqueueMessage(get_command_two);

    std::queue<std::chrono::milliseconds> delays;
    for (std::size_t idx{0}; idx < expected_responses.size(); ++idx) {
        delays.push(100ms);
    }

    std::jthread client_thread([&client, &expected_responses, &delays](std::stop_token stop_token) {
        client.run(stop_token, expected_responses, delays);
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));
}

TEST_F(RedisTest, setCommandWithExpiry) {
    std::queue<std::string> expected_responses;

    const auto set_command = "*5\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n$2\r\npx\r\n$3\r\n100\r\n";
    expected_responses.push("+OK\r\n");

    const auto get_command = "*2\r\n$3\r\nGET\r\n$3\r\nkey\r\n";

    // Key exists before expiry
    expected_responses.push("$5\r\nvalue\r\n");

    // Key does not exist after expiry
    expected_responses.push("$-1\r\n");

    Client client;
    client.enqueueMessage(set_command);
    client.enqueueMessage(get_command);
    client.enqueueMessage(get_command);

    std::queue<std::chrono::milliseconds> delays;

    // Before the first get command wait for 10ms.
    // The key should exists as its expiry is set to 100ms.
    delays.push(10ms);

    // Wait another 100ms to ensure the key is invalidated.
    delays.push(100ms);


    std::jthread client_thread([&client, &expected_responses, &delays](std::stop_token stop_token) {
        client.run(stop_token, expected_responses, delays);
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));
}
