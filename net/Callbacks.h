#pragma once
#include <functional>
#include <memory>

#include "Timestamp.h"

namespace mymuduo {

class Buffer;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback =
    std::function<void(const TcpConnectionPtr&, size_t)>;
using MessageCallback =
    std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;
using WriteCompeleteCallback = std::function<void(const TcpConnectionPtr&)>;

}  // namespace mymuduo