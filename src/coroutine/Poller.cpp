//
// Created by lqf on 23-4-28.
//
#include "coroutine/Poller.h"
#include <cerrno>
#include <cstring>

namespace clsn {
int Poller::EpollWait(std::vector<FileDescriptor *> &tasks, int timeout) noexcept {
  tasks.clear();
  int res = epoll_wait(m_epoll_fd_, m_events_, MAXEPOLLSIZE, timeout);
  if (res == -1) {
    CLSN_LOG_FATAL << "epoll wait failed,error info is:" << strerror(errno);
  } else if (res == 0) {
    CLSN_LOG_WARNING << "nothing happened!";
  } else {
    CLSN_LOG_DEBUG << res << " happened!";
    for (int i = 0; i < res; i++) {
      if (m_events_[i].data.ptr == nullptr) {
        continue;
      }
      tasks.push_back(static_cast<FileDescriptor *>(m_events_[i].data.ptr));
      tasks.back()->SetCurEvent(m_events_[i].events);
    }
  }
  return res;
}

void Poller::RegisterFd(FileDescriptor fdDescriptor) noexcept {
  int fd = fdDescriptor.GetFd();
  int ctl;
  uint32_t event;
  if (m_fds_.size() <= fd) {
    if (m_fds_.empty()) {
      m_fds_.resize(1024);
    } else {
      do {
        m_fds_.resize(m_fds_.size() << 1);
      } while (m_fds_.size() <= fd);
    }
  }

  ctl = EPOLL_CTL_ADD;
  m_fds_[fd] = (std::move(fdDescriptor));
  event = m_fds_[fd].GetEvent();

  if (-1 == EpollCtl(fd, ctl, event)) {
    CLSN_LOG_ERROR << "register m_socket_ failed!,m_socket_ is " << fd << " ,error is " << strerror(errno);
  }
}
}  // namespace clsn
