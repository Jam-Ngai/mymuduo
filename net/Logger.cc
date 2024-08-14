#include "Logger.h"

#include <iostream>

#include "Timestamp.h"

namespace mymuduo {
Logger& Logger::Instance() {
  static Logger logger;
  return logger;
}

void Logger::set_loglevel(int level) { loglevel_ = level; }

void Logger::log(const std::string& msg) {
  switch (loglevel_) {
    case ENUM_INFO:
      std::cout << "[INFO]";
      break;

    case ENUM_WARNING:
      std::cout << "[WARNING]";
      break;

    case ENUM_ERROR:
      std::cout << "[ERROR]";
      break;

    case ENUM_DEBUG:
      std::cout << "[DEBUG]";
      break;

    default:
      break;
  }
  std::cout << Timestamp::Now().ToString() << " : " << msg << std::endl;
}

}  // namespace mymuduo
