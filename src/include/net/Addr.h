//
// Created by lqf on 23-1-5.
//

#ifndef MCLOUDDISK_ADDR_H
#define MCLOUDDISK_ADDR_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <variant>

namespace clsn {
class Addr {
  static int FromString(const std::string &ip, uint16_t port, struct sockaddr_in *);

  static int FromString(const std::string &ip, uint16_t port, struct sockaddr_in6 *);

 public:
  Addr() noexcept = default;

  explicit Addr(const std::string &ipPort, bool ipv4 = true) noexcept;

  Addr(const std::string &ip, uint16_t port, bool ipv4 = true) noexcept;

  explicit Addr(const sockaddr_in *addr) noexcept { this->m_address_ = *addr; }

  explicit Addr(const sockaddr_in6 *addr6) { this->m_address_ = *addr6; }

  Addr(const Addr &addr) noexcept { this->m_address_ = addr.m_address_; }

  [[nodiscard]] sa_family_t GetFamily() const noexcept {
    switch (m_address_.index()) {
      case 0:
        return std::get<0>(m_address_).sin6_family;
      default:
        return std::get<1>(m_address_).sin_family;
    }
  }

  [[nodiscard]] size_t GetSockAddrSize() const noexcept {
    switch (m_address_.index()) {
      case 0:
        return sizeof(sockaddr_in6);
      default:
        return sizeof(sockaddr_in);
    }
  }

  sockaddr *GetSockAddr() noexcept { return reinterpret_cast<sockaddr *>(&m_address_); }

  [[nodiscard]] const sockaddr *GetSockAddr() const noexcept { return reinterpret_cast<const sockaddr *>(&m_address_); }

  [[nodiscard]] std::string ToString() const noexcept;

 private:
  std::variant<sockaddr_in6, sockaddr_in> m_address_{};
};
}  // namespace clsn

#endif  // MCLOUDDISK_ADDR_H
