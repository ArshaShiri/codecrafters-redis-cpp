#include <string>

#include "data_manager.hpp"
#include "tcp_server.hpp"

class Redis {
  public:
    Redis(int listening_port);
    void run();

  private:
    DataManager<std::string, std::string> data_manager_;
    TCPServer server_;
};
