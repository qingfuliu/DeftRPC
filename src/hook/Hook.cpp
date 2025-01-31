//
// Created by lqf on 23-5-9.
//
#include "hook/Hook.h"
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <cerrno>
#include "common/common.h"
#include "coroutine/Coroutine.h"
#include "coroutine/Scheduler.h"
#include "log/Log.h"

#define RECOVER_UNISTD_FUNC(FuncName)                                                           \
  FuncName##_t = reinterpret_cast<FuncName##_func>(dlsym(RTLD_NEXT, #FuncName));                \
  if (NULL == FuncName##_t) {                                                                   \
    CLSN_LOG_FATAL << "dlsym failed,funcName is: " #FuncName ",and the error is:" << dlerror(); \
  }

#define HOOK_RECOVER_SUMMERY(OPT) \
  OPT(socket)                     \
  OPT(connect)                    \
  OPT(accept)                     \
  OPT(accept4)                    \
  OPT(close)                      \
  OPT(read)                       \
  OPT(write)                      \
  OPT(readv)                      \
  OPT(writev)                     \
  OPT(sleep)                      \
  OPT(usleep)                     \
  OPT(nanosleep)

#define RECOVER_FUNC_DEFINE(FuncName) FuncName##_func FuncName##_t = nullptr;

HOOK_RECOVER_SUMMERY(RECOVER_FUNC_DEFINE)

#undef RECOVER_FUNC_DEFINE

namespace clsn {

static thread_local bool enable_hook = false;

namespace detail {
struct HookInitializer : public Singleton<HookInitializer> {
  friend class Singleton<HookInitializer>;

  SINGLETON_DEFINE(HookInitializer)

 public:
  void InitHook() noexcept {
    clsn::Init<0>({clsn::CreateConsoleLogAppender("[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s", clsn::LogLevel::Debug)});

    CLSN_LOG_DEBUG << "===============Initialize hook====================";
    HOOK_RECOVER_SUMMERY(RECOVER_UNISTD_FUNC)
    CLSN_LOG_DEBUG << "===============Initialize hook completed====================";
  }
};

}  // namespace detail

static bool IsSocket(int fd) noexcept {
  struct stat st{};
  int err = fstat(fd, &st);  // 获得文件的状态
  if (err < 0) {
    return false;
  }
  if (S_ISSOCK(st.st_mode)) {
    return true;
  }
  return false;
}

}  // namespace clsn

int socket(int domain, int type, int protocol) {
  if (!clsn::enable_hook) {
    return socket_t(domain, type, protocol);
  }
  return socket_t(domain, type | SOCK_NONBLOCK | SOCK_CLOEXEC, protocol);
}
// int pipe (int fd[2]){
//   if (!clsn::enable_hook) {
//     return pipe2()
//   }
// }
// int fd[2];
// char *p = "test for pipe\n";
//
// if (pipe(fd) == -1)

int connect(int fd, const struct sockaddr *address, socklen_t addressLen) {
  if (!clsn::enable_hook || !clsn::IsSocket(fd)) {
    return connect_t(fd, address, addressLen);
  }
  int res;
  do {
    res = connect_t(fd, address, addressLen);
  } while (-1 == res && errno == EINTR);

  if (res == -1 && errno == EINPROGRESS) {
    clsn::Scheduler::RegisterRead(fd, clsn::Scheduler::GetCurCoroutine());
    clsn::Scheduler::Yield();
    clsn::Scheduler::CancelRegister(fd);
    int error = 0;
    socklen_t len = sizeof(int);
    if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
      return -1;
    }
    if (0 == error) {
      return 0;
    }
    res = -1;
  }
  return res;
}

