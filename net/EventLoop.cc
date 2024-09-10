#include "EventLoop.h"

#include <error.h>
#include <sys/eventfd.h>

#include "Channel.h"
#include "Logger.h"
#include "Poller.h"
#include "fcntl.h"
#include "unistd.h"

using namespace mymuduo;

namespace {
// 防止一个线程创建多个Eventloop
__thread EventLoop* t_loopinthisthread = nullptr;
// 默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;
// 创建eventfd，用来唤醒subReactor
int CreateEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG_ERROR("eventfd error:%d\n", errno);
  }
  return evtfd;
}
}  // namespace

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingpendindfunctor_(false),
      threadid_(CurrentThread::Tid()),
      poller_(Poller::NewDefaultPoller(this)),
      wakeupfd_(CreateEventfd()),
      wakeupchannel_(new Channel(this, wakeupfd_)) {
#ifndef NDEBUG
  LOG_DEBUG("EventLoop created %p in thread %d. \n", this, threadid_);
#endif
  if (t_loopinthisthread) {
    LOG_ERROR("Another EventLoop %p exists in this thread %d. \n",
              t_loopinthisthread, threadid_);
    exit(-1);
  } else {
    t_loopinthisthread = this;
  }

  // 设置wakeupfd的事件类型以及事件发生的回调操作
  wakeupchannel_->set_readcallback(std::bind(&EventLoop::HandleRead, this));
  // 每一个eventloop都将监听wakeupchannel的读事件
  wakeupchannel_->EnableReading();
}

EventLoop::~EventLoop() {
  wakeupchannel_->DisableAll();
  wakeupchannel_->Remove();
  ::close(wakeupfd_);
  t_loopinthisthread = nullptr;
}

void EventLoop::HandleRead() {
  uint64_t one = 1;
  ssize_t n = read(wakeupfd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_WARNING("EventLoop::HandleRead() reads %d bytes instead of 8\n", &n);
  }
}

void EventLoop::Loop() {
  looping_ = true;
  quit_ = false;
  LOG_INFO("EventLoop %p start looping.\n", this);

  while (!quit_) {
    activechannels_.clear();
    // 监听client的fd和wakeupfd
    pollreturntime_ = poller_->Poll(kPollTimeMs, &activechannels_);
    for (auto channel : activechannels_) {
      // Poller返回发生事件的channel，eventloop处理事件
      channel->HandleEvent(pollreturntime_);
    }
    // 执行当前eventloop需要处理的回调操作
    DoPendingFunctor();
  }
  LOG_INFO("EventLoop %p stop looping.\n", this);
  looping_ = false;
}

void EventLoop::Quit() {
  quit_ = true;
  if (!IsInLoopThread()) {
    Wakeup();
  }
}

void EventLoop::RunInLoop(Functor cb) {
  if (IsInLoopThread()) {
    cb();
  } else {
    QueueInLoop(std::move(cb));
  }
}
void EventLoop::QueueInLoop(Functor cb) {
  {
    std::unique_lock<std::mutex> lock(mtx_);
    pendingfunctor_.emplace_back(cb);
  }
  if (!IsInLoopThread() || callingpendindfunctor_) {
    Wakeup();
  }
}

// 向wakeupfd写一个数据用来唤醒所在线程
// wakeupchannel会发生读事件，其线程就会被唤醒
void EventLoop::Wakeup() {
  uint64_t one = 1;
  ssize_t n = write(wakeupfd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG_WARNING("Eventloop::write() writes %lu bytes instead of %lu", n,
                sizeof one);
  }
}

void EventLoop::UpdateChannel(Channel* channel) {
  poller_->UpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel* channel) {
  poller_->RemoveChannel(channel);
}

bool EventLoop::HasChannel(Channel* channel) {
  return poller_->HasChannel(channel);
}

void EventLoop::DoPendingFunctor() {
  std::vector<Functor> functors;
  callingpendindfunctor_ = true;
  {
    std::unique_lock<std::mutex> lock(mtx_);
    functors.swap(pendingfunctor_);
  }
  for (const Functor& functor : functors) {
    functor();
  }
  callingpendindfunctor_ = false;
}