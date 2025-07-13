#pragma once

#include <cstring>
#include <stdexcept>

#define INVALID_SOCKET -1

class SocketException : public std::runtime_error {
  public:
    SocketException(const std::string &message)
      : std::runtime_error(message + " (errno: " + std::to_string(errno) + " - " + std::strerror(errno) + ")") {
    }
};