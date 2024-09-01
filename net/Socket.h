#pragma once

#include "noncopyable.h"

namespace mymuduo {
class InetAddress;
class Socket : noncopyable {
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {};
  ~Socket();

  int sockfd() const { return sockfd_; }

  void BindAddress(const InetAddress& localaddr);
  void Listen();
  int Accept(InetAddress* peeraddr);

  void ShutdownWrite();
  void SetTcpNodelay(bool on);
  void SetReuseAddr(bool on);
  void SetReusePort(bool on);
  void SetKeepAlive(bool on);

 private:
  const int sockfd_;
};
}  // namespace mymuduo