#include "Thread.h"

#include <semaphore.h>

#include <cstdio>

#include "CurrentThread.h"

namespace mymuduo {
std::atomic<int> Thread::numcreated_ = 0;

Thread::Thread(ThreadFunc func, const std::string& name)
    : started_(false),
      joined_(false),
      tid_(0),
      func_(std::move(func)),
      name_(name) {
  SetDefaultName();
}

Thread::~Thread() {
  if (started_ && !joined_) {
    thread_->detach();
  }
}

void Thread::Start() {
  started_ = true;
  // 开启一个新线程，执行线程函数
  sem_t sem;
  sem_init(&sem, 0, 0);
  thread_ = std::make_shared<std::thread>([&]() {
    tid_ = CurrentThread::Tid();
    sem_post(&sem);
    func_();
  });
  sem_wait(&sem);
}

void Thread::Join() {
  joined_ = true;
  thread_->join();
}

void Thread::SetDefaultName() {
  int num = numcreated_++;
  if (name_.empty()) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}
}  // namespace mymuduo