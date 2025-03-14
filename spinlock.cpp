#include "spinlock.h"
#include <atomic>

void spinlock::lock() {
  int locked = 1;
  int unlocked = 0;
  while (counter.compare_exchange_strong(locked, unlocked,
                                         std::memory_order_acq_rel)) {
  }
}

void spinlock::unlock() {
  int unlocked = 0;
  counter.exchange(unlocked, std::memory_order_acq_rel);
}
