//
// Created by lqf on 23-4-28.
//

#ifndef DEFTRPC_POLLER_H
#define DEFTRPC_POLLER_H

#include <sys/epoll.h>
#include <unistd.h>
#include <memory>
#include <utility>
#include <vector>
#include "common/event/Event.h"
#include "common/task/Task.h"
#include "log/Log.h"

namespace clsn {

inline constexpr size_t KMAXEPOLLSIZE = 1024 << 1;

class Coroutine;

class Poller {
  friend std::unique_ptr<Poller> CreateNewPoller() noexcept;
  friend class std::unique_ptr<Poller>;

 public:
  Poller();

  ~Poller();

  int EpollWait(std::vector<Runnable *> &tasks, int timeout = -1) noexcept;

  template <class TaskOrCoroutine>
  void RegisterRead(int fd, TaskOrCoroutine taskOrCoroutine) {
    if (fd < 0) {
      CLSN_LOG_ERROR << fd << " should not less then zero!";
      throw std::logic_error("fd should not less then zero!");
    }

    if (m_runnable_.size() <= fd || (0 == m_runnable_[fd].m_runnable_.index() && 0 == m_runnable_[fd].m_event_)) {
      RegisterFd(Runnable{fd, taskOrCoroutine, static_cast<uint32_t>(kEvent::Read)});
      return;
    } else {
      CLSN_LOG_ERROR << fd << " register read twice!";
      throw std::logic_error("fd register read twice!");
    }
  }
  template <class TaskOrCoroutine>
  void RegisterWrite(int fd, TaskOrCoroutine taskOrCoroutine) {
    if (m_runnable_.size() <= fd || (0 == m_runnable_[fd].m_runnable_.index() && 0 == m_runnable_[fd].m_event_)) {
      RegisterFd(Runnable{fd, taskOrCoroutine, static_cast<uint32_t>(kEvent::Write)});
      return;
    } else {
      CLSN_LOG_ERROR << fd << " register read twice!";
      throw std::logic_error("fd register read twice!");
    }
  }

  void CancelRegister(int fd);

 private:
  void RegisterFd(Runnable runnable);

  [[nodiscard]] int EpollCtl(int fd, int op, uint32_t event) const;

 private:
  int m_epoll_fd_;
  std::vector<Runnable> m_runnable_{1024};
  epoll_event m_events_[KMAXEPOLLSIZE]{};
};

inline std::unique_ptr<Poller> CreateNewPoller() noexcept { return std::make_unique<Poller>(); }

}  // namespace clsn

#endif  // DEFTRPC_POLLER_H
