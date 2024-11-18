//
// Created by lqf on 23-4-30.
//
#include "coroutine/Timer.h"
#include "log/Log.h"

namespace clsn {

static inline void ReadFromTimerFd(int timerFd) {
  uint64_t res;
  size_t size = read(timerFd, &res, sizeof res);
  if (size != sizeof res) {
    CLSN_LOG_FATAL << "Read From timer m_socket_ failed,error is " << errno;
  }
}

TimerQueue::TimerQueue() : Task([this]() { this->HandleExpireEvent(); }) {
  Timer timer;
  m_timers_.insert(std::make_pair(timer.GetId(), timer));
}

void TimerQueue::CancelTimer(TimerIdType Id) noexcept {
  if (m_handling_event_) {
    m_cancel_timer_.insert(Id);
  } else {
    TimeStamp cur_header = GetEarliestTime();

    auto it = m_timers_.find(Id);
    if (it != m_timers_.end()) {
      m_active_timers_.erase(&it->second);
      m_timers_.erase(it);
    }

    TimeStamp after_header = GetEarliestTime();
    if (cur_header < after_header) {
      ResetTimerFd(m_timer_fd_, after_header);
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
  ReadFromTimerFd(m_timer_fd_);
  m_timers_[0].Reset();
  auto p = std::upper_bound(m_active_timers_.begin(), m_active_timers_.end(), &m_timers_[0], TimerComparer);

  /**
   * 处理活跃的
   */
  if (p != m_active_timers_.begin()) {
    std::vector<Timer *> active_temp(m_active_timers_.begin(), p);
    for (auto it : active_temp) {
      auto &timer = *it;
      timer();
      m_active_timers_.erase(it);
    }

    /**
     * 处理重复的
     */
    for (auto it : active_temp) {
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
      TimeStamp after_header = (*m_active_timers_.begin())->GetTimeStamp();
      ResetTimerFd(m_timer_fd_, after_header);
    }
    m_cancel_timer_.clear();
  }
  m_handling_event_ = false;
  assert(m_active_timers_.size() + 1 == m_timers_.size());
}
}  // namespace clsn
