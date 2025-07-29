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
