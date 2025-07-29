#include "response_generator.hpp"
#include "resp_tokenizer.hpp"

ResponseGenerator::ResponseGenerator(const char *input,
                                     std::size_t input_size,
                                     DataManager<std::string, std::string> &data_manager)
  : tokenizer_(input, input_size), data_manager_(data_manager) {
    verify_input(input, input_size);

    const auto &tokens = tokenizer_.get_tokens();
    const auto number_of_array_elements = std::get<int>(tokens[0].value);
    const auto &command_token = tokens[1];

    if (command_token.type != TokenType::BULK_STRING) {
        throw std::runtime_error("Expected second token to be a BULK_STRING type");
    }

    const auto command = std::get<std::string_view>(command_token.value);

    if (command == "PING") {
        generate_ping_response();
    } else if (command == "ECHO") {
        if (number_of_array_elements != 2) {
            generate_error_response("ECHO command expects exactly one argument");
        }

        generate_echo_response(tokens);
    } else if (command == "SET") {
        if (number_of_array_elements != 3) {
            generate_error_response("SET command expects exactly three arguments");
        }

        generate_set_response(tokens);
    } else if (command == "GET") {
        generate_get_response(tokens);
    } else {
        generate_error_response("Unknown command: " + std::string(command));
    }
}

const std::string &ResponseGenerator::get_response() const {
    return response_;
}

void ResponseGenerator::verify_input(const char *input, std::size_t input_size) {
    tokenizer_ = RESPTokenizer(input, input_size);
    const auto &tokens = tokenizer_.get_tokens();

    if (tokens.empty()) {
        generate_error_response("Empty message");
        return;
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

void ResponseGenerator::generate_ping_response() {
    response_ = "+PONG\r\n";
}

void ResponseGenerator::generate_error_response(const std::string &error_message) {
    response_ = "-ERR " + error_message + "\r\n";
}

void ResponseGenerator::generate_echo_response(const std::vector<Token> &tokens) {
    const auto &echo_token = tokens[2];
    if (echo_token.type != TokenType::BULK_STRING) {
        throw std::runtime_error("Expected third token to be a BULK_STRING type for ECHO command");
    }

    const auto echo_value = std::get<std::string_view>(echo_token.value);
    response_ = "$" + std::to_string(echo_value.size()) + "\r\n" + std::string(echo_value) + "\r\n";
}

void ResponseGenerator::generate_set_response(const std::vector<Token> &tokens) {
    const auto &key_token = tokens[2];
    const auto &value_token = tokens[3];
    if (key_token.type != TokenType::BULK_STRING || value_token.type != TokenType::BULK_STRING) {
        throw std::runtime_error(
          "Expected third and fourth tokens to be "
          "BULK_STRING types for SET command");
    }

    const auto key = std::get<std::string_view>(key_token.value);
    const auto value = std::get<std::string_view>(value_token.value);
    if (!data_manager_.set(std::string(key), std::string(value))) {
        generate_error_response("Key already exists: " + std::string(key));
        return;
    }

    response_ = "+OK\r\n";
}

void ResponseGenerator::generate_get_response(const std::vector<Token> &tokens) {
    const auto &key_token = tokens[2];
    if (key_token.type != TokenType::BULK_STRING) {
        throw std::runtime_error("Expected third token to be a BULK_STRING type for GET command");
    }

    const auto key = std::get<std::string_view>(key_token.value);
    const auto [exists, value] = data_manager_.get(std::string(key));
    std::cout << "GET response for key: " << key << " exists: " << exists << ", value: " << value << std::endl;
    if (!exists) {
        response_ = "$-1\r\n"; // Null bulk string
    } else {
        response_ = "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
    }

    std::cout << "Generated GET response: " << response_ << std::endl;
}
