#include <atomic>
#include <string>

#include "data_manager.hpp"
#include "tcp_server.hpp"

class Redis {
  public:
    Redis(int listening_port);
    void run();
    void stop() {
        running_ = false;
    }

  private:
    TCPServer server_;
    std::atomic<bool> running_{true};
    DataManager<std::string, std::string> data_manager_;
};
