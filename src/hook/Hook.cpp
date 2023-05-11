//
// Created by lqf on 23-5-9.
//
#include<dlfcn.h>
#include<cerrno>
#include <sys/uio.h>
#include <sys/stat.h>
#include "common/common.h"
#include"hook/Hook.h"
#include "log/Log.h"
#include "common/this_thread.h"
#include "coroutine/Coroutine.h"


#define RECOVER_UNISTD_FUNC(FuncName) \
    FuncName##_t=reinterpret_cast<FuncName##_func>(dlsym(RTLD_NEXT,#FuncName)); \
    if(NULL==FuncName##_t){           \
        CLSN_LOG_FATAL<<"dlsym failed,funcName is: "#FuncName",and the error is:"<<dlerror();           \
    }

#define HOOK_RECOVER_SUMMERY(OPT) \
        OPT(socket)                          \
        OPT(connect)             \
        OPT(accept)                          \
        OPT(accept4)                          \
        OPT(close)                         \
        OPT(read)                \
        OPT(write)                \
        OPT(readv)                \
        OPT(writev)               \
        OPT(sleep)                     \
        OPT(usleep)                     \
        OPT(nanosleep)                   \

#define RECOVER_FUNC_DEFINE(FuncName) \
        FuncName##_func FuncName##_t=nullptr;

HOOK_RECOVER_SUMMERY(RECOVER_FUNC_DEFINE)

#undef RECOVER_FUNC_DEFINE

namespace CLSN {

    static thread_local bool enableHook = false;

    namespace detail {
        struct HookInitializer : public Singleton<HookInitializer> {
            friend class Singleton<HookInitializer>;

            SINGLETON_DEFINE(HookInitializer)

        public:
            void initHook() noexcept {
                CLSN::init<0>({
                                      CLSN::createConsoleLogAppender(
                                              "[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s",
                                              CLSN::LogLevel::Debug)});

                CLSN_LOG_DEBUG << "===============Initialize hook====================";
                HOOK_RECOVER_SUMMERY(RECOVER_UNISTD_FUNC)
                CLSN_LOG_DEBUG << "===============Initialize hook completed====================";
            }
        };

    }

    static bool is_socket(int fd) noexcept {
        struct stat st{};
        int err = fstat(fd, &st);//获得文件的状态
        if (err < 0) {
            return false;
        }
        if (S_ISSOCK(st.st_mode)) {
            return true;
        }
        return false;
    }

}


int socket(int domain, int type, int protocol) {
    if (!CLSN::enableHook) {
        return socket_t(domain, type, protocol);
    }
    return socket_t(domain, type | SOCK_NONBLOCK | SOCK_CLOEXEC, protocol);
}


int connect(int fd, const struct sockaddr *address, socklen_t addressLen) {
    if (!CLSN::enableHook || !CLSN::is_socket(fd)) {
        return connect_t(fd, address, addressLen);
    }
    int res = -1;
    do {
        res = connect_t(fd, address, addressLen);
    } while (-1 == res && errno == EINTR);

    if (res == -1 && errno == EINPROGRESS) {
        CLSN::this_thread::RegisterRead(fd, CLSN::Coroutine::GetCurCoroutine());
        CLSN::Coroutine::Yield();
        CLSN::this_thread::CancelRegister(fd);
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
    if (!CLSN::enableHook) {
        return accept4_t(sockFd, address, addressLen, flags);
    }

    int fd = -1;
    do {
        fd = accept4_t(sockFd, address, addressLen, flags);
    } while (-1 == fd && errno == EINTR);

    if (fd == -1 && errno == EAGAIN) {
        CLSN::this_thread::RegisterRead(sockFd, CLSN::Coroutine::GetCurCoroutine());
        CLSN::Coroutine::Yield();
        CLSN::this_thread::CancelRegister(sockFd);

        do {
            fd = accept4_t(sockFd, address, addressLen, flags);
        } while (-1 == fd && errno == EINTR);

    }
    return fd;
}

int accept(int sockFd, struct sockaddr *address, socklen_t *addressLen) {
    return accept4(sockFd, address, addressLen, SOCK_CLOEXEC | SOCK_NONBLOCK);
}


template<typename OriginFunc, typename...Args>
auto IOBase(OriginFunc originFunc, CLSN::Event event, int fd, Args...args) {
    if (!CLSN::enableHook || !CLSN::is_socket(fd)) {
        return originFunc(fd, args...);
    }
    ssize_t n = 0;
    while ((n = originFunc(fd, args...)) == -1 && errno == EINTR) {}

    if (n == -1 && errno == EAGAIN) {
        if (CLSN::Event::Read == event) {
            CLSN::this_thread::RegisterRead(fd, CLSN::Coroutine::GetCurCoroutine());
        } else {
            CLSN::this_thread::RegisterWrite(fd, CLSN::Coroutine::GetCurCoroutine());
        }
        do {
            CLSN::Coroutine::Yield();
            n = originFunc(fd, args...);
        } while (n == -1 && (errno == EINTR || errno == EAGAIN));
        CLSN::this_thread::CancelRegister(fd);
    }
    return n;
}

ssize_t read(int fd, void *buf, size_t count) {
    return IOBase(read_t, CLSN::Event::Read, fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return IOBase(write_t, CLSN::Event::Write, fd, buf, count);
}

int close(int fd) {
    if (CLSN::enableHook) {
        CLSN::this_thread::CancelRegister(fd);
    }
    return close_t(fd);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return IOBase(readv_t, CLSN::Event::Read, fd, iov, iovcnt);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return IOBase(writev_t, CLSN::Event::Write, fd, iov, iovcnt);
}

unsigned int sleep(unsigned int seconds) {
    if (!CLSN::enableHook) {
        return sleep_t(seconds);
    }
    CLSN::Scheduler::GetThreadScheduler()->DoAfter(std::chrono::seconds{seconds},
                                                   CLSN::Scheduler::GetCurCoroutine());
    CLSN::Coroutine::Yield();
    return 0;
}

int usleep(useconds_t microsecond) {
    if (!CLSN::enableHook) {
        return usleep_t(microsecond);
    }
    CLSN::Scheduler::GetThreadScheduler()->DoAfter(std::chrono::microseconds{microsecond},
                                                   CLSN::Scheduler::GetCurCoroutine());
    CLSN::Coroutine::Yield();
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (!CLSN::enableHook) {
        return nanosleep_t(req, rem);
    }
    CLSN::Scheduler::GetThreadScheduler()->DoAfter(
            std::chrono::nanoseconds{req->tv_sec * std::nano::den + req->tv_nsec},
            CLSN::Scheduler::GetCurCoroutine());
    CLSN::Coroutine::Yield();
    return 0;
}

namespace CLSN {

}

bool Is_Enable_Hook() noexcept {
    return CLSN::enableHook;
}

void Enable_Hook() noexcept {
    static CLSN::detail::HookInitializer *p = nullptr;
    if (nullptr == p) {
        static std::mutex mutex;
        mutex.lock();
        if (nullptr == p) {
            p = &CLSN::detail::HookInitializer::getInstance();
            p->initHook();
        }
    }
    CLSN::enableHook = true;
    assert(CLSN::enableHook);
}

bool Disable_Enable_Hook() noexcept {
    CLSN::enableHook = false;
}