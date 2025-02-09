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
    for (int i = 0; i < res; i++) {
      auto runnable = &m_runnable_context_[m_events_[i].data.fd];
      if (runnable->IsWriting()) {
        tasks.push_back(&(runnable->m_write_callback_));
      }
      if (runnable->IsReading()) {
        tasks.push_back(&(runnable->m_read_callback_));
      }
    }
  }
  return res;
}

void Poller::CancelRegister(int fd) {
  if (fd < 0) {
    CLSN_LOG_ERROR << fd << " should not less then zero!";
    throw std::logic_error("fd should not less then zero!");
  }

  if (m_runnable_context_.size() > fd &&
      (0 != m_runnable_context_[fd].m_write_callback_.index() ||
       0 != m_runnable_context_[fd].m_read_callback_.index() || 0 != m_runnable_context_[fd].m_event_ ||
       0 != m_runnable_context_[fd].m_fd_)) {
    m_runnable_context_[fd] = RunnableContext{0, {}, {}, 0};
    EpollCtl(fd, EPOLL_CTL_DEL, 0);
  }
}

void Poller::RegisterFd(RunnableContext runnable) {
  int fd = runnable.m_fd_;
  int ctl;
  uint32_t event;

  if (fd < 0) {
    CLSN_LOG_ERROR << fd << " should not less then zero!";
    throw std::logic_error("fd should not less then zero!");
  }

  if (m_runnable_context_.size() <= fd) {
    do {
      m_runnable_context_.resize(m_runnable_context_.size() << 1);
    } while (m_runnable_context_.size() <= static_cast<size_t>(fd));
  }

  if (m_runnable_context_[fd].IsNoneEvent()) {
    ctl = EPOLL_CTL_ADD;
    m_runnable_context_[fd] = runnable;
  } else {
    ctl = EPOLL_CTL_MOD;
    m_runnable_context_[fd].m_event_ |= runnable.m_event_;
    if (runnable.IsReading()) {
      m_runnable_context_[fd].m_read_callback_ = runnable.m_read_callback_;
    } else if (runnable.IsWriting()) {
      m_runnable_context_[fd].m_write_callback_ = runnable.m_write_callback_;
    }
  }
  event = runnable.m_event_ = m_runnable_context_[fd].m_event_;

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
