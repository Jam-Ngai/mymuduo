#include "Channel.h"

#include <sys/epoll.h>

#include "EventLoop.h"
namespace mymuduo {
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false) {}
Channel::~Channel() {}

void Channel::tie(const std::shared_ptr<void>& obj) {
  tie_ = obj;
  tied_ = true;
}
// 更新fd感兴趣的事件
void Channel::Update() { loop_->UpdateChannel(this); }

// 把当前的channel从eventloop中删除
void Channel::Remove() { loop_->RemoveChannel(this); }

void Channel::HandleEvent(Timestamp receive_time) {
  if (tied_) {
    std::shared_ptr<void> guard = tie_.lock();
    if (guard) {
      HanleEventWithGuard(receive_time);
    }
  } else {
    HanleEventWithGuard(receive_time);
  }
}
// channel调用具体的回调操作
void Channel::HanleEventWithGuard(Timestamp receive_time) {
  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    if (closecallback_) {
      closecallback_();
    }
  }
  if (revents_ & EPOLLERR) {
    if (errorcallback_) {
      errorcallback_();
    }
  }
  if (revents_ & (EPOLLIN | EPOLLPRI)) {
    if (readcallback_) {
      readcallback_(receive_time);
    }
  }
  if (revents_ & EPOLLOUT) {
    if (writecallback_) {
      writecallback_();
    }
  }
}

}  // namespace mymuduo