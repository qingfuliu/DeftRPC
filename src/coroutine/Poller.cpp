//
// Created by lqf on 23-4-28.
//
#include "coroutine/Poller.h"
#include <cerrno>
#include <cstring>

namespace CLSN {
int Poller::EpollWait(std::vector<FdDescriptor *> &tasks, int timeout) noexcept {
  tasks.clear();
  int res = epoll_wait(epollFd, events, MAXEPOLLSIZE, timeout);
  if (res == -1) {
    CLSN_LOG_FATAL << "epoll wait failed,error info is:" << strerror(errno);
  } else if (res == 0) {
    CLSN_LOG_WARNING << "nothing happened!";
  } else {
    CLSN_LOG_DEBUG << res << " happened!";
    for (int i = 0; i < res; i++) {
      if (events[i].data.ptr == nullptr) {
        continue;
      }
      tasks.push_back(static_cast<FdDescriptor *>(events[i].data.ptr));
      tasks.back()->setCurEvent(events[i].events);
    }
  }
  return res;
}

void Poller::registerFd(FdDescriptor fdDescriptor) noexcept {
  int fd = fdDescriptor.GetFd();
  int ctl;
  uint32_t event;
  if (fdDescriptors.size() <= fd) {
    if (fdDescriptors.empty()) {
      fdDescriptors.resize(1024);
    } else {
      do {
        fdDescriptors.resize(fdDescriptors.size() << 1);
      } while (fdDescriptors.size() <= fd);
    }
  }

  ctl = EPOLL_CTL_ADD;
  fdDescriptors[fd] = (std::move(fdDescriptor));
  event = fdDescriptors[fd].GetEvent();

  if (-1 == epollCtl(fd, ctl, event)) {
    CLSN_LOG_ERROR << "register sock failed!,sock is " << fd << " ,error is " << strerror(errno);
  }
}
}  // namespace CLSN
