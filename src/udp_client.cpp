#include <arpa/inet.h>
#include <array>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/socket.h>

static constexpr in_port_t kPort = 8080;
static constexpr std::size_t kBuffSize = 1LLU << 16LLU;

namespace {

bool DoClient(int sockfd, struct sockaddr_in& server_addr) {
  std::string msg;
  if (!(std::cin >> msg)) {
    return false;
  }

  if (msg.size() > kBuffSize) {
    msg.resize(kBuffSize);
  }

  static std::array<char, kBuffSize> buff = {};

  std::cout << "send " << msg.size() << " bytes\n";

  socklen_t len = sizeof(server_addr);
  if (sendto(sockfd,
             msg.data(),
             msg.size(),
             0,
             reinterpret_cast<const struct sockaddr*>(&server_addr),
             len) < 0) {
    perror("send");
    std::exit(-1);
  }

  ssize_t bytes_read =
      recvfrom(sockfd,
               buff.data(),
               kBuffSize,
               0,
               reinterpret_cast<struct sockaddr*>(&server_addr),
               &len);
  std::cout << "recv " << bytes_read << " bytes\n";

  if (bytes_read < 0) {
    perror("recv");
    return false;
  }

  assert(static_cast<std::size_t>(bytes_read) == msg.size());

  for (ssize_t i = 0; i < bytes_read; ++i) {
    assert((buff[i] ^ static_cast<char>(179)) == msg[i]);
  }

  return true;
}
}  // namespace

int main() {
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  if (sockfd < 0) {
    std::perror("socket");
    std::exit(-1);
  }

  struct sockaddr_in client_addr{};
  client_addr.sin_family = AF_INET;
  client_addr.sin_addr.s_addr = INADDR_ANY;
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
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(kPort);

  if (bind(sockfd,
           reinterpret_cast<const struct sockaddr*>(&client_addr),
           sizeof(client_addr)) < 0) {
    std::perror("bind");
    std::exit(-1);
  }

  while (DoClient(sockfd, server_addr)) {}
}
