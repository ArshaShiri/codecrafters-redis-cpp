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

enum class Role { master, slave };

struct RedisConfig {
    RDBConfig rdb_config{};
    int listening_port{6379};
    Role role{Role::master};
};

class Redis {
  public:
    Redis(RedisConfig config);
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

    const std::string &get_replication_id() const {
        return replication_id_;
    }

    std::size_t get_replication_offset() const {
        return replication_offset_;
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
    Role role_;
    std::string replication_id_{"8371b4fb1155b71f4a04d3e1bc3e18c4a990aeeb"};
    std::size_t replication_offset_{0};

    std::jthread tcp_server_thread_;
};
