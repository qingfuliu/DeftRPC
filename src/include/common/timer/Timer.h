//
// Created by lqf on 23-4-29.
//

#ifndef DEFTRPC_TIMER_H
#define DEFTRPC_TIMER_H

#include <sys/timerfd.h>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <functional>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "common/task/Task.h"
#include "log/Log.h"

namespace clsn {

template <typename Clock, typename Dur>
using TimePoint = typename std::chrono::time_point<Clock, Dur>;

class TimeStamp {
 public:
  using ClockType = std::chrono::system_clock;

  using DurationType = std::chrono::nanoseconds;

  using TimeType = TimePoint<ClockType, DurationType>;

  TimeStamp() = default;

  explicit TimeStamp(DurationType interval) noexcept
      : m_time_(std::chrono::time_point_cast<DurationType>(ClockType::now() + interval)) {}

  explicit TimeStamp(TimeType time_) noexcept : m_time_(time_) {}

  TimeStamp(const TimeStamp &) noexcept = default;

  TimeStamp &operator=(const TimeStamp &) noexcept = default;

  template <class Clock, class Duration>
  explicit TimeStamp(TimePoint<Clock, Duration> time_) noexcept
      : m_time_(std::chrono::time_point_cast<DurationType>(time_)) {}

  template <typename Rep, typename Period>
  void ResetWithDuration(const std::chrono::duration<Rep, Period> &duration) noexcept {
    m_time_ = std::chrono::time_point_cast<DurationType>(ClockType::now()) + duration;
  }

  bool operator<(const TimeStamp &other) const noexcept { return m_time_ < other.m_time_; }

  [[nodiscard]] timespec HowMuchFromNow() const noexcept {
    struct timespec res{};
    auto duration = m_time_ - std::chrono::time_point_cast<DurationType>(ClockType::now());
    res.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    res.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() % std::nano::den;
    return res;
  }

  static TimeStamp Max() noexcept { return TimeStamp{TimeType::max()}; }

  static TimeStamp Now() noexcept { return TimeStamp{ClockType::now()}; }

 private:
  TimeType m_time_;
};

static inline int CreateTimerFd() noexcept {
  int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (-1 == timer_fd) {
    CLSN_LOG_FATAL << "timer m_socket_ create failed,error is " << errno;
  }
  return timer_fd;
}

static inline void ResetTimerFd(int timerFd, const TimeStamp &time) {
  struct itimerspec new_time{};
  struct itimerspec old_time{};
  new_time.it_value = time.HowMuchFromNow();
  int res = timerfd_settime(timerFd, 0, &new_time, &old_time);
  if (-1 == res) {
    CLSN_LOG_FATAL << "timer fd set time failed,error is " << errno;
  }
}

using TimerIdType = int64_t;

class Timer {
 public:
  template <typename Rep, typename Period>
  explicit Timer(const std::chrono::duration<Rep, Period> &interval_, Task task_, bool repeated_ = false) noexcept
      : m_task_(std::move(task_)),
        m_repeat_(repeated_),
        m_interval_(std::chrono::duration_cast<TimeStamp::DurationType>(interval_)),
        m_expire_(m_interval_),
        m_id_(id.fetch_add(1, std::memory_order_release)) {}

  Timer() noexcept : m_id_(0), m_interval_(0), m_expire_(TimeStamp::ClockType::now()), m_repeat_(false) {}

  Timer(const Timer &timer) = default;

  ~Timer() = default;

  Timer &operator=(const Timer &) = default;

  void Reset() noexcept { m_expire_.ResetWithDuration(m_interval_); }

  [[nodiscard]] TimerIdType GetId() const noexcept { return m_id_; }

  [[nodiscard]] auto &GetTimeStamp() const noexcept { return m_expire_; }

  [[nodiscard]] auto GetRepeated() const noexcept { return m_repeat_; }

  bool operator<(const Timer &other) const noexcept { return m_expire_ < other.m_expire_; }

  bool operator==(const Timer &other) const noexcept { return other.m_id_ == m_id_; }

  void operator()() { m_task_(); }

 private:
  Task m_task_;
  bool m_repeat_;
  TimeStamp::DurationType m_interval_;
  TimeStamp m_expire_;
  TimerIdType m_id_;
  static inline std::atomic<TimerIdType> id{1};
};

inline bool TimerComparer(const Timer *a, const Timer *b) { return *a < *b; }

class TimerQueue {
 public:
  TimerQueue() = default;

  ~TimerQueue() {
    if (-1 != m_timer_fd_) {
      close(m_timer_fd_);
    }
  }

  template <typename Rep, typename Period>
  TimerIdType AddTimer(const std::chrono::duration<Rep, Period> &interval, Task task, bool repeated = false) noexcept {
    TimeStamp cur_header = GetEarliestTime();

    Timer timer = Timer(interval, std::move(task), repeated);
    auto pair = m_timers_.insert(std::make_pair(timer.GetId(), timer));
    m_active_timers_.insert(&pair.first->second);
    TimeStamp after_header = GetEarliestTime();

    if (after_header < cur_header) {
      ResetTimerFd(m_timer_fd_, after_header);
    }
    return pair.first->second.GetId();
  }

  void CancelTimer(TimerIdType Id) noexcept;

  [[nodiscard]] int GetTimerFd() const noexcept { return m_timer_fd_; }

  void HandleExpireEvent();

  void operator()() { HandleExpireEvent(); }

 private:
  [[nodiscard]] TimeStamp GetEarliestTime() const noexcept {
    TimeStamp earliest = TimeStamp::Max();
    if (!m_active_timers_.empty()) {
      earliest = (*m_active_timers_.begin())->GetTimeStamp();
    }
    return earliest;
  }

 private:
  int m_timer_fd_{CreateTimerFd()};
  bool m_handling_event_{false};
  std::unordered_map<TimerIdType, Timer> m_timers_;
  std::unordered_set<TimerIdType> m_cancel_timer_;
  std::set<Timer *, decltype(&TimerComparer)> m_active_timers_{&TimerComparer};
};

inline std::unique_ptr<TimerQueue> CreateNewTimerQueue() { return std::make_unique<TimerQueue>(); }

}  // namespace clsn

#endif  // DEFTRPC_TIMER_H
