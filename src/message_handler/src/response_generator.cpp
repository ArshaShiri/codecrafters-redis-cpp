#include "response_generator.hpp"
#include "resp_tokenizer.hpp"

namespace {
std::string generate_ping_response() {
    return "+PONG\r\n";
}

std::string generate_error_response(const std::string &error_message) {
    return "-ERR " + error_message + "\r\n";
}

std::string generate_echo_response(const std::vector<Token> &tokens) {
    const auto number_of_array_elements = std::get<int>(tokens[0].value);
    if (number_of_array_elements != 2) {
        return generate_error_response("ECHO command expects exactly one argument");
    }

    const auto &echo_token = tokens[2];
    if (echo_token.type != TokenType::BULK_STRING) {
        throw std::runtime_error("Expected third token to be a BULK_STRING type for ECHO command");
    }

    const auto echo_value = std::get<std::string_view>(echo_token.value);
    return "$" + std::to_string(echo_value.size()) + "\r\n" + std::string(echo_value) + "\r\n";
}
} // namespace

ResponseGenerator::ResponseGenerator(const char *input, std::size_t input_size) : tokenizer_(input, input_size) {
    verify_input(input, input_size);

    const auto &tokens = tokenizer_.get_tokens();
    const auto &command_token = tokens[1];

    if (command_token.type != TokenType::BULK_STRING) {
        throw std::runtime_error("Expected second token to be a BULK_STRING type");
    }

    const auto command = std::get<std::string_view>(command_token.value);

    if (command == "PING") {
        response_ = generate_ping_response();
    } else if (command == "ECHO") {
        response_ = generate_echo_response(tokens);
    } else {
        response_ = generate_error_response("Unknown command: " + std::string(command));
    }
}

const std::string &ResponseGenerator::get_response() const {
    return response_;
}

void ResponseGenerator::verify_input(const char *input, std::size_t input_size) {
    tokenizer_ = RESPTokenizer(input, input_size);
    const auto &tokens = tokenizer_.get_tokens();

    if (tokens.empty()) {
        response_ = generate_error_response("Empty message");
    }

    const auto &first_token = tokens[0];

    // For now, the first token is expected to be an array token
    if (first_token.type != TokenType::ARRAY) {
        throw std::runtime_error("Expected first token to be an ARRAY type");
    }

    const auto number_of_array_elements = std::get<int>(first_token.value);

    if (number_of_array_elements < 0) {
        throw std::runtime_error("Invalid number of array elements: " + std::to_string(number_of_array_elements));
    }

    if (number_of_array_elements == 0) {
        response_ = "*0\r\n";
    }

    if (static_cast<int>(tokens.size() - 1) < number_of_array_elements) {
        throw std::runtime_error("Not enough tokens for the number of array elements");
    }
}
