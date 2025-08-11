#include "message_handler.hpp"
#include "common.hpp"
#include "resp_tokenizer.hpp"

namespace {
using namespace std::chrono_literals;
}

MessageHandler::MessageHandler(const RDBConfig &rdb_config, DataManager<std::string, std::string> &data_manager)
  : tokenizer_{}, rdb_config_{rdb_config}, data_manager_{data_manager} {
}

const std::string &MessageHandler::genera_response(std::string_view input) {
    response_ = "";
    const auto &tokens = tokenizer_.generate_tokens(input);

    // TODO retrun of verification fails...
    verify_input(tokens);

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
            generate_error_response("SET command expects exactly two arguments");
        }

        generate_set_response(tokens);
    } else if (command == "GET") {
        generate_get_response(tokens);
    } else if (command == "CONFIG") {
        generate_rdb_config_response(tokens);
    } else {
        generate_error_response("Unknown command: " + std::string(command));
    }

    return response_;
}

void MessageHandler::verify_input(const std::vector<Token> &tokens) {
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

void MessageHandler::generate_ping_response() {
    response_ = "+PONG\r\n";
}

void MessageHandler::generate_error_response(const std::string &error_message) {
    response_ = "-ERR " + error_message + "\r\n";
}

void MessageHandler::generate_echo_response(const std::vector<Token> &tokens) {
    const auto &echo_token = tokens[2];
    if (echo_token.type != TokenType::BULK_STRING) {
        throw std::runtime_error("Expected third token to be a BULK_STRING type for ECHO command");
    }

    const auto echo_value = std::get<std::string_view>(echo_token.value);
    response_ = "$" + std::to_string(echo_value.size()) + "\r\n" + std::string(echo_value) + "\r\n";
}

void MessageHandler::generate_set_response(const std::vector<Token> &tokens) {
    const auto &key_token = tokens[2];
    const auto &value_token = tokens[3];
    if (key_token.type != TokenType::BULK_STRING || value_token.type != TokenType::BULK_STRING) {
        throw std::runtime_error(
          "Expected third and fourth tokens to be "
          "BULK_STRING types for SET command");
    }

    const auto key = std::get<std::string_view>(key_token.value);
    const auto value = std::get<std::string_view>(value_token.value);

    std::chrono::milliseconds expiry_duration{std::numeric_limits<int>::max()};
    const auto key_value_has_expiry = tokens.size() == 6;

    if (key_value_has_expiry) {
        const auto &expiry_token = tokens[4];
        if (expiry_token.type != TokenType::BULK_STRING) {
            throw std::runtime_error("Expected fifth token to be a BULK_STRING type for SET command with expiry");
        }

        const auto expiry_option = std::get<std::string_view>(expiry_token.value);
        if (expiry_option == "px") {
            expiry_duration =
              std::chrono::milliseconds(get_int_from_string_view(std::get<std::string_view>(tokens[5].value)));
        } else {
            throw std::runtime_error("Unknown expiry option: " + std::string(expiry_option));
        }
    }


    if (!data_manager_.set(std::string(key), std::string(value), expiry_duration)) {
        generate_error_response("Key already exists: " + std::string(key));
        return;
    }

    response_ = "+OK\r\n";
}

void MessageHandler::generate_get_response(const std::vector<Token> &tokens) {
    const auto &key_token = tokens[2];
    if (key_token.type != TokenType::BULK_STRING) {
        throw std::runtime_error("Expected third token to be a BULK_STRING type for GET command");
    }

    const auto key = std::get<std::string_view>(key_token.value);
    const auto [exists, value] = data_manager_.get(std::string(key));

    if (!exists) {
        response_ = "$-1\r\n"; // Null bulk string
    } else {
        response_ = "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
    }
}

void MessageHandler::generate_rdb_config_response(const std::vector<Token> &tokens) {
    const auto &get_command = tokens[2];
    if (get_command.type != TokenType::BULK_STRING) {
        throw std::runtime_error("Expected third token to be a BULK_STRING type for GET command");
    }

    const auto get = std::get<std::string_view>(get_command.value);
    if (get != "GET") {
        generate_error_response("Only GET is supported for CONFIG command");
    }

    const auto &target_command = tokens[3];
    if (target_command.type != TokenType::BULK_STRING) {
        throw std::runtime_error("Expected forth token to be a BULK_STRING type for GET command");
    }

    const auto target = std::get<std::string_view>(target_command.value);
    if ((target != "dir") || (target != "dbfilename")) {
        generate_error_response("Only dir and dbfilename are supported");
    }

    response_ = "*2\r\n$3\r\ndir\r\n";

    if (target == "dir") {
        response_ += "$" + std::to_string(rdb_config_.dir.size()) + "\r\n";
        response_ += rdb_config_.dir + "\r\n";
    } else {
        response_ += "$" + std::to_string(rdb_config_.dbfilename.size()) + "\r\n";
        response_ += rdb_config_.dbfilename + "\r\n";
    }
}
