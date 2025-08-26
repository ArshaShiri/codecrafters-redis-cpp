#include <filesystem>
#include <getopt.h>
#include <iostream>
#include <string>

#include "redis.hpp"
#include "tcp_server.hpp"

void printUsage(const char *progName) {
    std::cout << "Usage: " << progName << " [OPTIONS]\n";
    std::cout << "Options:\n";
    std::cout << "  --dir PATH          Directory path (default: /tmp)\n";
    std::cout << "  --dbfilename FILE   Database filename (default: dump.rdb)\n";
    std::cout << "  --port PORT_NUMBER  Port on which Redis listens (default: 6379)\n";
    std::cout << "  --replicaof         start redis as salve (default: master)\n";
    std::cout << "  --help, -h          Show this help message\n";
}

RedisConfig parseCommandLine(int argc, char *argv[]) {
    RedisConfig config;

    static struct option long_options[] = {{"dir", required_argument, nullptr, 'd'},
                                           {"dbfilename", required_argument, nullptr, 'f'},
                                           {"port", required_argument, nullptr, 'p'},
                                           {"replicaof", required_argument, nullptr, 'r'},
                                           {"help", required_argument, nullptr, 'h'},
                                           {0, 0, 0, 0}};

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "d:f:p:r:h", long_options, &option_index)) != -1) {
        switch (c) {
        case 'd':
            config.rdb_config.dir = optarg;
            break;
        case 'f':
            config.rdb_config.dbfilename = optarg;
            break;
        case 'p':
            config.listening_port = std::stoi(optarg);
            break;
        case 'r':
            config.role = Role::slave;
            break;
        case 'h':
            printUsage(argv[0]);
            exit(0);
            break;
        case '?':
            // getopt_long already printed an error message
            printUsage(argv[0]);
            exit(1);
            break;
        default:
            exit(1);
        }
    }

    return config;
}

int main(int argc, char *argv[]) {
    Redis redis(parseCommandLine(argc, argv));
    redis.run();

    return 0;
}
