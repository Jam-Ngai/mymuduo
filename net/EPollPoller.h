#pragma once
#include <vector>

#include "Poller.h"
#include "sys/epoll.h"

namespace mymuduo {
class EPollPoller : public Poller {
 public:
  EPollPoller(EventLoop* loop);
  ~EPollPoller() override;

  Timestamp Poll(int timeout_ms, ChannelList* active_channels) override;
  void UpdateChannel(Channel* channel) override;
  void RemoveChannel(Channel* channel) override;

 private:
  static const int kInitEventListSize = 16;
  // 填写活跃的链接
  void FillActiveChannels(int numevent, ChannelList* active_channels) const;
  // 更新channel
  void Update(int operation, Channel* channel);

  using EventList = std::vector<epoll_event>;

  int epollfd_;
  EventList events_;
};
}  // namespace mymuduo