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
#include "Task.h"

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

  void RegisterRead(int fd, Task task) noexcept {
    if (fd < 0) {
      CLSN_LOG_ERROR << "m_socket_ should not less then zero!";
      return;
    }
    if (m_fds_.size() <= fd || m_fds_[fd].IsNoneEvent()) {
      FileDescriptor file_descriptor(fd);
      file_descriptor.SetRead(std::move(task));
      RegisterFd(std::move(file_descriptor));
      return;
    }

    if (m_fds_[fd].IsReading()) {
      m_fds_[fd].SetRead(std::move(task));
      return;
    }

    CLSN_LOG_ERROR << "m_socket_: m_socket_ ,already have event :" << m_fds_[fd].GetEvent()
                   << ",but register read event";
  }

  void RegisterWrite(int fd, Task task) noexcept {
    if (fd < 0) {
      CLSN_LOG_ERROR << "m_socket_ should not less then zero!";
      return;
    }
    if (m_fds_.size() <= fd || m_fds_[fd].IsNoneEvent()) {
      FileDescriptor file_descriptor(fd);
      file_descriptor.SetWrite(std::move(task));
      RegisterFd(std::move(file_descriptor));
      return;
    }

    if (m_fds_[fd].IsWrite()) {
      m_fds_[fd].SetWrite(std::move(task));
      return;
    }

    CLSN_LOG_ERROR << "m_socket_: m_socket_ ,already have event :" << m_fds_[fd].GetEvent()
                   << ",but register write event";
  }

  void CancelRegister(int fd) noexcept {
    if (fd < 0) {
      CLSN_LOG_ERROR << "m_socket_ should not less then zero!";
      return;
    }
    if (m_fds_.size() >= fd && !m_fds_[fd].IsNoneEvent()) {
      m_fds_[fd] = nullptr;
      EpollCtl(fd, EPOLL_CTL_DEL, 0);
    }
  }

 private:

  void RegisterFd(FileDescriptor fdDescriptor) noexcept;

  int EpollCtl(int fd, int op, uint32_t event) {
    epoll_event ev{};
    ev.data.fd = fd;
    ev.data.ptr = static_cast<void *>(&(*m_fds_.begin()) + fd);
    ev.events = event;
    return epoll_ctl(m_epoll_fd_, op, fd, &ev);
  }

 private:
  int m_epoll_fd_;
  std::vector<FileDescriptor> m_fds_{};
  epoll_event m_events_[MAXEPOLLSIZE]{};
};

inline std::unique_ptr<Poller> CreateNewPoller() noexcept { return std::make_unique<Poller>(); }

}  // namespace clsn

#endif  // DEFTRPC_POLLER_H
