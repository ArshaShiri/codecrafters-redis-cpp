#pragma once

#include <string>
#include <vector>

#include "data_manager.hpp"

struct RDBConfig {
    std::string dir = "";
    std::string dbfilename = "";
};

class RDBFileHandler {
  public:
    RDBFileHandler(const RDBConfig &rdb_config, DataManager<std::string, std::string> &data_manager);

    const std::string &get_rdb_dir() const {
        return rdb_config_.dir;
    }

    const std::string &get_rdb_file_name() const {
        return rdb_config_.dbfilename;
    }

  private:
    void parse_file(const std::vector<unsigned char> buffer);

    const RDBConfig &rdb_config_;
    DataManager<std::string, std::string> &data_manager_;
};
