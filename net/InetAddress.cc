#include "InetAddress.h"

#include <arpa/inet.h>

#include <cstring>

namespace mymuduo {
InetAddress::InetAddress(uint16_t port, std::string ip) {
  std::memset(&sockaddr_, 0, sizeof(sockaddr_));
  sockaddr_.sin_family = AF_INET;
  sockaddr_.sin_port = htons(port);
  sockaddr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

std::string InetAddress::ToIp() const {
  char buffer[64] = "";
  ::inet_ntop(AF_INET, &sockaddr_.sin_addr, buffer, sizeof(buffer));
  return buffer;
}
std::string InetAddress::ToIpPort() const {
  char buffer[64] = "";
  ::inet_ntop(AF_INET, &sockaddr_.sin_addr, buffer, sizeof(buffer));
  return std::string(buffer) + ":" +
         std::to_string(::ntohs(sockaddr_.sin_port));
}
uint16_t InetAddress::ToPort() const { return ::ntohs(sockaddr_.sin_port); }
}  // namespace mymuduo