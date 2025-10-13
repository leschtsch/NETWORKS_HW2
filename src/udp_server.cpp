#include <arpa/inet.h>
#include <array>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>

static constexpr in_port_t kPort = 8080;
static constexpr std::size_t kBuffSize = 1LLU << 16LLU;

namespace {
void DoServer(int sockfd) {
  std::array<char, kBuffSize> buff = {};

  struct sockaddr_in client_addr;
  socklen_t len = sizeof(client_addr);

  ssize_t bytes_read =
      recvfrom(sockfd,
               buff.data(),
               kBuffSize,
               0,
               reinterpret_cast<struct sockaddr*>(&client_addr),
               &len);
  std::cout << "recv " << bytes_read << " bytes\n";

  if (bytes_read < 0) {
    perror("recv");
    return;
  }

  for (ssize_t i = 0; i < bytes_read; ++i) {
    buff[i] ^= static_cast<char>(179);
  }

  std::cout << "send " << bytes_read << "bytes\n";
  if (sendto(sockfd,
             buff.data(),
             bytes_read,
             0,
             reinterpret_cast<const struct sockaddr*>(&client_addr),
             len) < 0) {
    perror("send");
    std::exit(-1);
  }
}
}  // namespace

int main() {
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  if (sockfd < 0) {
    std::perror("socket");
    std::exit(-1);
  }

  struct sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(kPort);

  if (bind(sockfd,
           reinterpret_cast<const struct sockaddr*>(&server_addr),
           sizeof(server_addr)) < 0) {
    std::perror("bind");
    std::exit(-1);
  }

  while (true) {
    DoServer(sockfd);
  }
}
