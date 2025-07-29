#pragma once

#include <cstddef>
#include <string>

#include "resp_tokenizer.hpp"

class ResponseGenerator {
  public:
    ResponseGenerator(const char *input, std::size_t input_size);
    const std::string &get_response() const;

  private:
    void verify_input(const char *input, std::size_t input_size);

    std::string response_{""};
    RESPTokenizer tokenizer_;
};
