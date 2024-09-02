#include "TcpConnection.h"

#include <unistd.h>

#include <functional>
#include <utility>

#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"
#include "Socket.h"

namespace mymuduo {

static EventLoop* CheckLoopNotNull(EventLoop* loop) {
  if (loop == nullptr) {
    LOG_ERROR("%s:%s:%d TcpConnection Loop is null! \n", __FILE__, __FUNCTION__,
              __LINE__);
    exit(-1);
  } else
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name,
                             int sockfd, const InetAddress& localaddr,
                             const InetAddress& peeraddr)
    : loop_(CheckLoopNotNull(loop)),
      name_(name),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localaddr_(localaddr),
      peeraddr_(peeraddr),
      highwatermark_(64 * 1024 * 1024) {
  channel_->set_readcallback(
      std::bind(&TcpConnection::HandleRead, this, std::placeholders::_1));
  channel_->set_writecallback(std::bind(&TcpConnection::HandleWrite, this));
  channel_->set_closecallback(std::bind(&TcpConnection::HandleClose, this));
  channel_->set_errorcallback(std::bind(&TcpConnection::HandleError, this));
#ifdef DEBUG
  LOG_DEBUG("TcpConnection %s at %p, fd=%d constructed. \n", name_.c_str(),
            this, socket_);
#endif
  socket_->SetKeepAlive(true);
}

TcpConnection::~TcpConnection() {
#ifdef DEBUG
  LOG_DEBUG("TcpConnection %s at %p, fd=%d disconstructed. \n", name_.c_str(),
            this, socket_);
#endif
}

void TcpConnection::HandleRead(Timestamp receivetime) {
  int savederrno = 0;
  ssize_t n = inputbuffer_.ReadFd(socket_->sockfd(), &savederrno);
  if (n > 0) {
    // 调用用户传入的回调操作
    messagecallback_(shared_from_this(), &inputbuffer_, receivetime);
  } else if (n == 0) {
    HandleClose();
  } else {
    errno = savederrno;
    LOG_WARNING("%s:%s:%d TcpConnection::handleRead. \n", __FILE__,
                __FUNCTION__, __LINE__);
    HandleError();
  }
}

void TcpConnection::HandleWrite() {
  if (channel_->IsWritingEvent()) {
    int savederrno = 0;
    ssize_t n = outputbuffer_.WriteFd(socket_->sockfd(), &savederrno);
    if (n > 0) {
      outputbuffer_.Retrieve(n);
      if (outputbuffer_.ReadableBytes() == 0) {
        // 发送完数据，关闭读事件
        channel_->DisableWriting();
        if (writecompeletedcallback_) {
          loop_->QueueInLoop(
              std::bind(writecompeletedcallback_, shared_from_this()));
        }
        if (state_ == kDisconnecting) {
          ShutdownInLoop();
        }
      }
    } else {
      LOG_WARNING("%s:%s:%d TcpConnection::handleWrite. \n", __FILE__,
                  __FUNCTION__, __LINE__);
    }
  } else {
    LOG_WARNING("%s:%s:%d TcpConnection fd=%d is down, no more writing. \n",
                __FILE__, __FUNCTION__, __LINE__, socket_->sockfd());
  }
}

void TcpConnection::HandleClose() {
  LOG_INFO("TcpConnection fd=%d state=%d. \n", socket_->sockfd(),
           state_.load());
  SetState(kDisconnected);
  channel_->DisableAll();
  TcpConnectionPtr connptr(shared_from_this());
  connetioncallback_(connptr);
  closecallback_(connptr);
}

void TcpConnection::HandleError() {
  int optval;
  socklen_t optlen = sizeof optval;
  int err = 0;
  if (::getsockopt(socket_->sockfd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) <
      0) {
    err = errno;
  } else {
    err = optval;
  }
  LOG_WARNING("%s:%s:%d TcpConnection::HandleError name=%s,SO_ERROR:%d \n.",
              __FILE__, __FUNCTION__, __LINE__, name_.c_str(), err);
}

void TcpConnection::Send(const std::string& buf) {
  if (state_ == kConneted) {
    if (loop_->IsInLoopThread()) {
      SendInLoop(buf.c_str(), buf.size());
    } else {
      loop_->RunInLoop(
          std::bind(&TcpConnection::SendInLoop, this, buf.c_str(), buf.size()));
    }
  }
}

// 发送数据
// 应用写得快而内核发送慢
// 需要把待发送数据写入缓冲区，而且设置水位回调
void TcpConnection::SendInLoop(const void* data, size_t len) {
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool faulterror = false;
  // connection已经被shutdown
  if (state_ == kDisconnected) {
    LOG_WARNING("%s:%s:%d Disconnected, give up writing \n.", __FILE__,
                __FUNCTION__, __LINE__);
    return;
  }
  if (!channel_->IsWritingEvent() && outputbuffer_.ReadableBytes() == 0) {
    nwrote = ::write(socket_->sockfd(), data, len);
    if (nwrote >= 0) {
      remaining = len - nwrote;
      // 若一次发送完，则无需再注册EPOLLOUT事件
      if (remaining == 0 && writecompeletedcallback_) {
        loop_->QueueInLoop(
            std::bind(writecompeletedcallback_, shared_from_this()));
      }
    } else {  // nwrote <0
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LOG_WARNING("%s:%s:%d TcpConnection::SendInLoop ,err:%d\n.", __FILE__,
                    __FUNCTION__, __LINE__, errno);
        if (errno == EPIPE || errno == ECONNRESET) {
          faulterror = true;
        }
      }
    }
  }
  // 一次write没有全部发送，剩余数据需要保存到缓冲区中，然后给channel注册EPOLLOUT事件，
  // tcp缓冲区可以写时，poller通知channel，channel调用writecallback，
  // 也就是TcpConnection::HandleWrite，知道outputbuffer全部发送完
  if (!faulterror && remaining > 0) {
    size_t oldlen = outputbuffer_.ReadableBytes();
    if (oldlen + remaining >= highwatermark_ && oldlen < highwatermark_ &&
        highwatermarkcallback_) {
      loop_->QueueInLoop(std::bind(highwatermarkcallback_, shared_from_this(),
                                   oldlen + remaining));
    }
    outputbuffer_.Append(static_cast<const char*>(data) + nwrote, remaining);
    if (!channel_->IsWritingEvent()) {
      channel_->EnableWriting();
    }
  }
}

void TcpConnection::Shutdown() {
  if (state_ == kConneted) {
    SetState(kDisconnecting);
    loop_->RunInLoop(std::bind(&TcpConnection::ShutdownInLoop, this));
  }
}

void TcpConnection::ShutdownInLoop() {
  if (!channel_->IsWritingEvent()) {
    socket_->ShutdownWrite();
  }
}

void TcpConnection::ConnectEstablished() {
  SetState(kConneted);
  channel_->tie(shared_from_this());
  // channel注册EPOLLIN事件
  channel_->EnableReading();
  // 新连接建立回调
  connetioncallback_(shared_from_this());
}

void TcpConnection::ConnectDestroyed() {
  if (state_ == kConneted) {
    SetState(kDisconnected);
    channel_->DisableAll();
    connetioncallback_(shared_from_this());
  }
  channel_->Remove();
}

}  // namespace mymuduo