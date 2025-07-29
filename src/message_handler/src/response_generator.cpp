#include "response_generator.hpp"
#include "resp_tokenizer.hpp"

// TODO Support sending errors.
namespace {
std::string generate_ping_response() {
    return "+PONG\r\n";
}

std::string generate_echo_response(const std::vector<Token> &tokens, int number_of_array_elements) {
    if (number_of_array_elements != 2) {
        throw std::runtime_error("ECHO command expects exactly one argument");
    }

    const auto &echo_token = tokens[2];
    if (echo_token.type != TokenType::BULK_STRING) {
        throw std::runtime_error("Expected third token to be a BULK_STRING type for ECHO command");
    }

    const auto echo_value = std::get<std::string_view>(echo_token.value);
    return "$" + std::to_string(echo_value.size()) + "\r\n" + std::string(echo_value) + "\r\n";
}
} // namespace

std::string generate_response(const char *input, std::size_t input_size) {
    auto tokenizer = RESPTokenizer(input, input_size);
    const auto &tokens = tokenizer.tokenize();

    if (tokens.empty()) {
        return "";
    }

    const auto &first_token = tokens[0];

    // For now, the first token is expected to be an array token
    if (first_token.type != TokenType::ARRAY) {
        throw std::runtime_error("Expected first token to be an ARRAY type");
    }

    const auto number_of_array_elements = std::get<int>(first_token.value);

    if (number_of_array_elements < 0) {
        throw std::runtime_error("Invalid number of array elements");
    }

    if (number_of_array_elements == 0) {
        return "*0\r\n";
    }

    if (static_cast<int>(tokens.size() - 1) < number_of_array_elements) {
        throw std::runtime_error("Not enough tokens for the number of array elements");
    }

    const auto &command_token = tokens[1];

    if (command_token.type != TokenType::BULK_STRING) {
        throw std::runtime_error("Expected second token to be a BULK_STRING type");
    }

    const auto command = std::get<std::string_view>(command_token.value);
    if (command == "PING") {
        return generate_ping_response();
    } else if (command == "ECHO") {
        return generate_echo_response(tokens, number_of_array_elements);
    } else {
        throw std::runtime_error("Unknown command: " + std::string(command));
    }
}
