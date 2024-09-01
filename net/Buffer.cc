#include "Buffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

namespace mymuduo {
ssize_t Buffer::ReadFd(int fd, int* savederrno) {
  // 在栈上的内存空间
  char extrabuf[65536] = {0};
  iovec vec[2];
  // Buffer底层缓冲区剩余可写的大小
  const size_t writeable = WriteableBytes();
  vec[0].iov_base = buffer_.data() + writeindex_;
  vec[0].iov_len = writeable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  const int iovcnt = (writeable < sizeof extrabuf) ? 2 : 1;
  const ssize_t n = ::readv(fd, vec, iovcnt);
  if (n < 0) {
    *savederrno = errno;
  } else if (n <= writeable) {
    writeindex_ += n;
  } else {  // extrabuf也写入了数据
    writeindex_ = buffer_.size();
    Append(extrabuf, n - writeable);
  }
  return n;
}

ssize_t Buffer::WriteFd(int fd, int* savederrno) {
  ssize_t n = ::write(fd, Peek(), ReadableBytes());
  if (n < 0) {
    *savederrno = errno;
  }
  return n;
}
}  // namespace mymuduo