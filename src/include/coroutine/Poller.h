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


namespace clsn {

inline constexpr size_t KMAXEPOLLSIZE = 1024 << 1;

class Coroutine;

class Poller {
  friend std::unique_ptr<Poller> CreateNewPoller() noexcept;
  friend class std::unique_ptr<Poller>;

  struct CoroutineProxy {
    int m_fd_ = -1;
    Coroutine *m_coroutine_ = nullptr;
    uint32_t m_event_ = 0;
  };

 public:
  Poller();

  ~Poller();

  int EpollWait(std::vector<Coroutine *> &tasks, int timeout = -1) noexcept;

  void RegisterRead(int fd, Coroutine *coroutine);

  void RegisterWrite(int fd, Coroutine *coroutine);

  void CancelRegister(int fd);

 private:
  void RegisterFd(CoroutineProxy coroutineProxy);

  [[nodiscard]] int EpollCtl(int fd, int op, uint32_t event) const;

 private:
  int m_epoll_fd_;
  std::vector<CoroutineProxy> m_coroutine_{};
  epoll_event m_events_[KMAXEPOLLSIZE]{};
};

inline std::unique_ptr<Poller> CreateNewPoller() noexcept { return std::make_unique<Poller>(); }

}  // namespace clsn

#endif  // DEFTRPC_POLLER_H
