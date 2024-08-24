#include "Poller.h"

#include "Channel.h"

namespace mymuduo {
Poller::Poller(EventLoop* loop) : ownerloop_(loop) {};
Poller::~Poller() = default;

bool Poller::HasChannel(Channel* channel) const {
  auto it = channels_.find(channel->get_fd());
  return it != channels_.end() && it->second == channel;
}

}  // namespace mymuduo