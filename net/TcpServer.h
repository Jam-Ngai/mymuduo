#pragma once
// 服务器类
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "Callbacks.h"
#include "InetAddress.h"
#include "noncopyable.h"

namespace mymuduo {

class EventLoop;
class Acceptor;
class EventLoopThreadPool;

class TcpServer : noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  enum Option {
    kNoReusePort,
    kReusePort,
  };

  TcpServer(EventLoop* loop, const InetAddress& listenaddr,
            const std::string& name, Option option = kNoReusePort);
  ~TcpServer();

  const std::string& ipport() const { return ipport_; }
  const std::string& name() const { return name_; }

  EventLoop* GetLoop() const { return loop_; }
  //设置subloop的个数
  void SetThreadNum(int numthreads);
  void SetThreadInitCallback(const ThreadInitCallback& cb) {
    threadinitcallback_ = cb;
  }

  std::shared_ptr<EventLoopThreadPool> ThreadPool() { return threadpool_; }
  //开启服务监听
  void Start();

  void SetConnectionCallback(const ConnectionCallback& cb) {
    connectioncallback_ = cb;
  }
  void SetMessageCallback(const MessageCallback& cb) { messagecallback_ = cb; }

  void SetWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writecompletecallback_ = cb;
  }

 private:
  using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
  
  void NewConnection(int sockfd, const InetAddress& peerAddr);
  void RemoveConnection(const TcpConnectionPtr& conn);
  void RemoveConnectionInLoop(const TcpConnectionPtr& conn);
  
  // 用户定义的loop
  EventLoop* loop_;
  const std::string ipport_;
  const std::string name_;
  // 运行在baseloop，监听新连接事件
  std::unique_ptr<Acceptor> acceptor_;
  std::shared_ptr<EventLoopThreadPool> threadpool_;
  // 有新连接时的回调
  ConnectionCallback connectioncallback_;
  // 有写消息的回调
  MessageCallback messagecallback_;
  // 消息发送完成的额回调
  WriteCompeleteCallback writecompletecallback_;
  // loop线程初始化的回调
  ThreadInitCallback threadinitcallback_;
  int nextconnid_;
  std::atomic<int> started_;
  // 保存所有的链接
  ConnectionMap connections_;
};
}  // namespace mymuduo