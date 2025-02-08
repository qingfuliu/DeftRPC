//
// Created by lqf on 23-4-28.
//
#include "coroutine/Poller.h"
#include <cerrno>
#include <cstring>
#include "common/event/Event.h"
#include "coroutine/Coroutine.h"
#include "log/Log.h"
namespace clsn {

Poller::Poller() : m_epoll_fd_(epoll_create(KMAXEPOLLSIZE)) {}

Poller::~Poller() {
  if (m_epoll_fd_ != -1) {
    close(m_epoll_fd_);
  }
}

int Poller::EpollWait(std::vector<Coroutine *> &tasks, int timeout) noexcept {
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
      tasks.push_back(static_cast<Coroutine *>(m_events_[i].data.ptr));
    }
  }
  return res;
}

void Poller::RegisterRead(int fd, Coroutine *coroutine) {
  if (fd < 0) {
    CLSN_LOG_ERROR << fd << " should not less then zero!";
    throw std::logic_error("fd should not less then zero!");
  }

  if (m_coroutine_.size() <= fd || (nullptr == m_coroutine_[fd].m_coroutine_ && 0 == m_coroutine_[fd].m_event_)) {
    RegisterFd(CoroutineProxy{fd, coroutine, static_cast<uint32_t>(kEvent::Read)});
    return;
  } else {
    CLSN_LOG_ERROR << fd << " register read twice!";
    throw std::logic_error("fd register read twice!");
  }
}

void Poller::RegisterWrite(int fd, Coroutine *coroutine) {
  if (m_coroutine_.size() <= fd || (nullptr == m_coroutine_[fd].m_coroutine_ && 0 == m_coroutine_[fd].m_event_)) {
    RegisterFd(CoroutineProxy{fd, coroutine, static_cast<uint32_t>(kEvent::Write)});
    return;
  } else {
    CLSN_LOG_ERROR << fd << " register read twice!";
    throw std::logic_error("fd register read twice!");
  }
}

void Poller::CancelRegister(int fd) {
  if (fd < 0) {
    CLSN_LOG_ERROR << fd << " should not less then zero!";
    throw std::logic_error("fd should not less then zero!");
  }

  if (m_coroutine_.size() >= fd &&
      (nullptr == m_coroutine_[fd].m_coroutine_ || 0 != m_coroutine_[fd].m_event_ || 0 != m_coroutine_[fd].m_fd_)) {
    m_coroutine_[fd] = CoroutineProxy{0, nullptr, 0};
    EpollCtl(fd, EPOLL_CTL_DEL, 0);
  }
}

void Poller::RegisterFd(CoroutineProxy coroutineProxy) {
  if (coroutineProxy.m_fd_ < 0) {
    CLSN_LOG_ERROR << coroutineProxy.m_fd_ << " should not less then zero!";
    throw std::logic_error("fd should not less then zero!");
  }

  int fd = coroutineProxy.m_fd_;
  uint32_t event = coroutineProxy.m_event_;
  int ctl = EPOLL_CTL_ADD;

  if (m_coroutine_.size() <= fd) {
    do {
      m_coroutine_.resize(m_coroutine_.size() << 1);
    } while (m_coroutine_.size() <= static_cast<size_t>(fd));
  }

  m_coroutine_[fd] = coroutineProxy;

  if (-1 == EpollCtl(fd, ctl, event)) {
    CLSN_LOG_ERROR << "register m_socket_ failed!,m_socket_ is " << fd << " ,error is " << strerror(errno);
    throw std::logic_error("register m_socket_ failed!");
  }
}

int Poller::EpollCtl(int fd, int op, uint32_t event) const {
  epoll_event ev{};
  ev.data.fd = fd;
  ev.data.ptr = static_cast<void *>(m_coroutine_[fd].m_coroutine_);
  ev.events = event;
  return epoll_ctl(m_epoll_fd_, op, fd, &ev);
}

}  // namespace clsn
