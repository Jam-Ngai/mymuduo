#pragma once
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "CurrentThread.h"
#include "Timestamp.h"
#include "noncopyable.h"

// 事件循环类
// 主要包含Channel和Poller
namespace mymuduo {
class Poller;
class Channel;

class EventLoop : noncopyable {
 public:
  using Functor = std::function<void()>;

  EventLoop();
  ~EventLoop();
  // 开启事件循环
  void Loop();
  // 退出事件循环
  void Quit();

  Timestamp get_pollreturntime() const { return pollreturntime_; };
  // 在当前loop中执行cb
  void RunInLoop(Functor cb);
  // 把cb放入队列中，唤醒loop所在的线程，执行cb
  void QueueInLoop(Functor cb);
  // 唤醒loop所在的线程
  void Wakeup();

  void UpdateChannel(Channel* channel);
  void RemoveChannel(Channel* channel);
  bool HasChannel(Channel* channel);
  // 判断Eventloop时候在创建自己的线程里
  bool IsInLoopThread() const { return threadid_ == CurrentThread::Tid(); }

 private:
  using ChannelList = std::vector<Channel*>;
  // 唤醒线程使用
  void HandleRead();
  // 执行回调
  void DoPendingFunctor();

  // 是否正在执行事件循环
  std::atomic<bool> looping_;
  // 标识退出事件循环
  std::atomic<bool> quit_;
  // 标识当前Eventloop时候有需要执行的回调操作
  std::atomic<bool> callingpendindfunctor_;
  // 记录当前Eventloop所在线程的id
  const pid_t threadid_;
  // Poller返回channel发生事件的时间点
  Timestamp pollreturntime_;
  std::unique_ptr<Poller> poller_;
  // 当mainloop获取一个新用户的Channel，通过轮询算法选择一个subloop,通过该成员唤醒subloop处理
  int wakeupfd_;
  std::unique_ptr<Channel> wakeupchannel_;

  ChannelList activechannels_;
  // 存储Eventloop所有需要执行的回调操作
  std::vector<Functor> pendingfunctor_;
  // 用来保护上面vector容器的线程安全操作
  std::mutex mtx_;
};
}  // namespace mymuduo