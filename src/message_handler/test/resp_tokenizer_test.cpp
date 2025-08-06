#include <chrono>
#include <iostream>
#include <syncstream>
#include <thread>

#include <gtest/gtest.h>

#include "resp_tokenizer.hpp"

void print_tokents() {
}

TEST(RESPTokenizer, ping) {
    // Ping command encoded in RESP
    const std::string resp_ping = "*1\r\n$4\r\nPING\r\n";

    RESPTokenizer tokenizer(resp_ping.data(), resp_ping.size());
    const auto &tokens = tokenizer.get_tokens();
    const std::vector<Token> expected_tokens{Token{TokenType::ARRAY, 1}, Token{TokenType::BULK_STRING, "PING"}};

    EXPECT_EQ(tokens, expected_tokens);
}

TEST(RESPTokenizer, echo) {
    // Echo command encoded in RESP
    const std::string resp_echo = "*2\r\n$4\r\nECHO\r\n$3\r\nhey\r\n";

    RESPTokenizer tokenizer(resp_echo.data(), resp_echo.size());
    const auto &tokens = tokenizer.get_tokens();
    const std::vector<Token> expected_tokens{
      Token{TokenType::ARRAY, 2}, Token{TokenType::BULK_STRING, "ECHO"}, Token{TokenType::BULK_STRING, "hey"}};

    EXPECT_EQ(tokens, expected_tokens);
}

TEST(RESPTokenizer, setAndGet) {
    // Set and Get commands encoded in RESP
    const std::string resp_set = "*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n";
    const std::string resp_get = "*2\r\n$3\r\nGET\r\n$3\r\nkey\r\n";

    RESPTokenizer set_tokenizer(resp_set.data(), resp_set.size());
    const auto &set_tokens = set_tokenizer.get_tokens();
    const std::vector<Token> expected_set_tokens{Token{TokenType::ARRAY, 3},
                                                 Token{TokenType::BULK_STRING, "SET"},
                                                 Token{TokenType::BULK_STRING, "key"},
                                                 Token{TokenType::BULK_STRING, "value"}};

    EXPECT_EQ(set_tokens, expected_set_tokens);

    RESPTokenizer get_tokenizer(resp_get.data(), resp_get.size());
    const auto &get_tokens = get_tokenizer.get_tokens();
    const std::vector<Token> expected_get_tokens{
      Token{TokenType::ARRAY, 2}, Token{TokenType::BULK_STRING, "GET"}, Token{TokenType::BULK_STRING, "key"}};

    EXPECT_EQ(get_tokens, expected_get_tokens);
}

TEST(RESPTokenizer, setWithExpiry) {
    // Set command with expiry encoded in RESP
    const std::string resp_set_expiry = "*5\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n$2\r\npx\r\n$3\r\n100\r\n";

    RESPTokenizer tokenizer(resp_set_expiry.data(), resp_set_expiry.size());
    const auto &tokens = tokenizer.get_tokens();
    const std::vector<Token> expected_tokens{Token{TokenType::ARRAY, 5},
                                             Token{TokenType::BULK_STRING, "SET"},
                                             Token{TokenType::BULK_STRING, "key"},
                                             Token{TokenType::BULK_STRING, "value"},
                                             Token{TokenType::BULK_STRING, "px"},
                                             Token{TokenType::BULK_STRING, "100"}};

    EXPECT_EQ(tokens, expected_tokens);
}
