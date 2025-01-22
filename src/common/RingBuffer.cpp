//
// Created by lqf on 23-1-11.
//

#include "common/buffer/RingBuffer.h"
#include <sys/uio.h>
#include <algorithm>
#include <string>
#include "common/common.h"

namespace clsn {

std::string RingBuffer::Read(std::uint32_t len) {
  if (len == 0) {
    return {};
  }
  if (len > m_size_) {
    len = m_size_;
  }
  std::string data(len, '\0');
  do {
    if (m_begin_ < m_end_) {
      std::copy(m_buffer_.begin() + m_begin_, m_buffer_.begin() + m_begin_ + len, data.begin());
      break;
    }
    std::uint32_t tail_len = GetTailContentLen();
    tail_len = (tail_len > len) ? len : tail_len;
    std::copy(m_buffer_.begin() + m_begin_, m_buffer_.begin() + m_begin_ + tail_len, data.begin());
    if (len > tail_len) {
      std::copy(m_buffer_.begin(), m_buffer_.begin() + len - tail_len, data.begin() + tail_len);
    }
  } while (false);
  UpdateAfterRead(len);
  return data;
}

std::string RingBuffer::Peek(std::uint32_t len) {
  if (len == 0) {
    return {};
  }
  if (len > m_size_) {
    len = m_size_;
  }
  std::string data(len, '\0');
  do {
    if (m_begin_ < m_end_) {
      std::copy(m_buffer_.begin() + m_begin_, m_buffer_.begin() + m_begin_ + len, data.begin());
      break;
    }
    std::uint32_t tail_len = GetTailContentLen();
    tail_len = (tail_len > len) ? len : tail_len;
    std::copy(m_buffer_.begin() + m_begin_, m_buffer_.begin() + m_begin_ + tail_len, data.begin());
    if (len > tail_len) {
      std::copy(m_buffer_.begin(), m_buffer_.begin() + len - tail_len, data.begin() + tail_len);
    }
  } while (false);
  return data;
}

std::uint32_t RingBuffer::GetTailSpaceLen() const noexcept {
  if (m_begin_ > m_end_ || IsFull()) {
    return 0;
  }
  return static_cast<std::uint32_t>(m_buffer_.capacity()) - this->m_end_;
}

std::uint32_t RingBuffer::GetTailContentLen() const noexcept {
  if (m_begin_ < m_end_ || Empty()) {
    return 0;
  }
  return static_cast<std::uint32_t>(m_buffer_.capacity()) /**/ - m_begin_;
}

std::uint32_t RingBuffer::GetWritableCapacity() const noexcept {
  return static_cast<std::uint32_t>(m_buffer_.capacity()) - this->m_size_;
}

std::uint32_t RingBuffer::Write(const char *data, std::uint32_t len) {
  EnableWritableSpace(len);
  do {
    if (this->m_begin_ >= this->m_end_) {
      std::copy(data, data + len, m_buffer_.begin() + this->m_end_);
      break;
    }
    std::uint32_t tail_capacity = GetTailSpaceLen();
    tail_capacity = tail_capacity > len ? len : tail_capacity;
    std::copy(data, data + tail_capacity, m_buffer_.begin() + this->m_end_);
    if (tail_capacity < len) {
      std::copy(data + tail_capacity, data + len, m_buffer_.begin());
    }
  } while (false);
  UpdateAfterWrite(len);
  return len;
}

void RingBuffer::UpdateAfterRead(std::uint32_t len) noexcept {
  if (this->m_size_ == len) {
    m_size_ = 0;
    m_begin_ = 0;
    m_end_ = 0;
  } else {
    this->m_size_ -= len;
    m_begin_ = (m_begin_ + len) % static_cast<std::uint32_t>(this->m_buffer_.capacity());
  }
}

void RingBuffer::UpdateAfterWrite(std::uint32_t len) noexcept {
  if (len == 0) {
    return;
  }
  this->m_size_ += len;
  if (m_begin_ >= m_end_) {
    m_end_ += len;
    return;
  }
  m_end_ = (m_end_ + len) % static_cast<std::uint32_t>(this->m_buffer_.capacity());
}

void RingBuffer::EnableWritableSpace(std::uint32_t requiredFreeCapacity) noexcept {
  if (this->m_buffer_.capacity() >= requiredFreeCapacity + this->m_size_) {
    return;
  }

  std::uint32_t needle = this->m_size_ + requiredFreeCapacity;
  std::uint32_t new_capacity = this->m_buffer_.capacity();

  do {
    if (this->m_buffer_.capacity() <= MULTIPLY_EXPANSION_LIMIT) {
      new_capacity <<= 1;
      continue;
    }
    new_capacity += SINGLE_EXPANSION;
  } while (new_capacity < needle);

  std::vector<char> temp_buffer(new_capacity);
  do {
    if (this->m_begin_ <= this->m_end_) {
      std::copy(m_buffer_.begin() + m_begin_, m_buffer_.begin() + m_end_, temp_buffer.begin());
      break;
    }
    std::copy(m_buffer_.begin() + m_begin_, m_buffer_.end(), temp_buffer.begin());
    std::copy(m_buffer_.begin(), m_buffer_.begin() + m_end_, temp_buffer.begin() + GetTailContentLen());
  } while (false);
  temp_buffer.swap(m_buffer_);
  this->m_begin_ = 0;
  this->m_end_ = this->m_size_;
}

int RingBuffer::FetchDataFromFd(int fd) {
  char expansion_space[MAX_PACKAGE_LEN];
  struct iovec iov[3];
  int iov_len;
  std::uint32_t writable = GetWritableCapacity();
  if (GetTailSpaceLen() > 0) {
    iov[0].iov_base = &(*m_buffer_.begin()) + m_end_;
    iov[0].iov_len = GetTailSpaceLen();
    iov[1].iov_base = &(*m_buffer_.begin());
    iov[1].iov_len = m_begin_;
    iov[2].iov_base = expansion_space;
    iov[2].iov_len = MAX_PACKAGE_LEN;
    iov_len = 3;
  } else {
    iov[0].iov_base = &(*m_buffer_.begin()) + m_end_;
    iov[0].iov_len = writable;
    iov[1].iov_base = expansion_space;
    iov[1].iov_len = MAX_PACKAGE_LEN;
    iov_len = 2;
  }
  if (writable >= MAX_PACKAGE_LEN) {
    --iov_len;
  }

  int res_size = static_cast<int>(readv(fd, iov, iov_len));
  if (res_size < 0) {
    return -1;
  }
  if (res_size > writable) {
    UpdateAfterWrite(writable);
    Append(expansion_space, res_size - writable);
  } else {
    UpdateAfterWrite(res_size);
  }
  return res_size;
}

int RingBuffer::FlushDataToFd(int fd) noexcept {
  struct iovec iov[2];
  std::uint32_t iov_len;
  std::uint32_t tail_content_len = GetTailContentLen();

  if (tail_content_len > 0) {
    iov[0].iov_base = &(*m_buffer_.begin()) + m_begin_;
    iov[0].iov_len = tail_content_len;
    iov[1].iov_base = &(*m_buffer_.begin());
    iov[1].iov_len = m_size_ - tail_content_len;
    iov_len = 2;
  } else {
    iov[0].iov_base = &(*m_buffer_.begin()) + m_begin_;
    iov[0].iov_len = m_size_;
    iov_len = 1;
  }

  int write_size = writev(fd, iov, iov_len);

  if (write_size < 0) {
    return -1;
  }
  UpdateAfterRead(write_size);
  return write_size;
}

}  // namespace clsn
