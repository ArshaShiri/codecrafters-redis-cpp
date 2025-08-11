#include <charconv>
#include <iostream>
#include <stdexcept>
#include <system_error>

#include "common.hpp"
#include "resp_tokenizer.hpp"

namespace {
const std::string CRLF = "\r\n";
}

RESPTokenizer::RESPTokenizer() {
}

std::vector<Token> &RESPTokenizer::generate_tokens(std::string_view input) {
    position_ = 0;
    tokens_.clear();
    input_view_ = input;

    while (position_ < input_view_.size()) {
        const auto prefix = input_view_[position_];
        position_++;

        switch (prefix) {
        case '*':
            add_array();
            break;
        case '$':
            add_bulk_string();
            break;
        default:
            throw std::runtime_error("Invalid RESP prefix: " + std::string(1, prefix));
        }
    }

    return tokens_;
}

std::ostream &operator<<(std::ostream &os, const Token &token) {
    auto toke_type = "";

    switch (token.type) {
    case TokenType::ARRAY:
        toke_type = "ARRAY";
        break;
    case TokenType::BULK_STRING:
        toke_type = "BULK_STRING";
        break;

    default:
        toke_type = "Unknown";
        break;
    }
    os << "Token type: " << toke_type << '\n' << "Token value: ";

    if (token.type == TokenType::ARRAY) {
        os << std::get<int>(token.value);
    } else {
        os << std::get<std::string_view>(token.value);
    }
    os << '\n';

    return os;
}

void RESPTokenizer::print_tokens() const {
    for (const auto &token : tokens_) {
        std::cout << token;
    }
}

std::string_view RESPTokenizer::get_view_before_the_next_CRLF() {
    const auto CRLF_postion = input_view_.find(CRLF, position_);
    const auto length = (CRLF_postion - position_);
    const auto result = input_view_.substr(position_, length);

    // Skip the CRLF
    position_ += length + 2;
    return result;
}

void RESPTokenizer::add_array() {
    const auto array_length_view = get_view_before_the_next_CRLF();
    const auto token = Token{TokenType::ARRAY, get_int_from_string_view(array_length_view)};
    tokens_.emplace_back(token);
}

void RESPTokenizer::add_bulk_string() {
    // TODO: Handle null bulk strings
    const auto length = get_int_from_string_view(get_view_before_the_next_CRLF());
    tokens_.emplace_back(Token{TokenType::BULK_STRING, std::string_view(input_view_.data() + position_, length)});
    position_ += length + 2;
}
