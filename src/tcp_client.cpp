#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <vector>

#include "config.hpp"
#include "get_chunk_size.hpp"
#include "options.hpp"

namespace {

void Send(int sockfd, const std::string& msg, int chunk_size)
{
  int msg_size = static_cast<int>(msg.size());

  std::cout << "send " << msg.size() << " bytes\n";

  int already_sent = 0;

  while (already_sent < msg_size) {
    int need_send = std::min(msg_size - already_sent, chunk_size);

    if (send(sockfd, msg.data() + already_sent, need_send, 0) < 0) {
      perror("send");
      std::exit(-1);
    }

    already_sent += need_send;
  }
}

void Receive(int sockfd, const std::string& msg, int chunk_size, std::uint8_t xor_key)
{
  int msg_size = static_cast<int>(msg.size());

  static std::vector<std::uint8_t> buff;
  buff.clear();
  buff.resize(msg.size());

  int already_read = 0;

  while (already_read < msg_size) {
    int need_read = std::min(msg_size - already_read, chunk_size);

    ssize_t bytes_read = recv(sockfd, buff.data() + already_read, need_read, 0);

    if (bytes_read == 0) {
      perror("recv");
      std::exit(-1);
    }

    already_read += static_cast<int>(bytes_read);
  }

  std::cout << "recv " << already_read << " bytes\n";

  for (ssize_t i = 0; i < already_read; ++i) {
    if ((buff[i] ^ xor_key) != msg[i]) {
      std::cout << "incorrect encryption\n";
      break;
    }
  }

}

bool DoClient(int sockfd, int chunk_size, std::uint8_t xor_key) {
  std::string msg;
  if (!(std::cin >> msg)) {
    return false;
  }

  int msg_size = htonl(msg.size());

  if (send(sockfd, &msg_size, sizeof(msg_size), 0) < 0) {
    perror("send");
    std::exit(-1);
  }

  Send(sockfd, msg, chunk_size);
  Receive(sockfd, msg, chunk_size, xor_key);

  return true;
}

int Connect(const Options& options) {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    std::perror("socket");
    std::exit(-1);
  }

  struct sockaddr_in client_addr{};
  client_addr.sin_family = AF_INET;
  client_addr.sin_addr.s_addr = htons(INADDR_ANY);
  client_addr.sin_port = 0;

  struct timeval timeval;
  timeval.tv_sec = 5;
  timeval.tv_usec = 0;
  setsockopt(sockfd,
             SOL_SOCKET,
             SO_RCVTIMEO,
             reinterpret_cast<const char*>(&timeval),
             sizeof timeval);

  struct sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = options.addr;
  server_addr.sin_port = options.port;

  if (bind(sockfd,
           reinterpret_cast<const struct sockaddr*>(&client_addr),
           sizeof(client_addr)) < 0) {
    std::perror("bind");
    std::exit(-1);
  }

  if (connect(sockfd,
              reinterpret_cast<struct sockaddr*>(&server_addr),
              sizeof(server_addr)) < 0) {
    std::perror("connect");
    std::exit(-1);
  }

  return sockfd;
}
}  // namespace

int main(int argc, char* argv[]) {
  Options options;
  options.addr = kClientDefaultConnectAddr;
  options.port = kDefaultPort;
  options.xor_key = kDefaultXorKey;
  ParseOprions(argc, std::span<char*>(argv, argc), options);

  int sockfd = Connect(options);

  int chunk_size = GetChunkSize(sockfd);

  while (DoClient(sockfd, chunk_size, options.xor_key)) {}
}
