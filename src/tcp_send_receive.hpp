#ifndef TCP_SEND_RECEIVE
#define TCP_SEND_RECEIVE

#include <span>
#include <cstdint>
#include <cstddef>

std::size_t Send(int sockfd, std::size_t chunk_size, std::span<std::uint8_t> msg);
std::size_t Receive(int sockfd, std::size_t chunk_size, std::span<std::uint8_t> msg);

#endif  // TCP_SEND_RECEIVE
