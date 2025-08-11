#pragma once

#include <cstddef>
#include <string>

#include "data_manager.hpp"
#include "resp_tokenizer.hpp"

struct RDBConfig {
    std::string dir = "";
    std::string dbfilename = "";
};

struct RequestMessage {
    int client_id;
    std::string message;
};

struct ResponseMessage {
    int client_id;
    std::string message;
};

class MessageHandler {
  public:
    MessageHandler(const RDBConfig &rdb_config, DataManager<std::string, std::string> &data_manager);
    const std::string &genera_response(std::string_view input);

  private:
    void verify_input(const std::vector<Token> &tokens);
    void generate_ping_response();
    void generate_error_response(const std::string &error_message);
    void generate_echo_response(const std::vector<Token> &tokens);
    void generate_set_response(const std::vector<Token> &tokens);
    void generate_get_response(const std::vector<Token> &tokens);
    void generate_rdb_config_response(const std::vector<Token> &tokens);

    std::string response_{""};
    RESPTokenizer tokenizer_;
    const RDBConfig &rdb_config_;
    DataManager<std::string, std::string> &data_manager_;
};
