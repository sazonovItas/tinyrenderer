#pragma once

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#ifndef SPINLOCK_LOOP_COUNT
#define SPINLOCK_LOOP_COUNT 2400
#endif

#include <atomic>

class spinlock {
  std::atomic<bool> locked{false};

public:
  void lock();

  void unlock();
};

#endif
