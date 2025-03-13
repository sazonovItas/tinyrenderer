#include "spinlock.h"

#define SPINLOCK_LOOP_COUNT 2400

void spinlock::lock() {
#define do_nothing

  for (;;) {
    if (!locked.exchange(true))
      return;

    for (; locked.load();) {
      for (volatile int i = 0; i < SPINLOCK_LOOP_COUNT; i += 1)
        do_nothing;
    }
  }
}

void spinlock::unlock() { locked.store(false); }
