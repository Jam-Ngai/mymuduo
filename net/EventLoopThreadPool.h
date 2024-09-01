#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "noncopyable.h"

namespace mymuduo {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  EventLoopThreadPool(EventLoop* baseloop, const std::string& name);
  ~EventLoopThreadPool();

  void SetThreadNum(int numthreads) { numthreads_ = numthreads; }

  void Start(const ThreadInitCallback& cb = ThreadInitCallback());
  //如果工作在多线程中，baseloop默认以轮询的方式分配channel给subloop
  EventLoop* GetNextLoop();

  std::vector<EventLoop*> GetAllLoops();

  bool started() const { return started_; }

  const std::string& name() const { return name_; }

 private:
  EventLoop* baseloop_;
  std::string name_;
  bool started_;
  int numthreads_;
  int next_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
};
}  // namespace mymuduo