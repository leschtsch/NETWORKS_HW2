#include "options.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

void ParseOprions(int argc, std::span<char*> argv, Options& options) {
  int opt = 0;
  while ((opt = getopt(argc, argv.data(), "hi:p:x:")) != -1) {
    switch (opt) {
      case 'h': {
        std::cout << "options:\n"
                     "\th\tprint this message\n"
                     "\ti\tspecify ip address (for server to listen on, for "
                     "client to  connect to)\n"
                     "\tp\tspecify port\n"
                     "\tx\tset xor_key to xor every byte with (in [0; 255])\n";
        break;
      }

      case 'i': {
        in_addr_t addr = inet_addr(optarg);

        if (addr == INADDR_NONE) {
          std::cerr << "incorrect address\n";
          std::exit(-1);
        }

        options.addr = addr;
        break;
      }

      case 'p': {
        std::uint16_t port = atoi(optarg);
        options.port = htons(port);
        break;
      }

      case 'x': {
        int xor_key = atoi(optarg);
        options.xor_key = static_cast<std::uint8_t>(xor_key);
        break;
      }

      default: {
        std::exit(-1);
        break;
      }
    }
  }
}