int accept4(int sockFd, struct sockaddr *address, socklen_t *addressLen, int flags) {
  if (!clsn::enable_hook) {
    return accept4_t(sockFd, address, addressLen, flags);
  }

  int fd;
  do {
    fd = accept4_t(sockFd, address, addressLen, flags);
  } while (-1 == fd && errno == EINTR);

  if (fd == -1 && errno == EAGAIN) {
    clsn::Scheduler::RegisterRead(sockFd, clsn::Scheduler::GetCurCoroutine());
    clsn::Scheduler::Yield();
    clsn::Scheduler::CancelRegister(sockFd);

    do {
      fd = accept4_t(sockFd, address, addressLen, flags);
    } while (-1 == fd && errno == EINTR);
  }
  return fd;
}

int accept(int sockFd, struct sockaddr *address, socklen_t *addressLen) {
  return accept4(sockFd, address, addressLen, SOCK_CLOEXEC | SOCK_NONBLOCK);
}

template <typename OriginFunc, typename... Args>
auto IOBase(OriginFunc originFunc, clsn::kEvent event, int fd, Args... args) {
  if (!clsn::enable_hook || !clsn::IsSocket(fd)) {
    return originFunc(fd, args...);
  }
  ssize_t n = 0;
  while ((n = originFunc(fd, args...)) == -1 && errno == EINTR) {
  }

  if (n == -1 && errno == EAGAIN) {
    if (clsn::kEvent::Read == event) {
      clsn::Scheduler::RegisterRead(fd, clsn::Scheduler::GetCurCoroutine());
    } else {
      clsn::Scheduler::RegisterWrite(fd, clsn::Scheduler::GetCurCoroutine());
    }
    do {
      clsn::Scheduler::Yield();
      n = originFunc(fd, args...);
    } while (n == -1 && (errno == EINTR || errno == EAGAIN));
    clsn::Scheduler::CancelRegister(fd);
  }
  return n;
}

ssize_t read(int fd, void *buf, size_t count) { return IOBase(read_t, clsn::kEvent::Read, fd, buf, count); }

ssize_t write(int fd, const void *buf, size_t count) { return IOBase(write_t, clsn::kEvent::Write, fd, buf, count); }

int close(int fd) {
  if (clsn::enable_hook) {
    clsn::Scheduler::CancelRegister(fd);
  }
  return close_t(fd);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
  return IOBase(readv_t, clsn::kEvent::Read, fd, iov, iovcnt);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
  return IOBase(writev_t, clsn::kEvent::Write, fd, iov, iovcnt);
}

unsigned int sleep(unsigned int seconds) {
  if (!clsn::enable_hook) {
    return sleep_t(seconds);
  }
  clsn::Scheduler::GetThreadScheduler()->DoAfter(std::chrono::seconds{seconds}, clsn::Scheduler::GetCurCoroutine());
  clsn::Scheduler::Yield();
  return 0;
}

int usleep(useconds_t microsecond) {
  if (!clsn::enable_hook) {
    return usleep_t(microsecond);
  }
  clsn::Scheduler::GetThreadScheduler()->DoAfter(std::chrono::microseconds{microsecond},
                                                 clsn::Scheduler::GetCurCoroutine());
  clsn::Scheduler::Yield();
  return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
  if (!clsn::enable_hook) {
    return nanosleep_t(req, rem);
  }
  clsn::Scheduler::GetThreadScheduler()->DoAfter(std::chrono::nanoseconds{req->tv_sec * std::nano::den + req->tv_nsec},
                                                 clsn::Scheduler::GetCurCoroutine());
  clsn::Scheduler::Yield();
  return 0;
}

namespace clsn {}

bool IsEnableHook() noexcept { return clsn::enable_hook; }

void EnableHook() noexcept {
  static clsn::detail::HookInitializer *p = nullptr;
  if (nullptr == p) {
    static std::mutex mutex;
    mutex.lock();
    if (nullptr == p) {
      p = &clsn::detail::HookInitializer::GetInstance();
      p->InitHook();
    }
  }
  clsn::enable_hook = true;
  assert(clsn::enable_hook);
}

void DisableEnableHook() noexcept { clsn::enable_hook = false; }
