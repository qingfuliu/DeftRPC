//
// Created by lqf on 23-1-5.
//

#include "net/Socket.h"
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <cerrno>
#include "log/Log.h"
#include "net/Addr.h"

namespace clsn {

int MAX_LISTEN_COUNT = 100;

int CreateNoBlockSocket() noexcept {
  int fd = socket(AF_INET, SOCK_CLOEXEC | SOCK_NONBLOCK | SOCK_STREAM, IPPROTO_TCP);
  if (fd < 0) {
    // log
    return -1;
  }
  return fd;
}

int CreateBlockSocket() noexcept {
  int fd = socket(AF_INET, SOCK_CLOEXEC | SOCK_STREAM, IPPROTO_TCP);
  if (fd < 0) {
    // log
    return -1;
  }
  return fd;
}

int Socket::Listen() const noexcept { return ::listen(fd, MAX_LISTEN_COUNT); }

int Socket::Connect(const Addr *addr) const noexcept {
  int res;
  res = ::connect(fd, addr->getSockAddr(), addr->getSockAddrSize());
  return res;
}

int Socket::Accept(Addr *addr) const noexcept {
  socklen_t len = sizeof(sockaddr_in6);
  int newFd;
  newFd = ::accept4(fd, addr->getSockAddr(), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
  return newFd;
}

int Socket::Bind(const Addr *addr) const noexcept { return ::bind(fd, addr->getSockAddr(), addr->getSockAddrSize()); }

int Socket::SetTcpKeepAlive(bool val) const noexcept {
  int flag = val ? 1 : 0;
  if (0 != ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(int))) {
    // log
    CLSN_LOG_WARNING << "SetTcpKeepAlive failed!! line: " << __LINE__ << " file: " << __FILE__;
    return -1;
  }
  return 0;
}

int Socket::SetTCPNoDelay(bool val) const noexcept {
  int flag = val ? 1 : 0;
  if (0 != ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int))) {
    // log
    CLSN_LOG_WARNING << "SetTCPNoDelay failed!! line: " << __LINE__ << " file: " << __FILE__;

    return -1;
  }
  return 0;
}

int Socket::SetReusePort(bool val) const noexcept {
  int flag = val ? 1 : 0;
  if (0 != ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(int))) {
    // log
    CLSN_LOG_WARNING << "SetReusePort failed!! line: " << __LINE__ << " file: " << __FILE__;

    return -1;
  }
  return 0;
}

int Socket::SetReuseAddr(bool val) const noexcept {
  int flag = val ? 1 : 0;
  if (0 != ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int))) {
    // log
    CLSN_LOG_WARNING << "SetReuseAddr failed!! line: " << __LINE__ << " file: " << __FILE__;

    return -1;
  }
  return 0;
}

int Socket::SetNoBlock(bool val) const noexcept {
  int oldSocketFlag = fcntl(fd, F_GETFL, 0);
  int newSocketFlag = oldSocketFlag;
  if (val) {
    newSocketFlag |= O_NONBLOCK;
  } else {
    newSocketFlag |= (~O_NONBLOCK);
  }
  if (fcntl(fd, F_SETFL, newSocketFlag) == -1) {
    ::close(fd);
    CLSN_LOG_WARNING << "set socket to nonblock error.";
    return -1;
  }
  return 0;
}

int Socket::SetReadTimeout(int val) const noexcept {
  struct timeval timeout {
    val, 0
  };
  return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeval));
}

int Socket::SetWriteTimeout(int val) const noexcept {
  struct timeval timeout {
    val, 0
  };
  return setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeval));
}

int Socket::Read(char *buf, size_t len) const noexcept {
  int size = static_cast<int>(::read(fd, buf, len));
  if (size <= 0) {
    // log
    CLSN_LOG_WARNING << "Read failed!! line: " << __LINE__ << " file: " << __FILE__;
  }
  return size;
}

[[maybe_unused]] int Socket::Readv(const struct iovec *buf, size_t len) const noexcept {
  int size = static_cast<int>(::readv(fd, buf, static_cast<int>(len)));
  if (size <= 0) {
    // log
    CLSN_LOG_WARNING << "Readv failed!! line: " << __LINE__ << " file: " << __FILE__;
  }
  return size;
}

int Socket::Write(char *buf, size_t len) const noexcept {
  int size = static_cast<int>(::write(fd, buf, len));
  if (size <= 0) {
    // log
    CLSN_LOG_WARNING << "Write failed!! line: " << __LINE__ << " file: " << __FILE__;
  }
  return size;
}

int Socket::Close() const noexcept {
  int flag = ::close(fd);
  if (flag < 0) {
    // log
    CLSN_LOG_WARNING << "Close failed!! line: " << __LINE__ << " file: " << __FILE__;
  }
  return flag;
}
}  // namespace clsn
