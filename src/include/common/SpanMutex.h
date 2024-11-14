//
// Created by lqf on 23-4-21.
//

#ifndef DEFTRPC_SPANMUTEX_H
#define DEFTRPC_SPANMUTEX_H

#include <atomic>
#include "mutex.h"

namespace clsn {

class SpanMutex : public clsn::Mutex {
 public:
  SpanMutex() noexcept = default;

  ~SpanMutex() override = default;

  void Lock() noexcept override {
    bool flag = false;

    while (!m_mutex_.compare_exchange_weak(flag, true, std::memory_order_acquire, std::memory_order_release)) {
      flag = false;
    }
  }

  void UnLock() noexcept override { m_mutex_.store(false, std::memory_order_acquire); }

  bool TryLock() noexcept override {
    bool flag = false;
    m_mutex_.compare_exchange_strong(flag, true, std::memory_order_acquire, std::memory_order_release);
    return !flag;
  }

 private:
  std::atomic_bool m_mutex_{false};
};

}  // namespace clsn

#endif  // DEFTRPC_SPANMUTEX_H
