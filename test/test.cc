#include <iostream>

#include "InetAddress.h"
#include "Logger.h"

using namespace mymuduo;

int main(int argc, char* argv[]) {
  LOG_INFO("%s %d", "hello", 3);
#ifdef DEBUG
  LOG_DEBUG("%s %d", "hello", 4);
#endif

  InetAddress addr(5001);
  std::cout << addr.ToIp() << std::endl;
  std::cout << addr.ToIpPort() << std::endl;
  std::cout << addr.ToPort() << std::endl;

  return 0;
}