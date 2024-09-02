#pragma once
#include <functional>

#include "Channel.h"
#include "Socket.h"
#include "noncopyable.h"


namespace mymuduo {
class EventLoop;
class InetAddress;

class Acceptor : noncopyable {
 public:
  using NewConnectionCallback =
      std::function<void(int sockfd, const InetAddress&)>;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
  ~Acceptor();

  void SetNewConnectionCallback(const NewConnectionCallback& cb) {
    newconnectioncallback_ = cb;
  }

  void Listen();
  bool listening() const { return listening_; }

 private:
  void HandleRead();
  EventLoop* loop_;
  Socket acceptsocket_;
  Channel acceptchannel_;
  //轮询找到subloop，唤醒并分发新客户端的channel
  NewConnectionCallback newconnectioncallback_;
  bool listening_;
};
}  // namespace mymuduo