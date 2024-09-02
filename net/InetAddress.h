#pragma once

#include <netinet/in.h>

#include <string>

namespace mymuduo {

class InetAddress {
 public:
  explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
  explicit InetAddress(const sockaddr_in& sockaddr) : sockaddr_(sockaddr) {};

  void set_sockaddr(const sockaddr_in& addr) { sockaddr_ = addr; }

  std::string ToIp() const;
  std::string ToIpPort() const;
  uint16_t ToPort() const;

  const sockaddr_in* sockadrr() const { return &sockaddr_; }

 private:
  sockaddr_in sockaddr_;
};

}  // namespace mymuduo
