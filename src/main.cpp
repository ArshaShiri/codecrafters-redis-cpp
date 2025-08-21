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
    std::cout << "  --help, -h          Show this help message\n";
}

RDBConfig parseCommandLine(int argc, char *argv[]) {
    RDBConfig config;

    static struct option long_options[] = {{"dir", required_argument, nullptr, 'd'},
                                           {"dbfilename", required_argument, nullptr, 'f'},
                                           {"help", no_argument, nullptr, 'h'},
                                           {0, 0, 0, 0}};

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "d:f:h", long_options, &option_index)) != -1) {
        switch (c) {
        case 'd':
            config.dir = optarg;
            break;
        case 'f':
            config.dbfilename = optarg;
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
    const auto config = parseCommandLine(argc, argv);

    Redis redis(6379, config);
    redis.run();

    return 0;
}
