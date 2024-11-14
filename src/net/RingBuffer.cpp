//
// Created by lqf on 23-1-11.
//

#include "net/RingBuffer.h"
#include <sys/uio.h>
#include <algorithm>
#include "common/common.h"

#define MAXPACKAGELEN (1024)
#define SingleExpansion (1024)
#define MultiplyExpansionLimit (1024)
#define DefaultBufferLen (1024)

namespace clsn {
RingBuffer::RingBuffer() noexcept
    : m_begin_(0),
      m_end_(0),
      m_size_(0),
      m_buffer_(std::vector<char>(DefaultBufferLen)), m_temp_capacity_(0), m_temp_(nullptr) {}

PackageLengthType RingBuffer::GetPackageLength() const noexcept {
  if (m_size_ <= sizeof(PackageLengthType)) {
    return -1;
  }

  int tailLen = static_cast<int>(std::distance(m_buffer_.begin() + m_begin_, m_buffer_.end()));

  if (tailLen >= sizeof(PackageLengthType)) {
    const PackageLengthType *res = reinterpret_cast<const PackageLengthType *>(&(*(m_buffer_.begin() + m_begin_)));
    return ntohl(*res);
  }
  PackageLengthType res = 0;
  std::copy(m_buffer_.begin() + m_begin_, m_buffer_.end(), reinterpret_cast<char *>(&res));
  std::copy(m_buffer_.begin(), m_buffer_.begin() + sizeof(PackageLengthType) - tailLen,
            reinterpret_cast<char *>(&res) + tailLen);
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
    bool tailLen = GetTailContentLen();
    tailLen = (tailLen > len) ? len : tailLen;
    std::copy(m_buffer_.begin() + m_begin_, m_buffer_.begin() + m_begin_ + tailLen, buf);
    if (len > tailLen) {
      std::copy(m_buffer_.begin(), m_buffer_.begin() + len - tailLen, buf + tailLen);
    }
  } while (0);
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
    } else
      m_temp_capacity_ <<= 1;
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
    int tailCapacity = GetTailSpaceLen();
    tailCapacity = tailCapacity > len ? len : tailCapacity;
    std::copy(buf, buf + tailCapacity, m_buffer_.begin() + this->m_end_);
    if (tailCapacity < len) {
      std::copy(buf + tailCapacity, buf + len, m_buffer_.begin());
    }
  } while (0);
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
  int newCapacity = static_cast<int>(this->m_buffer_.capacity());

  do {
    if (this->m_buffer_.capacity() <= MultiplyExpansionLimit) {
      newCapacity <<= 1;
      continue;
    }
    newCapacity += SingleExpansion;
  } while (newCapacity < needle);

  std::vector<char> tempBuffer;
  tempBuffer.swap(m_buffer_);
  m_buffer_.resize(newCapacity);
  do {
    if (this->m_begin_ <= this->m_end_) {
      std::copy(tempBuffer.begin() + m_begin_, tempBuffer.begin() + m_end_, m_buffer_.begin());
      break;
    }
    std::copy(tempBuffer.begin() + m_begin_, tempBuffer.end(), m_buffer_.begin());
    std::copy(tempBuffer.begin(), tempBuffer.begin() + m_end_, m_buffer_.begin() + GetTailContentLen());
  } while (0);
  this->m_begin_ = 0;
  this->m_end_ = this->m_size_;
}

int RingBuffer::ReadFromFd(int fd) noexcept {
  char expansionSpace[MAXPACKAGELEN];
  struct iovec iov[3];
  int iovLen;
  int writable = GetWritableCapacity();
  if (GetTailSpaceLen() > 0) {
    iov[0].iov_base = &(*m_buffer_.begin()) + m_end_;
    iov[0].iov_len = GetTailSpaceLen();
    iov[1].iov_base = &(*m_buffer_.begin());
    iov[1].iov_len = m_begin_;
    iov[2].iov_base = expansionSpace;
    iov[2].iov_len = MAXPACKAGELEN;
    iovLen = 3;
  } else {
    iov[0].iov_base = &(*m_buffer_.begin()) + m_end_;
    iov[0].iov_len = writable;
    iov[1].iov_base = expansionSpace;
    iov[1].iov_len = MAXPACKAGELEN;
    iovLen = 2;
  }
  if (writable >= MAXPACKAGELEN) {
    --iovLen;
  }

  auto resSize = static_cast<int>(readv(fd, iov, iovLen));
  if (resSize < 0) {
    return -1;
  }
  if (resSize > writable) {
    UpdateAfterWrite(writable);
    Append(expansionSpace, resSize - writable);
  } else {
    UpdateAfterWrite(resSize);
  }
  return resSize;
}

int RingBuffer::WriteToFd(int fd) noexcept {
  struct iovec iov[2];
  int iovLen;
  int tailContentLen = GetTailContentLen();

  if (tailContentLen > 0) {
    iov[0].iov_base = &(*m_buffer_.begin()) + m_begin_;
    iov[0].iov_len = tailContentLen;
    iov[1].iov_base = &(*m_buffer_.begin());
    iov[1].iov_len = m_size_ - tailContentLen;
    iovLen = 2;
  } else {
    iov[0].iov_base = &(*m_buffer_.begin()) + m_begin_;
    iov[0].iov_len = m_size_;
    iovLen = 1;
  }

  auto writeSize = static_cast<int>(writev(fd, iov, iovLen));

  if (writeSize < 0) {
    return -1;
  }
  UpdateAfterRead(writeSize);
  return writeSize;
}

}  // namespace clsn