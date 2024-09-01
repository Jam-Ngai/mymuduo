#include "Acceptor.h"

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "InetAddress.h"
#include "Logger.h"
#include "Timestamp.h"

namespace mymuduo {

static int CreateNonblocking() {
  int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (sockfd < 0) {
    LOG_ERROR("listen socket create error:%d\n", errno);
  }
  return sockfd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr,
                   bool reuseport)
    : loop_(loop),
      acceptsocket_(CreateNonblocking()),
      acceptchannel_(loop, acceptsocket_.sockfd()),
      listening_(false) {
  acceptsocket_.SetReuseAddr(true);
  acceptsocket_.SetReusePort(reuseport);
  acceptsocket_.BindAddress(listenAddr);
  acceptchannel_.set_readcallback([this](Timestamp) { HandleRead(); });
}

Acceptor::~Acceptor() {
  acceptchannel_.DisableAll();
  acceptchannel_.Remove();
}

void Acceptor::Listen() {
  listening_ = true;
  acceptsocket_.Listen();
  acceptchannel_.EnableReading();
}

void Acceptor::HandleRead() {
  InetAddress peeraddr;
  int connfd = acceptsocket_.Accept(&peeraddr);
  if (connfd > 0) {
    if (newconnectioncallback_) {
      newconnectioncallback_(connfd, peeraddr);
    } else {
      ::close(connfd);
    }
  } else {
    LOG_WARNING("%s:%s:%d accept fail,errno:%d.", __FILE__, __FUNCTION__,
                __LINE__, errno);
    if (errno == EMFILE) {
      LOG_WARNING("%s:%s:%d accept reach limit!.", __FILE__, __FUNCTION__,
                  __LINE__);
    }
  }
}
}  // namespace mymuduo