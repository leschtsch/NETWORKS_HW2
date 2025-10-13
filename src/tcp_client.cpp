#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <vector>

static constexpr in_port_t kPort = 8080;
static constexpr std::size_t kBuffSize = 1LLU << 16LLU;

namespace {

int GetBufsize(int sockfd) {
  int res = 512;
  int cur_buf = 0;
  unsigned int mlen = sizeof(cur_buf);
  if (getsockopt(sockfd,
                 SOL_SOCKET,
                 SO_RCVBUF,
                 reinterpret_cast<void*>(&cur_buf),
                 &mlen) < 0) {
    perror("getsockopt");
    std::exit(-1);
  }
  res = std::min(res, cur_buf);
  if (getsockopt(sockfd,
                 SOL_SOCKET,
                 SO_SNDBUF,
                 reinterpret_cast<void*>(&cur_buf),
                 &mlen) < 0) {
    perror("getsockopt");
    std::exit(-1);
  }

  res = std::min(res, cur_buf);

  return res;
}

bool DoClient(int sockfd) {
  int bufsiz = GetBufsize(sockfd);

  std::string msg;
  if (!(std::cin >> msg)) {
    return false;
  }

  static std::vector<char> buff;
  buff.clear();
  buff.resize(msg.size());

  int msg_size = htonl(msg.size());

  if (send(sockfd, &msg_size, sizeof(msg_size), 0) < 0) {
    perror("send");
    std::exit(-1);
  }
  msg_size = ntohl(msg_size);

  std::cout << "send " << msg.size() << " bytes\n";
  int already_sent = 0;
  while (already_sent < msg_size) {
    int need_send = std::min(msg_size - already_sent, bufsiz);
    if (send(sockfd, msg.data() + already_sent, need_send, 0) < 0) {
      perror("send");
      std::exit(-1);
    }

    already_sent += need_send;
  }

  int already_read = 0;

  while (already_read < msg_size) {
    int need_read = std::min(msg_size - already_read, bufsiz);
    ssize_t bytes_read = recv(sockfd, buff.data() + already_read, need_read, 0);
    if (bytes_read < need_read) {
      perror("recv");
      std::exit(-1);
    }

    already_read += static_cast<int>(bytes_read);
  }
  std::cout << "recv " << already_read << " bytes\n";

  for (ssize_t i = 0; i < already_read; ++i) {
    assert((buff[i] ^ static_cast<char>(179)) == msg[i]);
  }

  return true;
}

int Connect() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

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

  if (connect(sockfd,
              reinterpret_cast<struct sockaddr*>(&server_addr),
              sizeof(server_addr)) < 0) {
    std::perror("connect");
    std::exit(-1);
  }

  return sockfd;
}
}  // namespace

int main() {
  int sockfd = Connect();

  while (DoClient(sockfd)) {}
}
