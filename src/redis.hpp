#include "tcp_server.hpp"

class Redis {
  public:
    Redis(int listening_port);
    void run();

  private:
    TCPServer server_;
};
