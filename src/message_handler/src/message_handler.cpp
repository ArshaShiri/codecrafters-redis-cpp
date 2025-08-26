#include "algorithm"

#include "common.hpp"
#include "message_handler.hpp"
#include "redis.hpp"
#include "resp_tokenizer.hpp"

namespace {
using namespace std::chrono_literals;

bool iequals(std::string_view lhs, std::string_view rhs) {
    const auto ichar_equals = [](char a, char b) {
        return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
    };

    return std::ranges::equal(lhs, rhs, ichar_equals);
}
} // namespace

MessageHandler::MessageHandler(const Redis &redis, DataManager<std::string, std::string> &data_manager)
  : tokenizer_{}, redis_{redis}, data_manager_{data_manager} {
}

const std::string &MessageHandler::genera_response(std::string_view input) {
    response_ = "";
    const auto &tokens = tokenizer_.generate_tokens(input);

    // TODO retrun of verification fails...
    verify_input(tokens);

    const auto number_of_array_elements = std::get<int>(tokens[0].value);
    const auto &command_token = tokens[1];

    if (command_token.type != TokenType::BULK_STRING) {
        generate_error_response("Expected second token to be a BULK_STRING type");
    }

    const auto command = std::get<std::string_view>(command_token.value);
    // std::transform(command.begin(), command.end(), command.begin(), [](auto c) { return std::tolower(c); });

    if (iequals(command, "PING")) {
        generate_ping_response();
    } else if (iequals(command, "ECHO")) {
        if (number_of_array_elements != 2) {
            generate_error_response("ECHO command expects exactly one argument");
        }

        generate_echo_response(tokens);
    } else if (iequals(command, "SET")) {
        generate_set_response(tokens);
    } else if (iequals(command, "GET")) {
        generate_get_response(tokens);
    } else if (iequals(command, "CONFIG")) {
        generate_rdb_config_response(tokens);
    } else if (iequals(command, "KEYS")) {
        generate_keys_response(tokens);
    } else if (iequals(command, "INFO")) {
        generate_redis_info_response(tokens);
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
            generate_error_response("Expected fifth token to be a BULK_STRING type for SET command with expiry");
            return;
        }

        const auto expiry_option = std::get<std::string_view>(expiry_token.value);
        if (expiry_option == "px") {
            expiry_duration =
              std::chrono::milliseconds(get_int_from_string_view(std::get<std::string_view>(tokens[5].value)));
        } else {
            generate_error_response("Unknown expiry option: " + std::string(expiry_option));
            return;
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
        generate_error_response("Expected third token to be a BULK_STRING type for GET command");
        return;
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
        generate_error_response("Expected third token to be a BULK_STRING type for GET command");
        return;
    }

    const auto get = std::get<std::string_view>(get_command.value);
    if (get != "GET") {
        generate_error_response("Only GET is supported for CONFIG command");
        return;
    }

    const auto &target_command = tokens[3];
    if (target_command.type != TokenType::BULK_STRING) {
        generate_error_response("Expected forth token to be a BULK_STRING type for GET command");
        return;
    }

    const auto target = std::get<std::string_view>(target_command.value);
    if ((target != "dir") && (target != "dbfilename")) {
        generate_error_response("Only dir and dbfilename are supported");
        return;
    }

    response_ = "*2\r\n$3\r\ndir\r\n";

    if (target == "dir") {
        const auto &dir = redis_.get_rdb_dir();
        response_ += "$" + std::to_string(dir.size()) + "\r\n";
        response_ += dir + "\r\n";
    } else {
        const auto &file_name = redis_.get_rdb_file_name();
        response_ += "$" + std::to_string(file_name.size()) + "\r\n";
        response_ += file_name + "\r\n";
    }
}

void MessageHandler::generate_redis_info_response(const std::vector<Token> &tokens) {
    const auto &replication_command = tokens[2];
    if (replication_command.type != TokenType::BULK_STRING) {
        generate_error_response("Expected third token to be a BULK_STRING type for replication command");
        return;
    }

    const auto replication = std::get<std::string_view>(replication_command.value);
    if (replication != "replication") {
        generate_error_response("Only replication is supported for INFO command");
        return;
    }

    const auto redis_role = redis_.get_role();
    const auto response = "role:" + redis_role;

    response_ = "$" + std::to_string(response.size()) + "\r\n" + response + "\r\n";
}

void MessageHandler::generate_keys_response(const std::vector<Token> &tokens) {
    const auto &pattern_token = tokens[2];
    const auto pattern = std::get<std::string_view>(pattern_token.value);

    if (pattern == "*") {
        generate_error_response("Only \"*\" is supported.");
    }

    // *<array_size>\r\n$<key_size>\r\n<key>\r\n
    const auto keys = data_manager_.get_keys();

    response_ = "*" + std::to_string(keys.size()) + "\r\n";
    for (const auto &key : keys) {
        response_ += "$" + std::to_string(key.size()) + "\r\n" + key + "\r\n";
    }
}
