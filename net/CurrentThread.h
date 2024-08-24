#pragma once
namespace mymuduo {

namespace CurrentThread {
extern __thread int t_cachedtid;
void CacheTid();

inline int Tid() {
  if (__builtin_expect(t_cachedtid == 0, 0)) {
    CacheTid();
  }
  return t_cachedtid;
}
}  // namespace CurrentThread
}  // namespace mymuduo