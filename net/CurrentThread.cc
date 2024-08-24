#include "CurrentThread.h"

#include <sys/syscall.h>
#include <unistd.h>

namespace mymuduo {
namespace CurrentThread {
__thread int t_cachedtid = 0;

void CacheTid() {
  if (t_cachedtid == 0) {
    // 通过Linux系统调用获取当前线程的pid
    t_cachedtid = static_cast<pid_t>(::syscall(SYS_getpid));
  }
}

}  // namespace CurrentThread
}  // namespace mymuduo