#include <iostream>

#include "redis.hpp"
#include "tcp_server.hpp"

int main() {
    Redis redis{6379};
    redis.run();

    return 0;
}
