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
#include "common/task/Task.h"

namespace clsn {
class Poller {
  friend std::unique_ptr<Poller> CreateNewPoller() noexcept;
  friend class std::unique_ptr<Poller>;

 public:
  Poller() : m_epoll_fd_(epoll_create(MAXEPOLLSIZE)) {}

  ~Poller() {
    if (m_epoll_fd_ != -1) {
      close(m_epoll_fd_);
    }
  }

  int EpollWait(std::vector<FileDescriptor *> &tasks, int timeout = -1) noexcept;

  void RegisterRead(int fd, Task task) noexcept;

  void RegisterWrite(int fd, Task task) noexcept;

  void CancelRegister(int fd) noexcept;

 private:
  void RegisterFd(FileDescriptor fdDescriptor) noexcept;

  int EpollCtl(int fd, int op, uint32_t event);

 private:
  int m_epoll_fd_;
  std::vector<FileDescriptor> m_fds_{};
  epoll_event m_events_[MAXEPOLLSIZE]{};
};

inline std::unique_ptr<Poller> CreateNewPoller() noexcept { return std::make_unique<Poller>(); }

}  // namespace clsn

#endif  // DEFTRPC_POLLER_H
