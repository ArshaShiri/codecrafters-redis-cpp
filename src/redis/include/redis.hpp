#include <atomic>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

#include "data_manager.hpp"
#include "message_handler.hpp"
#include "rdb_file_handler.hpp"
#include "spsc_queue.hpp"
#include "tcp_server.hpp"

class Redis {
  private:
    enum class Role { master, slave };

  public:
    Redis(int listening_port, RDBConfig rdb_config = {});
    void run();
    void stop();

    const std::string &get_rdb_dir() const {
        return rdb_handler_.get_rdb_dir();
    }

    const std::string &get_rdb_file_name() const {
        return rdb_handler_.get_rdb_file_name();
    }

    std::string get_role() const {
        return role_ == Role::master ? "master" : "slave";
    }

  private:
    void run_tcp_server();
    void run_message_handler();

    TCPServer server_;
    std::atomic<bool> running_{true};
    SPSCQueue<RequestMessage> message_queue_;
    SPSCQueue<ResponseMessage> response_queue_;

    DataManager<std::string, std::string> data_manager_;
    RDBFileHandler rdb_handler_;

    std::jthread tcp_server_thread_;
    Role role_{Role::master};
};
