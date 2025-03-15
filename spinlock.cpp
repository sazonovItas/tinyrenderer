#include "spinlock.h"
#include <atomic>

void spinlock::lock() {
  for (;;) {
    if (!_lock.exchange(true, std::memory_order_acquire)) {
      break;
    }

    while (_lock.load(std::memory_order_relaxed)) {
      for (int i = SPINLOCK_LOOP_CNT; i--;) {
      }
    }
  }
}

bool spinlock::try_lock() {
  return !_lock.load(std::memory_order_relaxed) &&
         !_lock.exchange(true, std::memory_order_acquire);
};

void spinlock::unlock() { _lock.store(false, std::memory_order_release); }
