//
// Created by lqf on 23-1-5.
//

#ifndef MCLOUDDISK_SOCKET_H
#define MCLOUDDISK_SOCKET_H

#include<unistd.h>
#include<arpa/inet.h>

namespace CLSN {
    class Addr;

    int CreateNoBlockSocket() noexcept;

    int CreateBlockSocket() noexcept;

    class Socket {
    public:
        explicit Socket() = default;

        explicit Socket(int fd) : fd(fd) {}

        ~Socket() = default;

        [[nodiscard]] int getFd() const noexcept {
            return fd;
        }

        int Listen() const noexcept;

        int Connect(const Addr *addr) const noexcept;

        int Accept(Addr *addr) const noexcept;

        int Bind(const Addr *addr) const noexcept;

        int SetTcpKeepAlive(bool) const noexcept;

        int SetTCPNoDelay(bool val) const noexcept;

        int SetReusePort(bool val) const noexcept;

        int SetReuseAddr(bool val) const noexcept;

        int SetNoBlock(bool val) const noexcept;

        int SetReadTimeout(int val) const noexcept;

        int SetWriteTimeout(int val) const noexcept;

        int Read(char *buf, size_t len) const noexcept;

        [[maybe_unused]] int Readv(const struct iovec *buf, size_t len) const noexcept;

        int Write(char *buf, size_t) const noexcept;

        int Close() const noexcept;

    private:
        int fd;
    };
}


#endif //MCLOUDDISK_SOCKET_H
