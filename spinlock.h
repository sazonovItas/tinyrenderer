#pragma once

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include <atomic>

class spinlock {
  std::atomic<int> counter{0};

public:
  spinlock() {};

  spinlock(const spinlock &spinlock) {}

  void lock();

  void unlock();
};

#endif
