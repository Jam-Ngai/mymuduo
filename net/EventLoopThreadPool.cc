#include "EventLoopThreadPool.h"

#include <cstring>

#include "EventLoopThread.h"

namespace mymuduo {
EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop,
                                         const std::string& name)
    : baseloop_(baseloop), name_(name), started_(false), next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::Start(const ThreadInitCallback& cb) {
  started_ = true;
  for (int i = 0; i < numthreads_; ++i) {
    char buf[name_.size() + 32];
    std::snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
    EventLoopThread* t = new EventLoopThread(cb, buf);
    threads_.emplace_back(std::unique_ptr<EventLoopThread>(t));
    loops_.emplace_back(t->StartLoop());
  }
  if (numthreads_ == 0 && cb) {
    cb(baseloop_);
  }
}

EventLoop* EventLoopThreadPool::GetNextLoop() {
  EventLoop* loop = baseloop_;
  if (loops_.empty()) {
    loop = loops_[next_];
    ++next_;
    if (static_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::GetAllLoops() {
  if (loops_.empty()) {
    return std::vector<EventLoop*>(1, baseloop_);
  } else
    return loops_;
}
}  // namespace mymuduo