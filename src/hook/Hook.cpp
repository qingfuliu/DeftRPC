//
// Created by lqf on 23-5-9.
//
#include "hook/Hook.h"
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <cerrno>
#include "common/common.h"
#include "common/this_thread.h"
#include "coroutine/Coroutine.h"
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

static thread_local bool enableHook = false;

namespace detail {
struct HookInitializer : public Singleton<HookInitializer> {
  friend class Singleton<HookInitializer>;

  SINGLETON_DEFINE(HookInitializer)

 public:
  void initHook() noexcept {
    clsn::Init<0>({clsn::CreateConsoleLogAppender("[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s", clsn::LogLevel::Debug)});

    CLSN_LOG_DEBUG << "===============Initialize hook====================";
    HOOK_RECOVER_SUMMERY(RECOVER_UNISTD_FUNC)
    CLSN_LOG_DEBUG << "===============Initialize hook completed====================";
  }
};

}  // namespace detail

static bool is_socket(int fd) noexcept {
  struct stat st {};
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
  if (!clsn::enableHook) {
    return socket_t(domain, type, protocol);
  }
  return socket_t(domain, type | SOCK_NONBLOCK | SOCK_CLOEXEC, protocol);
}

int connect(int fd, const struct sockaddr *address, socklen_t addressLen) {
  if (!clsn::enableHook || !clsn::is_socket(fd)) {
    return connect_t(fd, address, addressLen);
  }
  int res = -1;
  do {
    res = connect_t(fd, address, addressLen);
  } while (-1 == res && errno == EINTR);

  if (res == -1 && errno == EINPROGRESS) {
    clsn::this_thread::RegisterRead(fd, clsn::Coroutine::GetCurCoroutine());
    clsn::Coroutine::Yield();
    clsn::this_thread::CancelRegister(fd);
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
  if (!clsn::enableHook) {
    return accept4_t(sockFd, address, addressLen, flags);
  }

  int fd = -1;
  do {
    fd = accept4_t(sockFd, address, addressLen, flags);
  } while (-1 == fd && errno == EINTR);

  if (fd == -1 && errno == EAGAIN) {
    clsn::this_thread::RegisterRead(sockFd, clsn::Coroutine::GetCurCoroutine());
    clsn::Coroutine::Yield();
    clsn::this_thread::CancelRegister(sockFd);

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
  if (!clsn::enableHook || !clsn::is_socket(fd)) {
    return originFunc(fd, args...);
  }
  ssize_t n = 0;
  while ((n = originFunc(fd, args...)) == -1 && errno == EINTR) {
  }

  if (n == -1 && errno == EAGAIN) {
    if (clsn::kEvent::Read == event) {
      clsn::this_thread::RegisterRead(fd, clsn::Coroutine::GetCurCoroutine());
    } else {
      clsn::this_thread::RegisterWrite(fd, clsn::Coroutine::GetCurCoroutine());
    }
    do {
      clsn::Coroutine::Yield();
      n = originFunc(fd, args...);
    } while (n == -1 && (errno == EINTR || errno == EAGAIN));
    clsn::this_thread::CancelRegister(fd);
  }
  return n;
}

ssize_t read(int fd, void *buf, size_t count) { return IOBase(read_t, clsn::kEvent::Read, fd, buf, count); }

ssize_t write(int fd, const void *buf, size_t count) { return IOBase(write_t, clsn::kEvent::Write, fd, buf, count); }

int close(int fd) {
  if (clsn::enableHook) {
    clsn::this_thread::CancelRegister(fd);
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
  if (!clsn::enableHook) {
    return sleep_t(seconds);
  }
  clsn::Scheduler::GetThreadScheduler()->DoAfter(std::chrono::seconds{seconds}, clsn::Scheduler::GetCurCoroutine());
  clsn::Coroutine::Yield();
  return 0;
}

int usleep(useconds_t microsecond) {
  if (!clsn::enableHook) {
    return usleep_t(microsecond);
  }
  clsn::Scheduler::GetThreadScheduler()->DoAfter(std::chrono::microseconds{microsecond},
                                                 clsn::Scheduler::GetCurCoroutine());
  clsn::Coroutine::Yield();
  return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
  if (!clsn::enableHook) {
    return nanosleep_t(req, rem);
  }
  clsn::Scheduler::GetThreadScheduler()->DoAfter(std::chrono::nanoseconds{req->tv_sec * std::nano::den + req->tv_nsec},
                                                 clsn::Scheduler::GetCurCoroutine());
  clsn::Coroutine::Yield();
  return 0;
}

namespace clsn {}

bool IsEnableHook() noexcept { return clsn::enableHook; }

void EnableHook() noexcept {
  static clsn::detail::HookInitializer *p = nullptr;
  if (nullptr == p) {
    static std::mutex mutex;
    mutex.lock();
    if (nullptr == p) {
      p = &clsn::detail::HookInitializer::GetInstance();
      p->initHook();
    }
  }
  clsn::enableHook = true;
  assert(clsn::enableHook);
}

bool DisableEnableHook() noexcept { clsn::enableHook = false; }