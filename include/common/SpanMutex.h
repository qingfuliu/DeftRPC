//
// Created by lqf on 23-4-21.
//

#ifndef DEFTRPC_SPANMUTEX_H
#define DEFTRPC_SPANMUTEX_H

#include <atomic>
#include "mutex.h"

namespace CLSN {

class SpanMutex : public CLSN::Mutex {
 public:
  SpanMutex() noexcept = default;

  ~SpanMutex() override = default;

  void Lock() noexcept override {
    bool flag = false;

    while (!mutex.compare_exchange_weak(flag, true, std::memory_order_acquire, std::memory_order_release)) {
      flag = false;
    }
  }

  void UnLock() noexcept override { mutex.store(false, std::memory_order_acquire); }

  bool TryLock() noexcept override {
    bool flag = false;
    mutex.compare_exchange_strong(flag, true, std::memory_order_acquire, std::memory_order_release);
    if (flag) {
      flag = false;
      return false;
    }
    return true;
  }

 private:
  std::atomic_bool mutex{false};
};

}  // namespace CLSN

#endif  // DEFTRPC_SPANMUTEX_H
