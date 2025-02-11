//
// Created by lqf on 23-1-5.
//

#ifndef MCLOUDDISK_SOCKET_H
#define MCLOUDDISK_SOCKET_H

#include <arpa/inet.h>
#include <unistd.h>

namespace clsn {
class Addr;

int CreateNoBlockSocket() noexcept;

int CreateBlockSocket() noexcept;

class Socket {
 public:
  explicit Socket() = default;

  explicit Socket(int fd) : m_fd_(fd) {}

  ~Socket() = default;

  [[nodiscard]] int GetFd() const noexcept { return m_fd_; }

  int Listen() const noexcept;

  int Connect(const Addr *addr) const noexcept;

  int Accept(Addr *addr) const noexcept;

  int Bind(const Addr *addr) const noexcept;

  int SetTcpKeepAlive(bool val) const noexcept;

  int SetTCPNoDelay(bool val) const noexcept;

  int SetReusePort(bool val) const noexcept;

  int SetReuseAddr(bool val) const noexcept;

  int SetNoBlock(bool val) const noexcept;

  int SetReadTimeout(int val) const noexcept;

  int SetWriteTimeout(int val) const noexcept;

  int Read(char *buf, size_t len) const noexcept;

  [[maybe_unused]] int Readv(const struct iovec *buf, size_t len) const noexcept;

  int Write(char *buf, size_t len) const noexcept;

  int Close() noexcept;

 private:
  int m_fd_;
};
}  // namespace clsn

#endif  // MCLOUDDISK_SOCKET_H
