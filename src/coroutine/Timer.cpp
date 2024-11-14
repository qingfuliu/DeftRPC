//
// Created by lqf on 23-4-30.
//
#include "coroutine/Timer.h"
#include "log/Log.h"

namespace clsn {

static inline int createTimerFd() noexcept {
  int timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (-1 == timerFd) {
    CLSN_LOG_FATAL << "timer m_socket_ create failed,error is " << errno;
  }
  return timerFd;
}

static inline void readFromTimerFd(int timerFd) {
  uint64_t res;
  size_t size = read(timerFd, &res, sizeof res);
  if (size != sizeof res) {
    CLSN_LOG_FATAL << "Read From timer m_socket_ failed,error is " << errno;
  }
}

TimerQueue::TimerQueue()
    : Task([this]() { this->HandleExpireEvent(); }), m_timer_fd_(createTimerFd()), m_handling_event_(false) {
  Timer timer;
  m_timers_.insert(std::make_pair(timer.GetId(), timer));
}

void TimerQueue::CancelTimer(TimerIdType Id) noexcept {
  if (m_handling_event_) {
    m_cancel_timer_.insert(Id);
  } else {
    TimeStamp curHeader = GetEarliestTime();

    auto it = m_timers_.find(Id);
    if (it != m_timers_.end()) {
      m_active_timers_.erase(&it->second);
      m_timers_.erase(it);
    }

    TimeStamp afterHeader = GetEarliestTime();
    if (curHeader < afterHeader) {
      ResetTimerFd(m_timer_fd_, afterHeader);
    }
  }
  assert(m_active_timers_.size() + 1 == m_timers_.size());
}

/**
 *
 * 执行到期的timer
 * 将执行过的删除 并且保存
 * 如果是repeated、并且不再cancel里面 那么更新不删除 否则删除
 * 清空cacnle里面所包含的timer
 */
void TimerQueue::HandleExpireEvent() {
  m_handling_event_ = true;
  assert(m_active_timers_.size() + 1 == m_timers_.size());
  readFromTimerFd(m_timer_fd_);
  m_timers_[0].Reset();
  auto p = std::upper_bound(m_active_timers_.begin(), m_active_timers_.end(), &m_timers_[0], TimerComparer);

  /**
   * 处理活跃的
   */
  if (p != m_active_timers_.begin()) {
    std::vector<Timer *> activeTemp(m_active_timers_.begin(), p);
    for (auto it : activeTemp) {
      auto &timer = *it;
      timer();
      m_active_timers_.erase(it);
    }

    /**
     * 处理重复的
     */
    for (auto it : activeTemp) {
      auto &timer = *it;
      if (timer.GetRepeated() && m_cancel_timer_.end() == m_cancel_timer_.find(timer.GetId())) {
        timer.Reset();
        m_active_timers_.insert(it);
      } else {
        m_timers_.erase(it->GetId());
      }
    }
    /**
     *删除取消的
     */
    for (auto it : m_cancel_timer_) {
      auto t = m_timers_.find(it);
      if (t != m_timers_.end()) {
        m_active_timers_.erase(&t->second);
        m_timers_.erase(t);
      }
    }
    /**
     * 重置定时器时间
     */
    if (!m_active_timers_.empty()) {
      TimeStamp afterHeader = (*m_active_timers_.begin())->GetTimeStamp();
      ResetTimerFd(m_timer_fd_, afterHeader);
    }
    m_cancel_timer_.clear();
  }
  m_handling_event_ = false;
  assert(m_active_timers_.size() + 1 == m_timers_.size());
}
}  // namespace clsn