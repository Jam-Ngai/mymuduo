#include "Logger.h"
using namespace mymuduo;
int main(int argc, char* argv[]) {
  LOG_INFO("%s %d", "hello", 3);
#ifdef DEBUG
  LOG_DEBUG("%s %d", "hello", 4);
#endif
  return 0;
}