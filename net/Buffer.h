#pragma once
#include <algorithm>
#include <string>
#include <vector>

namespace mymuduo {
class Buffer {
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t initialsize = kInitialSize)
      : buffer_(kCheapPrepend + initialsize),
        readindex_(kCheapPrepend),
        writeindex_(kCheapPrepend) {}

  size_t ReadableBytes() const { return writeindex_ - readindex_; }

  size_t WriteableBytes() const { return buffer_.size() - writeindex_; }

  size_t PrependableBytes() const { return readindex_; }

  // 返回缓冲区中可读数据的起始地址
  const char* Peek() const { return buffer_.data() + readindex_; }

  void Retrieve(size_t len) {
    if (len < ReadableBytes()) {
      readindex_ += len;
    } else {
      RetrieveAll();
    }
  }
  void RetrieveAll() {
    readindex_ = kCheapPrepend;
    writeindex_ = kCheapPrepend;
  }

  // 把messagecallbacl上报的Buffer数据转换成string后返回
  std::string RetrieveAllAsString() {
    return RetrieveAsString(ReadableBytes());
  }

  std::string RetrieveAsString(size_t len) {
    std::string result(Peek(), len);
    // 已经读取了Buffer中len长度的字节，对Buffer进行复位
    Retrieve(len);
    return result;
  }

  void EnsureWriteableBytes(size_t len) {
    if (WriteableBytes() < len) {
      MakeSpace(len);
    }
  }

  void MakeSpace(size_t len) {
    if (WriteableBytes() + PrependableBytes() < len + kCheapPrepend) {
      buffer_.resize(writeindex_ + len);
    } else {
      size_t readable = ReadableBytes();
      auto beg = buffer_.begin();
      std::copy(beg + readindex_, beg + writeindex_, beg + kCheapPrepend);
      readindex_ = kCheapPrepend;
      writeindex_ = readindex_ + readable;
    }
  }

  void Append(const char* data, size_t len) {
    EnsureWriteableBytes(len);
    std::copy(data, data + len, buffer_.begin() + writeindex_);
    writeindex_ += len;
  }

  // 从fd上读数据
  ssize_t ReadFd(int fd, int* savederrno);
  // 向fd上写数据
  ssize_t WriteFd(int fd, int* savederrno);

 private:
  std::vector<char> buffer_;
  size_t readindex_;
  size_t writeindex_;
};
}  // namespace mymuduo