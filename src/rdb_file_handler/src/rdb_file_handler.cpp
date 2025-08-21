#include <filesystem>
#include <fstream>
#include <iostream>

#include "rdb_file_handler.hpp"

namespace {

std::size_t read_length(const std::vector<unsigned char> buffer, std::size_t &buffer_index) {
    uint8_t first_byte = buffer[buffer_index];
    std::size_t length{0};
    // std::printf("read_length first_byte: %02x\n", first_byte);

    // If the first two bits are 0b00, the length is the remaining 6 bits of the byte.
    if ((first_byte & 0xC0) == 0) {
        length = first_byte & ~0xC0;
    }
    // If the first two bits are 0b01, the length is the next 14 bits (remaining 6 bits in the first byte, combined with
    // the next byte), in big-endian (read left-to-right)
    else if ((first_byte & 0xC0) == 0x40) {
        length = (first_byte & ~0xC0) << 8 | buffer[buffer_index++];
    }
    // If the first two bits are 0b10, ignore the remaining 6 bits of the first byte. The length is the next 4 bytes, in
    // big-endian (read left-to-right).
    else if ((first_byte & 0xC0) == 0x80) {
        length = buffer[buffer_index++] << 24;
        length |= buffer[buffer_index++] << 16;
        length |= buffer[buffer_index++] << 8;
        length |= buffer[buffer_index++];
    } else {
        std::cout << "Cannot determine length." << std::endl;
    }

    return length;
}

} // namespace


RDBFileHandler::RDBFileHandler(const RDBConfig &rdb_config, DataManager<std::string, std::string> &data_manager)
  : rdb_config_{rdb_config}, data_manager_{data_manager} {
    if (rdb_config.dbfilename.empty() || rdb_config.dir.empty()) {
        return;
    }

    std::filesystem::path rdb_file_path = rdb_config.dir;
    rdb_file_path.append(rdb_config.dbfilename);

    if (!std::filesystem::exists(rdb_file_path)) {
        return;
    }

    std::ifstream file{rdb_file_path, std::ios::binary};
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>{});
    parse_file(buffer);
}

void RDBFileHandler::parse_file(const std::vector<unsigned char> buffer) {
    const std::string magic_string = "REDIS";
    const std::string version = "0011";

    // Skip the header of the file
    std::size_t index{magic_string.size() + version.size() - 1};

    // Skip the bytes until the database section
    unsigned char start_of_database = 0xFE;
    while ((buffer[index] != start_of_database) && (index < buffer.size())) {
        ++index;
    }

    // Skip the database start and the byte after
    index += 2;

    std::size_t hash_table_size;
    for (; index < buffer.size(); ++index) {
        const auto byte = buffer[index];

        if (byte == 0xFB) {
            index++;
            hash_table_size = read_length(buffer, index);
            std::cout << "hash_table_size: " << hash_table_size << std::endl;
            index++;
            read_length(buffer, index);
        }

        // Parse key value
        //
        // 00                       The 1-byte flag that specifies the value’s type and encoding.
        //                            Here, the flag is 0, which means "string." */
        // 06 66 6F 6F 62 61 72     The name of the key (string encoded). Here, it's "foobar".
        // 06 62 61 7A 71 75 78     The value (string encoded). Here, it's "bazqux".
        if (byte == 0x00) {
            index++;
            const auto key_length = read_length(buffer, index);
            index++;
            std::string key(buffer.begin() + index, buffer.begin() + index + key_length);
            index += key_length - 1;

            index++;
            const auto value_length = read_length(buffer, index);
            index++;
            std::string value(buffer.begin() + index, buffer.begin() + index + value_length);
            index += value_length - 1;
            data_manager_.set(std::move(key), std::move(value));
        }
    }

    // std::size_t count{0};
    // for (const auto byte : buffer) {
    //     std::printf("%02x ", byte);
    //     if (++count % 16 == 0)
    //         std::cout << '\n';
    // }
}
