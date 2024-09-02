#include <functional>
#include <string>
#include <utility>

#include "Buffer.h"
#include "EventLoop.h"
#include "Logger.h"
#include "TcpConnection.h"
#include "TcpServer.h"

using namespace mymuduo;

class EchoServer {
 public:
  EchoServer(EventLoop* loop, const InetAddress& addr, const std::string& name)
      : loop_(loop), server_(loop, addr, name) {
    server_.SetConnectionCallback(
        std::bind(&EchoServer::OnConnection, this, std::placeholders::_1));
    server_.SetMessageCallback(
        std::bind(&EchoServer::OnMessage, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
    server_.SetThreadNum(3);
  }
  ~EchoServer() {};

  void Start() { server_.Start(); }

 private:
  // 连接建立或断开的回调
  void OnConnection(const TcpConnectionPtr& conn) {
    if (conn->Connected()) {
      LOG_INFO("INFO Connection UP: %s", conn->peeraddr().ToIpPort().c_str());
    } else {
      LOG_INFO("INFO Connection DOWN: %s", conn->peeraddr().ToIpPort().c_str());
    }
  }
  // 可读事件的回调
  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    std::string msg = buf->RetrieveAllAsString();
    conn->Send(msg);
    conn->Shutdown();
  }
  EventLoop* loop_;
  TcpServer server_;
};

int main(int argc, char* argv[]) {
  EventLoop loop;
  InetAddress addr(8000);
  EchoServer server(&loop, addr, "EchoServer-01");
  server.Start();
  loop.Loop();
  return 0;
}