//
// Created by lqf on 23-4-30.
//
#include "coroutine/Timer.h"
#include "log/Log.h"

namespace CLSN {

static inline int createTimerFd() noexcept {
  int timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (-1 == timerFd) {
    CLSN_LOG_FATAL << "timer sock create failed,error is " << errno;
  }
  return timerFd;
}

static inline void readFromTimerFd(int timerFd) {
  uint64_t res;
  size_t size = read(timerFd, &res, sizeof res);
  if (size != sizeof res) {
    CLSN_LOG_FATAL << "Read From timer sock failed,error is " << errno;
  }
}

TimerQueue::TimerQueue()
    : Task([this]() { this->HandleExpireEvent(); }), timerFd(createTimerFd()), handlingEvent(false) {
  Timer timer;
  timers.insert(std::make_pair(timer.GetId(), timer));
}

void TimerQueue::CancelTimer(TimerIdType Id) noexcept {
  if (handlingEvent) {
    cancleTimer.insert(Id);
  } else {
    TimeStamp curHeader = GetEarliestTime();

    auto it = timers.find(Id);
    if (it != timers.end()) {
      activeTimers.erase(&it->second);
      timers.erase(it);
    }

    TimeStamp afterHeader = GetEarliestTime();
    if (curHeader < afterHeader) {
      resetTimerFd(timerFd, afterHeader);
    }
  }
  assert(activeTimers.size() + 1 == timers.size());
}

/**
 *
 * 执行到期的timer
 * 将执行过的删除 并且保存
 * 如果是repeated、并且不再cancel里面 那么更新不删除 否则删除
 * 清空cacnle里面所包含的timer
 */
void TimerQueue::HandleExpireEvent() {
  handlingEvent = true;
  assert(activeTimers.size() + 1 == timers.size());
  readFromTimerFd(timerFd);
  timers[0].Reset();
  auto p = std::upper_bound(activeTimers.begin(), activeTimers.end(), &timers[0], PtrCompare);

  /**
   * 处理活跃的
   */
  if (p != activeTimers.begin()) {
    std::vector<Timer *> activeTemp(activeTimers.begin(), p);
    for (auto it : activeTemp) {
      auto &timer = *it;
      timer();
      activeTimers.erase(it);
    }

    /**
     * 处理重复的
     */
    for (auto it : activeTemp) {
      auto &timer = *it;
      if (timer.GetRepeated() && cancleTimer.end() == cancleTimer.find(timer.GetId())) {
        timer.Reset();
        activeTimers.insert(it);
      } else {
        timers.erase(it->GetId());
      }
    }
    /**
     *删除取消的
     */
    for (auto it : cancleTimer) {
      auto t = timers.find(it);
      if (t != timers.end()) {
        activeTimers.erase(&t->second);
        timers.erase(t);
      }
    }
    /**
     * 重置定时器时间
     */
    if (!activeTimers.empty()) {
      TimeStamp afterHeader = (*activeTimers.begin())->GetTimeStamp();
      resetTimerFd(timerFd, afterHeader);
    }
    cancleTimer.clear();
  }
  handlingEvent = false;
  assert(activeTimers.size() + 1 == timers.size());
}
}  // namespace CLSN