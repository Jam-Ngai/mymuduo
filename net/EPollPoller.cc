#include "EPollPoller.h"

#include <errno.h>

#include <cstring>

#include "Channel.h"
#include "Logger.h"
#include "unistd.h"

namespace mymuduo {

namespace {
// Channel的index成员
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}  // namespace

EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  if (epollfd_ < 0) {
    LOG_ERROR("epoll_create error:%d \n", errno);
    exit(-1);
  }
}

EPollPoller::~EPollPoller() { ::close(epollfd_); }

void EPollPoller::UpdateChannel(Channel* channel) {
#ifndef NDEBUG
  LOG_INFO("func=%s, fd=%s, events=%d,index=%d \n", __FUNCTION__,
           channel->fd(), channel->events(), channel->index());
#endif
  const int index = channel->index();
  if (index == kNew || index == kDeleted) {
    int fd = channel->fd();
    // 通过EPOLL_CTL_ADD添加新的channel
    if (index == kNew) {
      channels_[fd] = channel;
    }
    channel->set_index(kAdded);
    Update(EPOLL_CTL_ADD, channel);
  }
  // 通过EPOLL_CTL_MOD/DEL修改已注册的channel
  else {
    if (channel->IsNoneEvent()) {
      Update(EPOLL_CTL_DEL, channel);
      channel->set_index(kDeleted);
    } else {
      Update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EPollPoller::RemoveChannel(Channel* channel) {
  int fd = channel->fd();
#ifndef NDEBUG
  LOG_INFO("func:%s, fd=%d \n", __FUNCTION__, fd);
#endif
  int index = channel->index();
  channels_.erase(fd);
  if (index == kAdded) {
    Update(EPOLL_CTL_DEL, channel);
  }
  channel->set_index(kNew);
}

// EventLoop通过该函数获取发送事件的channel列表
void EPollPoller::FillActiveChannels(int numevent,
                                     ChannelList* active_channels) const {
  for (int i = 0; i < numevent; ++i) {
    Channel* channel = reinterpret_cast<Channel*>(events_[i].data.ptr);
    channel->set_revent(events_[i].events);
    active_channels->push_back(channel);
  }
}

// 更新Channel
void EPollPoller::Update(int operation, Channel* channel) {
  epoll_event event;
  std::memset(&event, 0, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG_WARNING("epoll_ctl del error:%d \n", errno);
    } else {
      LOG_ERROR("epoll_ctl del error:%d \n", errno);
      exit(-1);
    }
  }
}

Timestamp EPollPoller::Poll(int timeout_ms, ChannelList* active_channels) {
  // LOG_INFO("func:%s, total fd count: %lu\n", __FUNCTION__, channels_.size());
  int numevents = ::epoll_wait(epollfd_, events_.data(),
                               static_cast<int>(events_.size()), timeout_ms);
  int save_errno = errno;
  Timestamp now(Timestamp::Now());
  if (numevents > 0) {
#ifndef NDEBUG
    LOG_DEBUG("%d events happened.\n", numevents);
#endif
    FillActiveChannels(numevents, active_channels);
    if (numevents == static_cast<int>(events_.size())) {
      events_.resize(events_.size() * 2);
    }
  } else if (numevents == 0) {
#ifndef NDEBUG
    LOG_DEBUG("func=%s timeout.\n", __FUNCTION__);
#endif
  } else {
    if (save_errno != EINTR) {
      errno = save_errno;
      LOG_ERROR("EPollPoller::Poller() error:%d \n", errno);
      exit(-1);
    }
  }
  return now;
}

}  // namespace mymuduo