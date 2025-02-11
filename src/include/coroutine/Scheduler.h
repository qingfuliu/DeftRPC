//
// Created by lqf on 23-4-18.
//

#ifndef DEFTRPC_SCHEDULER_H
#define DEFTRPC_SCHEDULER_H

#include <unistd.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
#include "common/LockFreeQueue.h"
#include "common/common.h"
#include "common/mutex.h"
#include "common/task/Task.h"
#include "common/thread.h"
#include "common/timer/Timer.h"
namespace clsn {
class Mutex;

class SharedStack;

class Poller;

class Coroutine;

class Scheduler {
 public:
  // for hook
 public:
  static Scheduler *GetThreadScheduler();

  static Coroutine *GetCurCoroutine();

  static Coroutine *GetPrevCoroutine();

  static SharedStack *GetThreadSharedStack();

  static Poller *GetThreadPoller();

  static void RegisterWrite(int fd, Coroutine *coroutine);

  static void RegisterRead(int fd, Coroutine *coroutine);

  static void CancelRegister(int fd);

  // TODO:Under specific circumstances, thread local variables can be optimized
  static void SwapIn(Coroutine *coroutine);

  static void Yield();

  static void Terminal();

 public:
  using ExtraTask = Task;

  enum class kSchedulerState : std::uint16_t { DoNothing = 0, EpollWait, HandleActiveFd, HandleExtraState };

  explicit Scheduler(size_t sharedStackSize);

  Scheduler() noexcept : Scheduler(0) {}

  virtual ~Scheduler();

  void AddDefer(ExtraTask f) noexcept {
    m_extra_tasks_.EnQueue(std::move(f));
    if (auto cur_state = m_state_.load(std::memory_order_acquire);
        cur_state == kSchedulerState::EpollWait || kSchedulerState::DoNothing == cur_state) {
      Notify();
    }
  }

  virtual void Start(int timeout);

  virtual bool IsRunning() noexcept { return !m_stop_.load(std::memory_order_acquire); }

  virtual void Stop();

  [[nodiscard]] bool IsStop() const noexcept { return m_stop_.load(std::memory_order_acquire); }

  template <typename Rep, typename Period>
  TimerIdType DoAfter(const std::chrono::duration<Rep, Period> &interval, Coroutine *coroutine) {
    return DoEvery(interval, [coroutine]() { Scheduler::SwapIn(coroutine); }, false);
  }

  template <typename Rep, typename Period>
  TimerIdType DoAfter(const std::chrono::duration<Rep, Period> &interval, ExtraTask f) {
    return DoEvery(interval, f, false);
  }

  template <typename Clock, typename Dur>
  uint64_t DoUntil(const std::chrono::time_point<Clock, Dur> &timePoint, ExtraTask f) noexcept {
    typename Clock::time_point now_clock = Clock::now();
    auto delta = now_clock - timePoint;
    if (IsInLoopThread()) {
      return m_timer_queue_->AddTimer(delta, std::move(f));
    }

    AddTask([this, delta, f]() mutable { m_timer_queue_->AddTimer(delta, std::move(f)); });
    return -1;
  }

  template <typename Rep, typename Period>
  TimerIdType DoEvery(const std::chrono::duration<Rep, Period> &interval, ExtraTask f, bool repeated = true) {
    if (IsInLoopThread()) {
      return m_timer_queue_->AddTimer(interval, std::move(f), repeated);
    }
    AddTask([this, interval, f, repeated]() mutable { m_timer_queue_->AddTimer(interval, std::move(f), repeated); });
    return -1;
  }

  void CancelTimer(TimerIdType id) noexcept {
    if (IsInLoopThread()) {
      m_timer_queue_->CancelTimer(id);
      return;
    }
    AddTask([this, id]() mutable { m_timer_queue_->CancelTimer(id); });
  }

  [[nodiscard]] bool IsInLoopThread() const noexcept { return thread::ThisThreadId() == m_pid_; }

  void SetStopCallback(const Task &f) { m_stop_callback_ = f; }

  SharedStack *GetSharedStack() noexcept { return m_shared_stack_.get(); }

 private:
  void AddTask(ExtraTask f) noexcept {
    if (IsInLoopThread()) {
      f();
      return;
    }
    m_extra_tasks_.EnQueue(std::move(f));
    if (auto cur_state = m_state_.load(std::memory_order_acquire);
        cur_state == kSchedulerState::EpollWait || kSchedulerState::DoNothing == cur_state) {
      Notify();
    }
  }

 private:
  void Notify() noexcept { WriteEventFd(); }

  void ReadEventFd() const noexcept;

  void WriteEventFd() const noexcept;

 protected:
  std::atomic_bool m_stop_;

 private:
  const pid_t m_pid_;
  std::atomic<kSchedulerState> m_state_;

