#pragma once
#include <unordered_map>
#include <vector>

#include "Timestamp.h"
#include "noncopyable.h"

namespace mymuduo {
class Channel;
class EventLoop;

// 多路事件分发器的核心IO复用模块
class Poller : noncopyable {
 public:
  using ChannelList = std::vector<Channel*>;

  Poller(EventLoop* loop);
  virtual ~Poller();

  // 所有IO复用保留的接口
  virtual Timestamp Poll(int timeout_ms, ChannelList* active_channels) = 0;
  virtual void UpdateChannel(Channel* channel) = 0;
  virtual void RemoveChannel(Channel* channel) = 0;
  // 判断channel时候在当前Poller中
  bool HasChannel(Channel* channel) const;
  // 获取默认的IO复用具体实现对象
  static Poller* NewDefaultPoller(EventLoop* loop);

 protected:
  // map的key：sockfd value:sockfd所属的channel
  using ChannelMap = std::unordered_map<int, Channel*>;
  ChannelMap channels_;

 private:
  EventLoop* ownerloop_;  // Poller所属的事件循环
};
}  // namespace mymuduo