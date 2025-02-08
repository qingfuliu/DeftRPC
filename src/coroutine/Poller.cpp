//
// Created by lqf on 23-4-28.
//
#include "coroutine/Poller.h"
#include <cerrno>
#include <cstring>

namespace clsn {

Poller::Poller() : m_epoll_fd_(epoll_create(KMAXEPOLLSIZE)) {}

Poller::~Poller() {
  if (m_epoll_fd_ != -1) {
    close(m_epoll_fd_);
  }
}

int Poller::EpollWait(std::vector<Runnable *> &tasks, int timeout) noexcept {
  tasks.clear();
  int res = epoll_wait(m_epoll_fd_, m_events_, KMAXEPOLLSIZE, timeout);
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
      tasks.push_back(&m_runnable_[m_events_[i].data.fd]);
    }
  }
  return res;
}

void Poller::CancelRegister(int fd) {
  if (fd < 0) {
    CLSN_LOG_ERROR << fd << " should not less then zero!";
    throw std::logic_error("fd should not less then zero!");
  }

  if (m_runnable_.size() > fd &&
      (0 != m_runnable_[fd].m_runnable_.index() || 0 != m_runnable_[fd].m_event_ || 0 != m_runnable_[fd].m_fd_)) {
    m_runnable_[fd] = Runnable{0, {}, 0};
    EpollCtl(fd, EPOLL_CTL_DEL, 0);
  }
}

void Poller::RegisterFd(Runnable runnable) {
  if (runnable.m_fd_ < 0) {
    CLSN_LOG_ERROR << runnable.m_fd_ << " should not less then zero!";
    throw std::logic_error("fd should not less then zero!");
  }

  int fd = runnable.m_fd_;
  uint32_t event = runnable.m_event_;
  int ctl = EPOLL_CTL_ADD;

  if (m_runnable_.size() <= fd) {
    do {
      m_runnable_.resize(m_runnable_.size() << 1);
    } while (m_runnable_.size() <= static_cast<size_t>(fd));
  }

  m_runnable_[fd] = runnable;

  if (-1 == EpollCtl(fd, ctl, event)) {
    CLSN_LOG_ERROR << "register m_socket_ failed!,m_socket_ is " << fd << " ,error is " << strerror(errno);
    throw std::logic_error("register m_socket_ failed!");
  }
}

int Poller::EpollCtl(int fd, int op, uint32_t event) const {
  epoll_event ev{};
  ev.data.fd = fd;
  ev.events = event;
  return epoll_ctl(m_epoll_fd_, op, fd, &ev);
}

}  // namespace clsn
