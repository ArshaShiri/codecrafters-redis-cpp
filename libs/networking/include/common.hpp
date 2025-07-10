#pragma once

#include <cstring>
#include <stdexcept>

class SocketException : public std::runtime_error {
  public:
    SocketException(const std::string &message)
      : std::runtime_error(message + " (errno: " + std::to_string(errno) + " - " + std::strerror(errno) + ")") {
    }
};