#include <arpa/inet.h>
#include <array>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>
#include <string>

static constexpr in_port_t kPort = 8080;
static constexpr std::size_t kBuffSize = 1LLU << 16LLU;

namespace {

void WaitOnce()
{
  std::string str;
  std::getline(std::cin, str);
}


bool DoClient(int sockfd, struct sockaddr_in& server_addr) {
  static std::array<char, kBuffSize> buff = {};
  buff.fill(57);

  ssize_t msg_size = 1 + static_cast<ssize_t>(rand() % kBuffSize);
  std::cout << "send " << msg_size << " bytes\n";

  socklen_t len = sizeof(server_addr);
  sendto(sockfd,
         buff.data(),
         msg_size,
         MSG_CONFIRM,
         reinterpret_cast<const struct sockaddr*>(&server_addr),
         len);

  ssize_t bytes_read =
      recvfrom(sockfd,
               buff.data(),
               kBuffSize,
               MSG_WAITALL,
               reinterpret_cast<struct sockaddr*>(&server_addr),
               &len);
  std::cout << "recv " << bytes_read << " bytes\n";

  if (bytes_read < 0) {
    perror("recv");
    return false;
  }

  assert(bytes_read == msg_size);

  for (ssize_t i = 0; i < bytes_read; ++i) {
    assert((buff[i] ^ static_cast<char>(179)) == 57);
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
  client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
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

  while (true) {
    WaitOnce();

    if (!DoClient(sockfd, server_addr)) {
      std::cout << "bad(\n";
      std::exit(-1);
    } else {
      std::cout << "OK\n";
    }
  }
}
