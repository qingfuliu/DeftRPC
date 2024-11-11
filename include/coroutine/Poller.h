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

namespace CLSN {
class Poller {
  friend std::unique_ptr<Poller> CreateNewPoller() noexcept;

 public:
  ~Poller() {
    if (epollFd != -1) {
      close(epollFd);
    }
  }

  int EpollWait(std::vector<FdDescriptor *> &tasks, int timeout = -1) noexcept;

  void RegisterRead(int fd, Task task) noexcept {
    if (fd < 0) {
      CLSN_LOG_ERROR << "sock should not less then zero!";
      return;
    }
    if (fdDescriptors.size() <= fd || fdDescriptors[fd].IsNoneEvent()) {
      FdDescriptor fdDescriptor(fd);
      fdDescriptor.SetRead(std::move(task));
      registerFd(std::move(fdDescriptor));
      return;
    }

    if (fdDescriptors[fd].IsReading()) {
      fdDescriptors[fd].SetRead(std::move(task));
      return;
    }

    CLSN_LOG_ERROR << "sock: sock ,already have event :" << fdDescriptors[fd].GetEvent() << ",but register read event";
  }

  void RegisterWrite(int fd, Task task) noexcept {
    if (fd < 0) {
      CLSN_LOG_ERROR << "sock should not less then zero!";
      return;
    }
    if (fdDescriptors.size() <= fd || fdDescriptors[fd].IsNoneEvent()) {
      FdDescriptor fdDescriptor(fd);
      fdDescriptor.SetWrite(std::move(task));
      registerFd(std::move(fdDescriptor));
      return;
    }

    if (fdDescriptors[fd].IsWrite()) {
      fdDescriptors[fd].SetWrite(std::move(task));
      return;
    }

    CLSN_LOG_ERROR << "sock: sock ,already have event :" << fdDescriptors[fd].GetEvent() << ",but register write event";
  }

  void CancelRegister(int fd) noexcept {
    if (fd < 0) {
      CLSN_LOG_ERROR << "sock should not less then zero!";
      return;
    }
    if (fdDescriptors.size() >= fd && !fdDescriptors[fd].IsNoneEvent()) {
      fdDescriptors[fd] = nullptr;
      epollCtl(fd, EPOLL_CTL_DEL, 0);
    }
  }

 private:
  Poller() : epollFd(epoll_create(MAXEPOLLSIZE)) {}

  void registerFd(FdDescriptor fdDescriptor) noexcept;

  int epollCtl(int fd, int op, uint32_t event) {
    epoll_event ev{};
    ev.data.fd = fd;
    ev.data.ptr = static_cast<void *>(&(*fdDescriptors.begin()) + fd);
    ev.events = event;
    return epoll_ctl(epollFd, op, fd, &ev);
  }

 private:
  int epollFd;
  std::vector<FdDescriptor> fdDescriptors;
  epoll_event events[MAXEPOLLSIZE]{};
};

inline std::unique_ptr<Poller> CreateNewPoller() noexcept { return std::unique_ptr<Poller>(new Poller); }

}  // namespace CLSN

#endif  // DEFTRPC_POLLER_H
