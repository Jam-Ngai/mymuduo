#pragma once
#include <functional>
#include <memory>

#include "Timestamp.h"
#include "noncopyable.h"

namespace mymuduo {
class EventLoop;

// 封装了sockfd和其感兴趣的event，如EPOLLIN、EPOLLOUT事件
class Channel : noncopyable {
 public:
  using EventCallBack = std::function<void()>;
  using ReadEventCallBack = std::function<void(Timestamp)>;

  Channel(EventLoop* loop, int fd);
  ~Channel();

  void HandleEvent(Timestamp receive_time);  // fd得到Poller通知以后，处理事件

  // 设置回调函数对象
  void set_readcallback(ReadEventCallBack cb) {
    readcallback_ = std::move(cb);
  };
  void set_writecallback(EventCallBack cb) { writecallback_ = std::move(cb); };
  void set_closecallback(EventCallBack cb) { closecallback_ = std::move(cb); };
  void set_errorcallback(EventCallBack cb) { errorcallback_ = std::move(cb); };

  // 防止Channel被手动remove掉，还在执行回调操作
  void tie(const std::shared_ptr<void>&);

  int get_fd() const { return fd_; };
  int get_events() const { return events_; };
  void set_revent(int revents) { revents_ = revents; };

  void EnableReading() {
    events_ |= kReadEvent;
    update();
  }
  void DisableReading() {
    events_ &= ~kReadEvent;
    update();
  }
  void EnableWriting() {
    events_ |= kWriteEvent;
    update();
  }
  void DisableWriting() {
    events_ &= ~kWriteEvent;
    update();
  }
  void DisableAll() {
    events_ = kNoneEvent;
    update();
  }

  bool IsNoneEvent() const { return events_ == kNoneEvent; }
  bool IsReadingEvent() const { return events_ & kReadEvent; }
  bool IsWritingEvent() const { return events_ & kWriteEvent; }

  int get_index() const { return index_; };
  void set_index(int idx) { index_ = idx; };

  EventLoop* OwnerLoop() { return loop_; };
  void remove();

 private:
  void update();
  void HanleEventWithGuard(Timestamp receive_time);

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;
  EventLoop* loop_;  // 事件循环
  const int fd_;     // Poller监听的对象
  int events_;       // 注册fd感兴趣事件
  int revents_;      // Poller返回的具体发生的事件
  int index_;

  std::weak_ptr<void> tie_;
  bool tied_;
  // 因为channel通道里面能够获知fd最终发生的具体的事件revents，所以它负责调用具体事件的回调操作
  ReadEventCallBack readcallback_;
  EventCallBack writecallback_;
  EventCallBack closecallback_;
  EventCallBack errorcallback_;
};
}  // namespace mymuduo