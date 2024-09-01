#pragma once
#include <atomic>
#include <memory>
#include <string>

#include "Buffer.h"
#include "Callbacks.h"
#include "InetAddress.h"
#include "Timestamp.h"
#include "noncopyable.h"

namespace mymuduo {

class Channel;
class Socket;
class EventLoop;

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop* loop, const std::string& name_, int sockfd,
                const InetAddress& localaddr, const InetAddress& peeraddr);
  ~TcpConnection();

  EventLoop* GetLoop() const { return loop_; }
  const std::string& name() const { return name_; }
  const InetAddress& localaddr() const { return localaddr_; }
  const InetAddress& peeraddr() const { return peeraddr_; }

  bool Connected() const { return state_ == kConneted; }
  bool Disconnected() const { return state_ == kDisconnected; }

  void SetConnectionCallback(const ConnectionCallback& cb) {
    connetioncallback_ = cb;
  }
  void SetMessageCallback(const MessageCallback& cb) { messagecallback_ = cb; }
  void SetWriteCompletedCallback(const WriteCompeleteCallback& cb) {
    writecompeletedcallback_ = cb;
  }
  void SetHighWaterMarkCallback(const HighWaterMarkCallback& cb,
                                size_t highwatermark) {
    highwatermarkcallback_ = cb;
    highwatermark_ = highwatermark;
  }
  void SetCloseCallback(const CloseCallback& cb) { closecallback_ = cb; }

  // 连接建立
  void ConnectEstablished();
  // 连接销毁
  void ConnectDestroyed();

  void Send(const std::string& buf);

  void Shutdown();

 private:
  enum StateE { kDisconnected, kConnecting, kConneted, kDisconnecting };

  void SetState(StateE state) { state_ = state; }

  void HandleRead(Timestamp receivetime);
  void HandleWrite();
  void HandleClose();
  void HandleError();

  void SendInLoop(const void* data, size_t len);

  void ShutdownInLoop();

  // 绝对不会是baseloop，因为TcpConnection都是在subloop里管理的
  EventLoop* loop_;
  const std::string name_;
  std::atomic<int> state_;
  bool reading_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  const InetAddress localaddr_;
  const InetAddress peeraddr_;

  ConnectionCallback connetioncallback_;
  MessageCallback messagecallback_;
  WriteCompeleteCallback writecompeletedcallback_;
  HighWaterMarkCallback highwatermarkcallback_;
  CloseCallback closecallback_;

  size_t highwatermark_;

  Buffer inputbuffer_;
  Buffer outputbuffer_;
};

}  // namespace mymuduo