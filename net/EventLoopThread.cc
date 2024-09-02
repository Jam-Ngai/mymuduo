#include "EventLoopThread.h"

#include <memory>
#include <utility>

#include "EventLoop.h"

namespace mymuduo {
EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const std::string& name)
    : loop_(nullptr),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::ThreadFunc, this), name),
      mtx_(),
      callback_(cb) {}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_ != nullptr) {
    loop_->Quit();
    thread_.Join();
  }
}

EventLoop* EventLoopThread::StartLoop() {
  thread_.Start();  // 启动新线程
  EventLoop* loop = nullptr;
  {
    std::unique_lock<std::mutex> lock(mtx_);
    while (loop == nullptr) {
      cond_.wait(lock);
    }
    loop = loop_;
  }
  return loop;
}

// 在新线程里运行的回调函数
void EventLoopThread::ThreadFunc() {
  EventLoop loop;  // 创建一个独立的EventLoop，与上面的线程一一对应
  if (callback_) {
    callback_(&loop);
  }

  {
    std::unique_lock<std::mutex> lock(mtx_);
    loop_ = &loop;
    cond_.notify_one();
  }
  loop.Loop();
  std::unique_lock<std::mutex> lock(mtx_);
  loop_ = nullptr;
}
}  // namespace mymuduo