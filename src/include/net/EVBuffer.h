//
// Created by lqf on 23-5-13.
//

#ifndef DEFTRPC_EVBUFFER_H
#define DEFTRPC_EVBUFFER_H

#include <sys/uio.h>
#include <memory>
#include <string>
#include <string_view>

namespace clsn {
/**
 * 不会持有任何空间,之持有指针
 * 在尾部写入不会分配空间
 */
class EVBuffer {
 public:
  EVBuffer() noexcept : m_iovecs_(std::make_unique<struct iovec[]>(3)) {}

  ~EVBuffer() noexcept = default;

  void Write(const char *data, size_t len) noexcept {
    if (len == 0) {
      return;
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
  }

  void Write(std::string &data) noexcept { Write(data.data(), data.size()); }

  void WritePrepend(const char *data, size_t len) noexcept {
    if (len == 0) {
      return;
    }
    if (m_begin_ != 0) {
      --m_begin_;
    } else if (m_end_ < m_size_) {
      std::copy(m_iovecs_.get() + m_begin_, m_iovecs_.get() + m_end_, m_iovecs_.get() + m_begin_ + 1);
      ++m_end_;
    } else {
      ++m_size_;
      auto temp = std::make_unique<struct iovec[]>(m_size_ + 1);
      std::copy(m_iovecs_.get(), m_iovecs_.get() + m_end_, temp.get() + 1);
      m_iovecs_.swap(temp);
      ++m_end_;
    }
    m_iovecs_[m_begin_].iov_base = const_cast<char *>(data);
    m_iovecs_[m_begin_].iov_len = len;
  }

  void WritePrepend(std::string &str) noexcept { WritePrepend(str.data(), str.size()); }

  void WritePrepend(std::string_view view) noexcept { WritePrepend(const_cast<char *>(view.data()), view.size()); }

  size_t ReadAll() noexcept {
    size_t res = 0;
    if (!IsEmpty()) {
      for (auto i = m_begin_; i != m_end_; i++) {
        res += m_iovecs_[i].iov_len;
      }
      m_begin_ = 0;
      m_end_ = 0;
    }
    return res;
  }

  int Read(int len) noexcept {
    if (IsEmpty()) {
      return 0;
    }
    int res = 0;
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
    return res;
  }

  [[nodiscard]] const struct iovec *GetIovecs() const noexcept {
    if (IsEmpty()) {
      return nullptr;
    }
    return m_iovecs_.get() + m_begin_;
  }

  [[nodiscard]] int GetIovecsSize() const noexcept { return static_cast<int>(m_end_ - m_begin_); }

  [[nodiscard]] bool IsEmpty() const noexcept { return m_begin_ == m_end_; }

  void Clear() noexcept {
    m_begin_ = 0;
    m_end_ = 0;
  }

  int WriteToFd(int fd) noexcept;

 private:
  std::unique_ptr<struct iovec[]> m_iovecs_;
  unsigned int m_size_{3};
  unsigned int m_begin_{0};
  unsigned int m_end_{0};
};

}  // namespace clsn

#endif  // DEFTRPC_EVBUFFER_H
