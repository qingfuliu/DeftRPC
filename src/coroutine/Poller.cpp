//
// Created by lqf on 23-4-28.
//
#include "coroutine/Poller.h"
#include <cerrno>
#include <cstring>
#include "log/Log.h"

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

void Poller::RegisterRead(int fd, Task task) noexcept {
  if (fd < 0) {
    CLSN_LOG_ERROR << "fd should not less then zero!";
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

  CLSN_LOG_ERROR << "m_socket_: m_socket_ ,already have event :" << m_fds_[fd].GetEvent() << ",but register read event";
}

void Poller::RegisterWrite(int fd, Task task) noexcept {
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

void Poller::CancelRegister(int fd) noexcept {
  if (fd < 0) {
    CLSN_LOG_ERROR << "m_socket_ should not less then zero!";
    return;
  }
  if (m_fds_.size() >= fd && !m_fds_[fd].IsNoneEvent()) {
    m_fds_[fd] = nullptr;
    EpollCtl(fd, EPOLL_CTL_DEL, 0);
  }
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

int Poller::EpollCtl(int fd, int op, uint32_t event) {
  epoll_event ev{};
  ev.data.fd = fd;
  ev.data.ptr = static_cast<void *>(&(*m_fds_.begin()) + fd);
  ev.events = event;
  return epoll_ctl(m_epoll_fd_, op, fd, &ev);
}

}  // namespace clsn
