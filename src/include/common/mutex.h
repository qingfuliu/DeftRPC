//
// Created by lqf on 23-4-21.
//

#ifndef DEFTRPC_MUTEX_H
#define DEFTRPC_MUTEX_H

#include <atomic>
#include <cassert>
#include <type_traits>
#include "common/thread.h"

namespace clsn {

class SpanMutex;

class Mutex {
 public:
  Mutex() noexcept = default;

  virtual ~Mutex() = default;

  virtual void Lock() = 0;

  virtual void UnLock() noexcept = 0;

  virtual bool TryLock() noexcept = 0;
};

class MutexGuard {
 public:
  explicit MutexGuard(Mutex *m) noexcept : m_mutex_(m) {
    assert(nullptr != m);
    m_mutex_->Lock();
  }

  ~MutexGuard() { m_mutex_->UnLock(); }

 public:
  Mutex *m_mutex_;
};

template <typename mutex, typename v = std::enable_if_t<std::is_base_of_v<Mutex, mutex> > >
class ReentrantMutex : public mutex {
  using Base = std::decay_t<mutex>;

 public:
  ReentrantMutex() noexcept = default;

  ~ReentrantMutex() override = default;

  void Lock() noexcept override {
    auto temp = m_thread_id_.load(std::memory_order_acquire);
    auto this_id = thread::ThisThreadId();
    if (temp == this_id) {
      return;
    }

    if (-1 == temp) {
      Base::Lock();
      m_thread_id_.store(this_id, std::memory_order_release);
      return;
    }
  }

  void UnLock() noexcept override {
    m_thread_id_.store(-1, std::memory_order_release);
    Base::UnLock();
  }

  bool TryLock() noexcept override {
    auto this_id = thread::ThisThreadId();
    auto temp = m_thread_id_.load(std::memory_order_acquire);
    if (temp == this_id) {
      return true;
    }
    if (temp == -1 && Base::TryLock()) {
      m_thread_id_.store(this_id, std::memory_order_release);
      return true;
    }
    return false;
  }

 private:
  std::atomic_int m_thread_id_{-1};
};

}  // namespace clsn

#endif  // DEFTRPC_MUTEX_H
