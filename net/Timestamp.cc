#include "Timestamp.h"

#include <string.h>
namespace mymuduo {

Timestamp::Timestamp() : microseconds_since_epoch_(0) {};

Timestamp::Timestamp(int64_t microseconds_since_epoch)
    : microseconds_since_epoch_(microseconds_since_epoch) {};

Timestamp Timestamp::Now() { return Timestamp(std::time(NULL)); }

std::string Timestamp::ToString() const {
  char buffer[128];
  memset(buffer, 0, sizeof(buffer));
  tm *tm_time = localtime(&microseconds_since_epoch_);
  snprintf(buffer, sizeof(buffer), "%4d-%02d-%02d %02d:%02d:%02d",
           tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
           tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);
  return buffer;
  
}
}  // namespace mymuduo