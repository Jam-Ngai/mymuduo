#pragma once

// 禁用拷贝构造和赋值
namespace mymuduo {

class noncopyable {
 public:
  noncopyable(const noncopyable&) = delete;
  noncopyable& operator=(const noncopyable&) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};
}  // namespace mymuduo