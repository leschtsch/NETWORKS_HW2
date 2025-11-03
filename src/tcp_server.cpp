#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <span>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "config.hpp"
#include "get_chunk_size.hpp"
#include "options.hpp"
#include "tcp_send_receive.hpp"

namespace {

bool ServerOneMsg(int sockfd, std::size_t chunk_size, std::uint8_t xor_key) {
  std::uint32_t msg_size = 0;
  std::span<uint8_t> msg_size_span{reinterpret_cast<uint8_t*>(&msg_size),
                                   sizeof(msg_size)};
  if (Receive(sockfd, chunk_size, msg_size_span) < sizeof(msg_size)) {
    std::cout << "something bad happened to the client while transmitting "
                 "msg_size(\n";
    return false;
  }

  msg_size = ntohl(msg_size);

  static std::vector<std::uint8_t> buff;
  buff.clear();
  buff.resize(msg_size);

  if (Receive(sockfd, chunk_size, {buff.data(), msg_size}) < msg_size) {
    std::cout
        << "something bad happened to the client while transmitting data(\n";
    return false;
  }

  for (ssize_t i = 0; i < msg_size; ++i) {
    buff[i] ^= xor_key;
  }

  std::size_t sent = Send(sockfd, chunk_size, {buff.data(), msg_size});

  if (sent < msg_size) {
    std::cout << "something bad happened to the client while receiving data(\n";
    return false;
  }

  return true;
}

void DoServer(int sockfd, std::uint8_t xor_key) {
  std::size_t chunk_size = GetChunkSize(sockfd);

  /*
  Здесь можно было бы что-то отправить, если бы клиент не ожидал определенный
  ответ.
  */

  while (ServerOneMsg(sockfd, chunk_size, xor_key)) {}

  if (close(sockfd) < 0) {
    std::perror("close");
    std::exit(-1);
  }
}
}  // namespace

int main(int argc, char* argv[]) {
  Options options;
  options.addr = kServerDefaultListenAddr;
  options.port = kDefaultPort;
  options.xor_key = kDefaultXorKey;
  ParseOprions(argc, std::span<char*>(argv, argc), options);

  int lisetnfd = socket(AF_INET, SOCK_STREAM, 0);
  if (lisetnfd < 0) {
    std::perror("socket");
    std::exit(-1);
  }

  int flag = 1;
  int status =
      setsockopt(lisetnfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
  if (status < 0) {
    std::perror("setsockopt");
    std::exit(-1);
  }

  struct sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = options.addr;
  server_addr.sin_port = options.port;

  if (bind(lisetnfd,
           reinterpret_cast<const struct sockaddr*>(&server_addr),
           sizeof(server_addr)) < 0) {
    std::perror("bind");
    std::exit(-1);
  }

  if (listen(lisetnfd, 1) < 0) {
    std::perror("listen");
    std::exit(-1);
  }

  std::cout << "listening on " << inet_ntoa({options.addr}) << ":"
            << ntohs(options.port) << " with xor_key "
            << static_cast<int>(options.xor_key) << "\n";

  while (int clientfd = accept(lisetnfd, nullptr, nullptr)) {
    std::cout << "new client\n";
    DoServer(clientfd, options.xor_key);
  }
}
