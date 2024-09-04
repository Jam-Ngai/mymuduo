#include "Socket.h"

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>

#include "InetAddress.h"
#include "Logger.h"

namespace mymuduo {
Socket::~Socket() { ::close(sockfd_); }

void Socket::BindAddress(const InetAddress& localaddr) {
  if (0 !=
      ::bind(sockfd_, (sockaddr*)localaddr.sockadrr(), sizeof(sockaddr_in))) {
    LOG_ERROR("Bind sockfd:%d fail. \n", sockfd_);
    exit(-1);
  }
}

void Socket::Listen() {
  if (0 != ::listen(sockfd_, SOMAXCONN)) {
    LOG_ERROR("Listen sockfd:%d fail. \n", sockfd_);
    exit(-1);
  }
}

int Socket::Accept(InetAddress* peeraddr) {
  sockaddr_in addr;
  socklen_t len = sizeof addr;
  std::memset(&addr, 0, sizeof addr);
  int connfd =
      ::accept4(sockfd_, (sockaddr*)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd >= 0) {
    peeraddr->set_sockaddr(addr);
  }
  return connfd;
}

void Socket::ShutdownWrite() {
  if (::shutdown(sockfd_, SHUT_WR) < 0) {
    LOG_WARNING("Shutdown failed, sockfd:%d.", sockfd_);
  }
}

void Socket::SetTcpNodelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}

void Socket::SetReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::SetReusePort(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof optval);
}
void Socket::SetKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}
}  // namespace mymuduo