#include <arpa/inet.h>
#include <array>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

static constexpr in_port_t kPort = 8080;
static constexpr std::size_t kBuffSize = 1LLU << 16LLU;

namespace {
void DoServer(int sockfd) {
  std::array<char, kBuffSize> buff = {};

  while (true) {
    ssize_t bytes_read = recv(sockfd, buff.data(), kBuffSize, 0);

    if (bytes_read < 0) {
      perror("recv");
      return;
    }
    if (bytes_read == 0) {
      std::cout << "client left\n";
      break;
    }

    std::cout << "recv " << bytes_read << " bytes\n";

    for (ssize_t i = 0; i < bytes_read; ++i) {
      buff[i] ^= static_cast<char>(179);
    }

    std::cout << "send " << bytes_read << "bytes\n";
    if (send(sockfd, buff.data(), bytes_read, 0) < 0) {
      perror("send");
      std::exit(-1);
    }
  }

  if (close(sockfd) < 0) {
    perror("close");
    std::exit(-1);
  }
}
}  // namespace

int main() {
  int lisetnfd = socket(AF_INET, SOCK_STREAM, 0);

  if (lisetnfd < 0) {
    std::perror("socket");
    std::exit(-1);
  }

  struct sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(kPort);

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

  while (int clientfd = accept(lisetnfd, nullptr, nullptr)) {
    std::cout << "new client\n";
    DoServer(clientfd);
  }
}
