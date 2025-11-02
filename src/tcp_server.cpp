#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <span>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

#include "config.hpp"
#include "get_buff_size.hpp"
#include "options.hpp"

namespace {
bool ServerOneMsg(int sockfd, int bufsiz, std::uint8_t xor_key) {
  int msg_size = 0;
  ssize_t bytes_read = recv(sockfd, &msg_size, sizeof(msg_size), 0);
  msg_size = ntohl(msg_size);

  static std::vector<std::uint8_t> buff;
  buff.clear();
  buff.resize(msg_size);

  if (bytes_read < 0) {
    perror("recv");
    return false;
  }
  if (bytes_read == 0) {
    std::cout << "client left\n";
    return false;
  }

  int already_read = 0;

  while (already_read < msg_size) {
    int need_read = std::min(msg_size - already_read, bufsiz);
    bytes_read = recv(sockfd, buff.data() + already_read, need_read, 0);
    if (bytes_read < need_read) {
      std::cout << "client left\n";
      return false;
    }

    already_read += static_cast<int>(bytes_read);
  }

  std::cout << "recv " << already_read << " bytes\n";

  for (ssize_t i = 0; i < already_read; ++i) {
    buff[i] ^= xor_key;
  }

  std::cout << "send " << already_read << "bytes\n";

  int already_sent = 0;
  while (already_sent < msg_size) {
    int need_send = std::min(msg_size - already_sent, bufsiz);
    if (send(sockfd, buff.data() + already_sent, need_send, 0) < 0) {
      perror("send");
      std::exit(-1);
    }

    already_sent += need_send;
  }

  return true;
}

void DoServer(int sockfd, std::uint8_t xor_key) {
  int bufsiz = GetBufsize(sockfd);

  /*
  if (send(sockfd, buff.data(), 13, 0) < 0) {
    perror("send");
    std::exit(-1);
  }

  Здесь можно было бы что-то отправить, если бы клиент не ожидал определенный
  ответ.
  */

  while (ServerOneMsg(sockfd, bufsiz, xor_key)) {}

  if (close(sockfd) < 0) {
    perror("close");
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
            << options.port << " with xor_key "
            << static_cast<int>(options.xor_key) << "\n";

  while (int clientfd = accept(lisetnfd, nullptr, nullptr)) {
    std::cout << "new client\n";
    DoServer(clientfd, options.xor_key);
  }
}
