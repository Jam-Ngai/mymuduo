#pragma once

#include <unistd.h>

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "noncopyable.h"

namespace mymuduo {
class Thread : noncopyable {
 public:
  using ThreadFunc = std::function<void()>;

  explicit Thread(ThreadFunc, const std::string &name = std::string());
  ~Thread();

  void Start();
  void Join();

  bool started() const { return started_; }
  bool joined() const { return joined_; }
  pid_t tid() const { return tid_; }
  std::string name() const { return name_; }

  static int numcreated() { return numcreated_; }

 private:
  void SetDefaultName();
  bool started_;
  bool joined_;
  std::shared_ptr<std::thread> thread_;
  pid_t tid_;
  ThreadFunc func_;
  std::string name_;
  static std::atomic<int> numcreated_;
};

}  // namespace mymuduo
