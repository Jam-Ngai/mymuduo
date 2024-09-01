#include "Thread.h"

#include <semaphore.h>

#include <cstring>

#include "CurrentThread.h"

namespace mymuduo {
std::atomic<int> Thread::numcreated_(0);

Thread::Thread(ThreadFunc func, const std::string &name)
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
  // 创建新线程
  sem_t sem;
  sem_init(&sem, 0, 0);
  thread_ = std::make_unique<std::thread>([&]() {
    tid_ = CurrentThread::Tid();
    sem_post(&sem);
    func_();
  });
  // 等待获取新线程的tid
  sem_wait(&sem);
}

void Thread::Join() {
  joined_ = true;
  thread_->join();
}

void Thread::SetDefaultName() {
  int num = ++numcreated_;
  if (name_.empty()) {
    char buf[32] = {0};
    std::snprintf(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}
}  // namespace mymuduo