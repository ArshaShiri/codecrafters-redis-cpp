#pragma once

#include <iostream>
#include <string>
#include <variant>
#include <vector>

enum class TokenType {
    SIMPLE_STRING, // +
    ERROR, // -
    INTEGER, // :
    BULK_STRING, // $
    ARRAY // *
};

struct Token {
    TokenType type;
    std::variant<std::string_view, int> value;

    bool operator==(const Token &other) const {
        return (type == other.type) && (value == other.value);
    };
};

class RESPTokenizer {
  public:
    RESPTokenizer(const char *input, std::size_t input_size);
    std::vector<Token> &get_tokens();
    void print_tokens() const;

  private:
    std::string_view get_view_before_the_next_CRLF();

    void add_array();
    void add_bulk_string();

    std::string_view input_view_;
    std::size_t position_{0};
    std::vector<Token> tokens_{};
};
