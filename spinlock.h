#pragma once

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#ifndef SPINLOCK_LOOP_CNT
#define SPINLOCK_LOOP_CNT 2400
#endif

#include <atomic>

class spinlock {
  std::atomic<bool> _lock{false};

public:
  spinlock() {}

  spinlock(const spinlock &spinlock) {}

  void lock();

  bool try_lock();

  void unlock();
};

#endif
