#include "tcp_server.hpp"

int main() {
    TCPServer server{6030};
    server.run();
    return 0;
}