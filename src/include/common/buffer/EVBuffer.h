//
// Created by lqf on 23-5-13.
//

#ifndef DEFTRPC_EVBUFFER_H
#define DEFTRPC_EVBUFFER_H

#include <sys/uio.h>
#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include "Buffer.h"

namespace clsn {
/**
 * 不会持有任何空间,之持有指针
 * 在尾部写入不会分配空间
 */
class EVBuffer : public Buffer {
 public:
  EVBuffer() noexcept : m_iovecs_(std::make_unique<struct iovec[]>(3)) {}

  ~EVBuffer() noexcept override = default;

  std::uint32_t Write(const char *data, std::uint32_t len) override {
    if (len == 0) {
      return 0;
    }
    if (m_end_ >= m_size_) {
      if (m_begin_ != 0) {
        std::copy(m_iovecs_.get() + m_begin_, m_iovecs_.get() + m_end_, m_iovecs_.get() + m_begin_ - 1);
        --m_end_;
      } else {
        ++m_size_;
        auto temp = std::make_unique<struct iovec[]>(m_size_);
        std::copy(m_iovecs_.get(), m_iovecs_.get() + m_end_, temp.get());
        m_iovecs_.swap(temp);
      }
    }

    m_iovecs_[m_end_].iov_len = len;
    m_iovecs_[m_end_].iov_base = const_cast<char *>(data);
    ++m_end_;
    return len;
  }

  void Write(std::string &data) noexcept { Write(data.data(), data.size()); }

  std::string Read(std::uint32_t len) noexcept override {
    if (Empty()) {
      return {};
    }
    std::uint32_t res = 0;
    while (m_begin_ != m_end_) {
      if (len >= m_iovecs_[m_begin_].iov_len) {
        res += static_cast<int>(m_iovecs_[m_begin_].iov_len);
        len -= static_cast<int>(m_iovecs_[m_begin_].iov_len);
        m_begin_++;
      } else {
        m_iovecs_[m_begin_].iov_base = static_cast<char *>(m_iovecs_[m_begin_].iov_base) + len;
        m_iovecs_[m_begin_].iov_len -= len;
        res += len;
        break;
      }
    }
    if (m_begin_ == m_end_) {
      m_begin_ = 0;
      m_end_ = 0;
    }
    m_size_ = m_end_ - m_begin_;
    return {};
  }

  std::string Peek(std::uint32_t len) noexcept override {
    if (Empty()) {
      return {};
    }
    std::uint32_t res = 0;
    while (m_begin_ != m_end_) {
      if (len >= m_iovecs_[m_begin_].iov_len) {
        res += static_cast<int>(m_iovecs_[m_begin_].iov_len);
        len -= static_cast<int>(m_iovecs_[m_begin_].iov_len);
        m_begin_++;
      } else {
        m_iovecs_[m_begin_].iov_base = static_cast<char *>(m_iovecs_[m_begin_].iov_base) + len;
        m_iovecs_[m_begin_].iov_len -= len;
        res += len;
        break;
      }
    }
    if (m_begin_ == m_end_) {
      m_begin_ = 0;
      m_end_ = 0;
    }
    m_size_ = m_end_ - m_begin_;
    return {};
  }

  [[nodiscard]] bool Empty() const noexcept override { return m_begin_ == m_end_; }

  [[nodiscard]] std::uint32_t Size() const noexcept override { return 0; }

  std::uint32_t Capacity() const noexcept override { return 0; }

  void Clear() noexcept {
    m_begin_ = 0;
    m_end_ = 0;
  }

  int FetchDataFromFd(int fd) override { return 0; }

  int FlushDataToFd(int fd) noexcept override;

 private:
  std::unique_ptr<struct iovec[]> m_iovecs_;
  unsigned int m_size_{3};
  unsigned int m_begin_{0};
  unsigned int m_end_{0};
};

}  // namespace clsn

#endif  // DEFTRPC_EVBUFFER_H
