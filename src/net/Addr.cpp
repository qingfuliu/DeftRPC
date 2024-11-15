//
// Created by lqf on 23-1-5.
//

#include "net/Addr.h"
#include <algorithm>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string_view>
#include "log/Log.h"

namespace clsn {

int Addr::FromString(const std::string &ip, uint16_t port, struct sockaddr_in *addr) {
  std::fill(reinterpret_cast<char *>(addr), reinterpret_cast<char *>(addr) + sizeof(sockaddr_in), 0);
  addr->sin_family = AF_INET;
  if (inet_pton(AF_INET, ip.c_str(), &addr->sin_addr) != 1) {
    CLSN_LOG_ERROR << "Addr FromString failed!";
    return -1;
  }
  addr->sin_port = htons(port);
  return 0;
}

int Addr::FromString(const std::string &ip, uint16_t port, struct sockaddr_in6 *addr6) {
  std::fill(reinterpret_cast<char *>(addr6), reinterpret_cast<char *>(addr6) + sizeof(sockaddr_in6), 0);

  addr6->sin6_family = AF_INET6;
  if (inet_pton(AF_INET6, ip.c_str(), &addr6->sin6_addr) != 1) {
    CLSN_LOG_ERROR << "Addr FromString failed!";
    return -1;
  }
  addr6->sin6_port = htons(port);
  return 0;
}

Addr::Addr(const std::string &ipPort, bool ipv4) noexcept {
  do {
    auto pos = static_cast<std::string::size_type>(ipPort.find(':', 0));
    if (pos == std::string::npos) {
      CLSN_LOG_ERROR << "ipPort is invalid!";
      break;
    }
    auto temp = std::string_view(ipPort.c_str() + pos + 1);
    auto port = static_cast<uint16_t>(atoi(temp.data()));
    if (ipv4) {
      m_address_ = sockaddr_in{};
      FromString(std::string(ipPort.begin(), ipPort.begin() + pos), port, &std::get<1>(m_address_));
    } else {
      m_address_ = sockaddr_in6{};
      FromString(std::string(ipPort.begin(), ipPort.begin() + pos), port, &std::get<0>(m_address_));
    }
  } while (false);
}

Addr::Addr(const std::string &ip, uint16_t port, bool ipv4) noexcept {
  if (ipv4) {
    FromString(ip, port, &std::get<1>(m_address_));
  } else {
    FromString(ip, port, &std::get<0>(m_address_));
  }
}

std::string Addr::ToString() const noexcept {
  char *buf;
  uint16_t port = 0;
  std::ostringstream oss;
  do {
    if (1 == m_address_.index()) {
      auto &addr = std::get<1>(m_address_);
      buf = static_cast<char *>(malloc(INET_ADDRSTRLEN));
      if (inet_ntop(AF_INET, &addr.sin_addr, buf, INET_ADDRSTRLEN) == nullptr) {
        // log
        CLSN_LOG_ERROR << "inet_ntop failed!! line: " << __LINE__ << " file: " << __FILE__;
        break;
      }
      oss << buf << ":" << ntohs(addr.sin_port);
    } else {
      auto &addr6 = std::get<0>(m_address_);
      buf = static_cast<char *>(malloc(INET6_ADDRSTRLEN));
      if (inet_ntop(AF_INET6, &addr6.sin6_addr, buf, INET6_ADDRSTRLEN) == nullptr) {
        // log
        CLSN_LOG_ERROR << "inet_ntop failed!! line: " << __LINE__ << " file: " << __FILE__;
        break;
      }
      oss << buf << ":" << ntohs(addr6.sin6_port);
    }
  } while (false);
  free(buf);
  return oss.str();
}

}  // namespace clsn