  std::unique_ptr<Coroutine> m_main_coroutine_;  // header of m_coroutines_list_
  std::list<Coroutine *> m_coroutines_list_;
  // all coroutines
  std::vector<std::unique_ptr<Coroutine>> m_coroutines_;
  //  ExtraTask
  LockFreeQueue<ExtraTask> m_extra_tasks_;
  // 共享栈
  std::unique_ptr<SharedStack> m_shared_stack_;
  // m_poller_
  std::unique_ptr<Poller> m_poller_;
  std::vector<Runnable *> m_active_runnable_;
  // event m_socket_
  int m_event_fd_;
  // timer queue
  std::unique_ptr<TimerQueue> m_timer_queue_;
  // close callback
  Task m_stop_callback_{nullptr};
};

class MultiThreadScheduler : public Scheduler {
 public:
  class SchedulerThread {
   public:
    SchedulerThread() = default;
    ~SchedulerThread() = default;

    Scheduler *GetScheduler() { return m_scheduler_; }

    void Start(int timeout) {
      m_thread_ = std::thread([&, timeout] { this->ThreadFunction(timeout); });
      std::unique_lock<std::mutex> guard(m_mutex_);
      m_condition_variable_.wait(guard, [&] { return nullptr != m_scheduler_; });
    }

    void Stop() {
      {
        std::unique_lock<std::mutex> guard(m_mutex_);
        if (m_scheduler_ == nullptr) {
          return;
        }
        m_scheduler_->Stop();
        m_condition_variable_.wait(guard, [&] { return nullptr == m_scheduler_; });
      }
      m_thread_.join();
    }

    [[nodiscard]] bool IsRunning() noexcept {
      std::unique_lock<std::mutex> guard(m_mutex_);
      return nullptr != m_scheduler_ && m_scheduler_->IsRunning();
    }

    SchedulerThread &WithPreparation(std::function<void(void)> preparation) {
      m_preparation_ = std::move(preparation);
      return *this;
    }

    SchedulerThread &WithCleanup(std::function<void(void)> cleanup) {
      m_cleanup_ = std::move(cleanup);
      return *this;
    }

    int GetIndex() const noexcept { return m_index_; }

    void SetIndex(int index) noexcept { m_index_ = index; }

   private:
    void ThreadFunction(int timeout) {
      if (m_preparation_) {
        m_preparation_();
      }

      Scheduler scheduler;
      {
        std::unique_lock<std::mutex> guard(m_mutex_);
        m_scheduler_ = &scheduler;
        m_condition_variable_.notify_one();
      }
      scheduler.Start(timeout);

      if (m_cleanup_) {
        m_cleanup_();
      }

      {
        std::unique_lock<std::mutex> guard(m_mutex_);
        m_scheduler_ = nullptr;
        m_condition_variable_.notify_one();
      }
    }

   private:
    int m_index_{-1};
    std::mutex m_mutex_{};
    std::condition_variable m_condition_variable_;
    std::thread m_thread_{};
    std::function<void(void)> m_preparation_;
    std::function<void(void)> m_cleanup_;
    Scheduler *m_scheduler_{nullptr};
  };

 public:
  MultiThreadScheduler(std::uint32_t threadNumber, size_t sharedStackSize)
      : Scheduler(sharedStackSize), m_schedulers_(threadNumber) {}

  MultiThreadScheduler() noexcept : Scheduler(0) {}

  ~MultiThreadScheduler() override = default;

  /**
   * Non thread safe
   * @return
   */
  SchedulerThread *GetNextScheduler() noexcept {
    if (m_schedulers_.size() == 1) {
      m_schedulers_[0].get();
    }
    static size_t index = 0;
    index = (index + 1) % m_schedulers_.size();
    return m_schedulers_[index].get();
  }

  void Start(int timeout) override;

  bool IsRunning() noexcept override {
    if (!Scheduler::IsRunning()) {
      return false;
    }
    std::unique_lock<std::mutex> guard(m_mutex_);
    for (auto &ptr : m_schedulers_) {
      if (!ptr->IsRunning()) {
        return false;
      }
    }
    return true;
  }

  void Stop() noexcept override;

  MultiThreadScheduler &WithPreparation(std::function<void(void)> preparation) {
    m_preparation_ = std::move(preparation);
    return *this;
  }

  MultiThreadScheduler &WithCleanup(std::function<void(void)> cleanup) {
    m_cleanup_ = std::move(cleanup);
    return *this;
  }

 private:
  std::mutex m_mutex_;
  std::vector<std::unique_ptr<SchedulerThread>> m_schedulers_;
  std::function<void(void)> m_preparation_;
  std::function<void(void)> m_cleanup_;
};

}  // namespace clsn

#endif  // DEFTRPC_SCHEDULER_H
