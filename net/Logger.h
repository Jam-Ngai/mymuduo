#pragma once
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>

#include "noncopyable.h"

namespace mymuduo {
// 日志类，单例模式
class Logger : noncopyable {
 public:
  enum LogLevel {
    ENUM_INFO,     // 一般信息
    ENUM_WARNING,  // 错误信息
    ENUM_ERROR,    // core信息
    ENUM_DEBUG     // 调试信息
  };
  static Logger& Instance();
  void set_loglevel(int level);
  template <typename... Args>
  void LogInfo(const std::string& format, Args&&... args);
  template <typename... Args>
  void LogWarning(const std::string& format, Args&&... args);
  template <typename... Args>
  void LogError(const std::string& format, Args&&... args);
  template <typename... Args>
  void LogDebug(const std::string& format, Args&&... args);

 private:
  int loglevel_;
  std::mutex mtx_;

  void log(const std::string& msg);
};

template <typename... Args>
void Logger::LogInfo(const std::string& format, Args&&... args) {
  std::unique_lock<std::mutex> lock(mtx_);
  Logger& logger = Logger::Instance();
  logger.set_loglevel(ENUM_INFO);
  char buffer[1024];
  std::memset(buffer, 0, sizeof(buffer));
  std::snprintf(buffer, sizeof(buffer), format.data(),
                std::forward<Args>(args)...);
  logger.log(buffer);
}

template <typename... Args>
void Logger::LogWarning(const std::string& format, Args&&... args) {
  std::unique_lock<std::mutex> lock(mtx_);
  Logger& logger = Logger::Instance();
  logger.set_loglevel(ENUM_WARNING);
  char buffer[1024];
  std::memset(buffer, 0, sizeof(buffer));
  std::snprintf(buffer, sizeof(buffer), format.data(),
                std::forward<Args>(args)...);
  logger.log(buffer);
}

template <typename... Args>
void Logger::LogError(const std::string& format, Args&&... args) {
  std::unique_lock<std::mutex> lock(mtx_);
  Logger& logger = Logger::Instance();
  logger.set_loglevel(ENUM_ERROR);
  char buffer[1024];

  std::memset(buffer, 0, sizeof(buffer));
  std::snprintf(buffer, sizeof(buffer), format.data(),
                std::forward<Args>(args)...);
  logger.log(buffer);
}

template <typename... Args>
void Logger::LogDebug(const std::string& format, Args&&... args) {
  std::unique_lock<std::mutex> lock(mtx_);
  Logger& logger = Logger::Instance();
  logger.set_loglevel(ENUM_DEBUG);
  char buffer[1024];
  std::memset(buffer, 0, sizeof(buffer));
  std::snprintf(buffer, sizeof(buffer), format.data(),
                std::forward<Args>(args)...);
  logger.log(buffer);
}

#define LOG_INFO(format, ...)              \
  {                                        \
    Logger& logger = Logger::Instance();   \
    logger.LogInfo(format, ##__VA_ARGS__); \
  }
#define LOG_WARNING(format, ...)              \
  {                                           \
    Logger& logger = Logger::Instance();      \
    logger.LogWarning(format, ##__VA_ARGS__); \
  }
#define LOG_ERROR(format, ...)              \
  {                                         \
    Logger& logger = Logger::Instance();    \
    logger.LogError(format, ##__VA_ARGS__); \
  }
#ifdef DEBUG
#define LOG_DEBUG(format, ...)              \
  {                                         \
    Logger& logger = Logger::Instance();    \
    logger.LogDebug(format, ##__VA_ARGS__); \
  }
#endif

}  // namespace mymuduo
