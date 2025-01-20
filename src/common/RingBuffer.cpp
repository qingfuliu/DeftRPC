//
// Created by lqf on 23-1-11.
//

#include "net/RingBuffer.h"
#include <sys/uio.h>
#include <algorithm>
#include "common/common.h"

namespace clsn {

PackageLengthType RingBuffer::GetPackageLength() const noexcept {
  if (m_size_ <= sizeof(PackageLengthType)) {
    return -1;
  }

  int tail_len = static_cast<int>(std::distance(m_buffer_.begin() + m_begin_, m_buffer_.end()));

  if (tail_len >= sizeof(PackageLengthType)) {
    const PackageLengthType *res = reinterpret_cast<const PackageLengthType *>(&(*(m_buffer_.begin() + m_begin_)));
    return ntohl(*res);
  }
  PackageLengthType res = 0;
  std::copy(m_buffer_.begin() + m_begin_, m_buffer_.end(), reinterpret_cast<char *>(&res));
  std::copy(m_buffer_.begin(), m_buffer_.begin() + sizeof(PackageLengthType) - tail_len,
            reinterpret_cast<char *>(&res) + tail_len);
  return ntohl(res);
}

int RingBuffer::GetTailSpaceLen() const noexcept {
  if (m_begin_ > m_end_ || IsFull()) {
    return 0;
  }
  return static_cast<int>(m_buffer_.capacity()) - this->m_end_;
}

int RingBuffer::GetTailContentLen() const noexcept {
  if (m_begin_ < m_end_ || IsEmpty()) {
    return 0;
  }
  return static_cast<int>(m_buffer_.capacity()) /**/ - m_begin_;
}

int RingBuffer::GetWritableCapacity() const noexcept { return static_cast<int>(m_buffer_.capacity()) - this->m_size_; }

// int RingBuffer::ReadAll(char*buf,int*len){
//     return Read(buf,size);
// }

int RingBuffer::Read(char *buf, int len) noexcept {
  if (len > m_size_) {
    len = m_size_;
  }
  if (len == 0) {
    return 0;
  }
  do {
    if (m_begin_ < m_end_) {
      std::copy(m_buffer_.begin() + m_begin_, m_buffer_.begin() + m_begin_ + len, buf);
      break;
    }
    int tail_len = GetTailContentLen();
    tail_len = (tail_len > len) ? len : tail_len;
    std::copy(m_buffer_.begin() + m_begin_, m_buffer_.begin() + m_begin_ + tail_len, buf);
    if (len > tail_len) {
      std::copy(m_buffer_.begin(), m_buffer_.begin() + len - tail_len, buf + tail_len);
    }
  } while (false);
  UpdateAfterRead(len);
  return len;
}

char *RingBuffer::ReadAll(int *len) noexcept {
  if (m_size_ == 0) {
    if (len != nullptr) {
      *len = 0;
    }
    return nullptr;
  }
  if (m_temp_capacity_ < m_size_) {
    m_temp_capacity_ = m_size_;
    m_temp_ = std::unique_ptr<char[]>(new char[m_temp_capacity_]);
  }
  if (len != nullptr) {
    *len = Read(m_temp_.get(), m_size_);
  } else {
    Read(m_temp_.get(), m_size_);
  }
  return m_temp_.get();
}

//    char*
char *RingBuffer::Read(int size, int *len) noexcept {
  if (size <= 0) {
    return nullptr;
  }
  while (m_temp_capacity_ < size) {
    if (m_temp_capacity_ == 0) {
      m_temp_capacity_ = 1024;
    } else {
      m_temp_capacity_ <<= 1;
    }
    m_temp_.reset(new char[m_temp_capacity_]);
  }
  size = Read(m_temp_.get(), size);
  if (len != nullptr) {
    *len = size;
  }
  return m_temp_.get();
}

int RingBuffer::Write(const char *buf, int len) noexcept {
  EnableWritableSpace(len);
  do {
    if (this->m_begin_ >= this->m_end_) {
      std::copy(buf, buf + len, m_buffer_.begin() + this->m_end_);
      break;
    }
    int tail_capacity = GetTailSpaceLen();
    tail_capacity = tail_capacity > len ? len : tail_capacity;
    std::copy(buf, buf + tail_capacity, m_buffer_.begin() + this->m_end_);
    if (tail_capacity < len) {
      std::copy(buf + tail_capacity, buf + len, m_buffer_.begin());
    }
  } while (false);
  UpdateAfterWrite(len);
  return len;
}

void RingBuffer::UpdateAfterRead(int len) noexcept {
  if (this->m_size_ == len) {
    this->m_size_ = 0;
    m_begin_ = 0;
    m_end_ = 0;
  } else {
    this->m_size_ -= len;
    m_begin_ = (m_begin_ + len) % static_cast<int>(this->m_buffer_.capacity());
  }
}

void RingBuffer::UpdateAfterWrite(int len) noexcept {
  if (len == 0) {
    return;
  }
  this->m_size_ += len;
  if (m_begin_ >= m_end_) {
    m_end_ += len;
    return;
  }
  m_end_ = (m_end_ + len) % static_cast<int>(this->m_buffer_.capacity());
}

void RingBuffer::EnableWritableSpace(int targetSize) noexcept {
  if (this->m_buffer_.capacity() >= targetSize + this->m_size_) {
    return;
  }

  int needle = this->m_size_ + targetSize;
  int new_capacity = static_cast<int>(this->m_buffer_.capacity());

  do {
    if (this->m_buffer_.capacity() <= MULTIPLY_EXPANSION_LIMIT) {
      new_capacity <<= 1;
      continue;
    }
    new_capacity += SINGLE_EXPANSION;
  } while (new_capacity < needle);

  std::vector<char> temp_buffer;
  temp_buffer.swap(m_buffer_);
  m_buffer_.resize(new_capacity);
  do {
    if (this->m_begin_ <= this->m_end_) {
      std::copy(temp_buffer.begin() + m_begin_, temp_buffer.begin() + m_end_, m_buffer_.begin());
      break;
    }
    std::copy(temp_buffer.begin() + m_begin_, temp_buffer.end(), m_buffer_.begin());
    std::copy(temp_buffer.begin(), temp_buffer.begin() + m_end_, m_buffer_.begin() + GetTailContentLen());
  } while (false);
  this->m_begin_ = 0;
  this->m_end_ = this->m_size_;
}

int RingBuffer::ReadFromFd(int fd) noexcept {
  char expansion_space[MAX_PACKAGE_LEN];
  struct iovec iov[3];
  int iov_len;
  int writable = GetWritableCapacity();
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

  auto res_size = static_cast<int>(readv(fd, iov, iov_len));
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

//int RingBuffer::WriteToFd(int fd) noexcept {
//  struct iovec iov[2];
//  int iov_len;
//  int tail_content_len = GetTailContentLen();
//
//  if (tail_content_len > 0) {
//    iov[0].iov_base = &(*m_buffer_.begin()) + m_begin_;
//    iov[0].iov_len = tail_content_len;
//    iov[1].iov_base = &(*m_buffer_.begin());
//    iov[1].iov_len = m_size_ - tail_content_len;
//    iov_len = 2;
//  } else {
//    iov[0].iov_base = &(*m_buffer_.begin()) + m_begin_;
//    iov[0].iov_len = m_size_;
//    iov_len = 1;
//  }
//
//  auto write_size = static_cast<int>(writev(fd, iov, iov_len));
//
//  if (write_size < 0) {
//    return -1;
//  }
//  UpdateAfterRead(write_size);
//  return write_size;
//}

}  // namespace clsn
