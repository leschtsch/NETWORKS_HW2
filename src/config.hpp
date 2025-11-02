#ifndef CONFIG_H
#define CONFIG_H

#include <arpa/inet.h>

static const in_port_t kDefaultPort = htons(8080);

static const in_addr_t kServerDefaultListenAddr = htonl(INADDR_ANY);
static const in_addr_t kClientDefaultConnectAddr = inet_addr("127.0.0.1");

static constexpr char kDefaultXorKey = 0;

static constexpr int kMaxChunkSize = 1LLU << 16LLU;

#endif  // CONFIG_H
