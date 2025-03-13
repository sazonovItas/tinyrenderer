#pragma once

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include <atomic>

class spinlock {
  std::atomic<bool> locked{false};

public:
  void lock();

  void unlock();
};

#endif
