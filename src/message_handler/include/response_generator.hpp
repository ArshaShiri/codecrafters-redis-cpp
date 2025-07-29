#pragma once

#include <cstddef>
#include <string>

#include "data_manager.hpp"
#include "resp_tokenizer.hpp"

class ResponseGenerator {
  public:
    ResponseGenerator(const char *input, std::size_t input_size, DataManager<std::string, std::string> &data_manager);
    const std::string &get_response() const;

  private:
    void verify_input(const char *input, std::size_t input_size);
    void generate_ping_response();
    void generate_error_response(const std::string &error_message);
    void generate_echo_response(const std::vector<Token> &tokens);
    void generate_set_response(const std::vector<Token> &tokens);
    void generate_get_response(const std::vector<Token> &tokens);

    std::string response_{""};
    RESPTokenizer tokenizer_;
    DataManager<std::string, std::string> &data_manager_;
};
