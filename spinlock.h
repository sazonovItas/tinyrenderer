#pragma once

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#ifndef SPINLOCK_LOOP_COUNT
#define SPINLOCK_LOOP_COUNT 24
#endif

#include <atomic>

class spinlock {
  std::atomic_int counter{0};

public:
  void lock();

  void unlock();
};

#endif
