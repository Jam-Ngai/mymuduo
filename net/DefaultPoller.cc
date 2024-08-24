#include <stdlib.h>

#include "EPollPoller.h"
#include "Poller.h"

namespace mymuduo {
Poller* Poller::NewDefaultPoller(EventLoop* loop) {
  if (::getenv("MUDUO_USE_POLL")) {
    return nullptr;
  } else {
    return new EPollPoller(loop);
  }
}
}  // namespace mymuduo