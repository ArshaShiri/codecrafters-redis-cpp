#include "tcp_server.hpp"

int main() {
    TCPServer server{6379};
    server.run();
    return 0;
}
