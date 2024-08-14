#pragma once
#include <ctime>
#include <iostream>
#include <string>
// 时间类
namespace mymuduo {
class Timestamp {
 public:
  Timestamp();
  explicit Timestamp(int64_t microseconds_since_epoch);
  static Timestamp Now();
  std::string ToString() const;

 private:
  int64_t microseconds_since_epoch_;
};
}  // namespace mymuduo