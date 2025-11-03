#include "get_chunk_size.hpp"

#include<sys/socket.h>
#include <algorithm>
#include <cstdlib>
#include <cerrno>
#include <cstdio>

#include "config.hpp"

std::size_t GetChunkSize(int sockfd) {
  int res = kMaxChunkSize;

  int cur_buf = 0;
  unsigned int mlen = sizeof(cur_buf);

  if (getsockopt(sockfd,
                 SOL_SOCKET,
                 SO_RCVBUF,
                 reinterpret_cast<void*>(&cur_buf),
                 &mlen) < 0) {
    std::perror("getsockopt");
    std::exit(-1);
  }

  res = std::min(res, cur_buf);

  if (getsockopt(sockfd,
                 SOL_SOCKET,
                 SO_SNDBUF,
                 reinterpret_cast<void*>(&cur_buf),
                 &mlen) < 0) {
    std::perror("getsockopt");
    std::exit(-1);
  }

  res = std::min(res, cur_buf);

  return res;
}

