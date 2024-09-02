#include "TcpServer.h"

#include <cstdio>
#include <utility>

#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "Logger.h"
#include "TcpConnection.h"

namespace mymuduo {
static EventLoop* CheckLoopNotNull(EventLoop* loop) {
  if (loop == nullptr) {
    LOG_ERROR("%s:%s:%d Mainloop is null! \n", __FILE__, __FUNCTION__,
              __LINE__);
    exit(-1);
  } else
    return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenaddr,
                     const std::string& name, Option option)
    : loop_(CheckLoopNotNull(loop)),
      ipport_(listenaddr.ToIpPort()),
      name_(name),
      acceptor_(new Acceptor(loop, listenaddr, option == kReusePort)),
      threadpool_(new EventLoopThreadPool(loop, name)),
      nextconnid_(1),
      started_(0) {
  // 有新用户连接时执行，在Acceptor::HandleRead中执行
  acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this,
                                                std::placeholders::_1,
                                                std::placeholders::_2));
}

TcpServer::~TcpServer() {
  for (auto& item : connections_) {
    // 防止TcpConnection被销毁
    TcpConnectionPtr conn(item.second);
    // 释放对TcpConnection的所有权
    item.second.reset();
    conn->GetLoop()->RunInLoop(
        std::bind(&TcpConnection::ConnectDestroyed, conn));
  }
}

void TcpServer::SetThreadNum(int numthreads) {
  threadpool_->SetThreadNum(numthreads);
}

void TcpServer::Start() {
  // 防止一个TcpServer被多次开启
  if (started_++ == 0) {
    threadpool_->Start();
    loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
  }
}

// 有新客户端连接，Acceptor会执行这个回调
void TcpServer::NewConnection(int sockfd, const InetAddress& peerAddr) {
  // 轮询算法，选择一个subloop来管理channel
  EventLoop* ioloop = threadpool_->GetNextLoop();
  char buf[64] = {0};
  std::snprintf(buf, sizeof buf, "-%s#%d", ipport_.c_str(), nextconnid_);
  ++nextconnid_;
  std::string connname = name_ + buf;
  LOG_INFO("TcpServer::NewConnection [%s] - new connection [%s] from %s. \n",
           name_.c_str(), connname.c_str(), peerAddr.ToIpPort().c_str());
  sockaddr_in local;
  std::memset(&local, 0, sizeof local);
  socklen_t addrlen = static_cast<socklen_t>(sizeof local);
  if (::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0) {
    LOG_WARNING("%s:%s:%d WARNING getsockname fail. \n.", __FILE__,
                __FUNCTION__, __LINE__);
  }
  InetAddress localaddr(local);
  TcpConnectionPtr conn(
      new TcpConnection(ioloop, connname, sockfd, localaddr, peerAddr));
  connections_[connname] = conn;
  // 下面的回调都是用户设置给TcpServer
  // TcpServer->TcpConnection->Channel->Poller->通知channel调用回调
  conn->SetConnectionCallback(connectioncallback_);
  conn->SetMessageCallback(messagecallback_);
  conn->SetWriteCompletedCallback(writecompletecallback_);

  conn->SetCloseCallback(
      std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
  ioloop->RunInLoop(std::bind(&TcpConnection::ConnectEstablished, conn));
}

void TcpServer::RemoveConnection(const TcpConnectionPtr& conn) {
  loop_->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr& conn) {
  LOG_INFO("TcpServer::RemoveConnectionInLoop [%s] - connection [%s]. \n",
           name_.c_str(), conn->name().c_str());
  size_t n = connections_.erase(conn->name());
  EventLoop* ioloop = conn->GetLoop();
  ioloop->QueueInLoop(std::bind(&TcpConnection::ConnectDestroyed, conn));
}
}  // namespace mymuduo