#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <arpa/inet.h>
#include <cstdint>
#include <span>

struct Options {
  in_addr_t addr{0};
  in_port_t port{0};
  std::uint8_t xor_key{0};
};

void ParseOprions(int argc, std::span<char*> argv, Options& options);

#endif  // OPTIONS_HPP
