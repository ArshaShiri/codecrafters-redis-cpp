#include <atomic>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

#include "data_manager.hpp"
#include "message_handler.hpp"
#include "spsc_queue.hpp"
#include "tcp_server.hpp"

class Redis {
  public:
    Redis(int listening_port);
    void run();
    void stop();

  private:
    void run_tcp_server();
    void run_message_handler();

    TCPServer server_;
    std::atomic<bool> running_{true};
    SPSCQueue<RequestMessage> message_queue_;
    SPSCQueue<ResponseMessage> response_queue_;

    DataManager<std::string, std::string> data_manager_;

    std::jthread tcp_server_thread_;
};
