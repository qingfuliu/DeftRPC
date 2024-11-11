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

namespace CLSN {
RingBuffer::RingBuffer() noexcept
    : begin(0), end(0), size(0), buffer(std::vector<char>(DefaultBufferLen)), tempCapacity(0), temp(nullptr) {}

PackageLengthType RingBuffer::GetPackageLength() const noexcept {
  if (size <= sizeof(PackageLengthType)) {
    return -1;
  }

  int tailLen = static_cast<int>(std::distance(buffer.begin() + begin, buffer.end()));

  if (tailLen >= sizeof(PackageLengthType)) {
    const PackageLengthType *res = reinterpret_cast<const PackageLengthType *>(&(*(buffer.begin() + begin)));
    return ntohl(*res);
  }
  PackageLengthType res = 0;
  std::copy(buffer.begin() + begin, buffer.end(), reinterpret_cast<char *>(&res));
  std::copy(buffer.begin(), buffer.begin() + sizeof(PackageLengthType) - tailLen,
            reinterpret_cast<char *>(&res) + tailLen);
  return ntohl(res);
}

int RingBuffer::getTailSpaceLen() const noexcept {
  if (begin > end || isFull()) {
    return 0;
  }
  return static_cast<int>(buffer.capacity()) - this->end;
}

int RingBuffer::getTailContentLen() const noexcept {
  if (begin < end || IsEmpty()) {
    return 0;
  }
  return static_cast<int>(buffer.capacity()) /**/ - begin;
}

int RingBuffer::getWritableCapacity() const noexcept { return static_cast<int>(buffer.capacity()) - this->size; }

// int RingBuffer::ReadAll(char*buf,int*len){
//     return Read(buf,size);
// }

int RingBuffer::Read(char *buf, int len) noexcept {
  if (len > size) {
    len = size;
  }
  if (len == 0) {
    return 0;
  }
  do {
    if (begin < end) {
      std::copy(buffer.begin() + begin, buffer.begin() + begin + len, buf);
      break;
    }
    bool tailLen = getTailContentLen();
    tailLen = (tailLen > len) ? len : tailLen;
    std::copy(buffer.begin() + begin, buffer.begin() + begin + tailLen, buf);
    if (len > tailLen) {
      std::copy(buffer.begin(), buffer.begin() + len - tailLen, buf + tailLen);
    }
  } while (0);
  updateAfterRead(len);
  return len;
}

char *RingBuffer::ReadAll(int *len) noexcept {
  if (size == 0) {
    if (len != nullptr) {
      *len = 0;
    }
    return nullptr;
  }
  if (tempCapacity < size) {
    tempCapacity = size;
    temp = std::unique_ptr<char[]>(new char[tempCapacity]);
  }
  if (len != nullptr) {
    *len = Read(temp.get(), size);
  } else {
    Read(temp.get(), size);
  }
  return temp.get();
}

//    char*
char *RingBuffer::Read(int size, int *len) noexcept {
  if (size <= 0) {
    return nullptr;
  }
  while (tempCapacity < size) {
    if (tempCapacity == 0) {
      tempCapacity = 1024;
    } else
      tempCapacity <<= 1;
    temp.reset(new char[tempCapacity]);
  }
  size = Read(temp.get(), size);
  if (len != nullptr) {
    *len = size;
  }
  return temp.get();
}

int RingBuffer::Write(const char *buf, int len) noexcept {
  enableWritableSpace(len);
  do {
    if (this->begin >= this->end) {
      std::copy(buf, buf + len, buffer.begin() + this->end);
      break;
    }
    int tailCapacity = getTailSpaceLen();
    tailCapacity = tailCapacity > len ? len : tailCapacity;
    std::copy(buf, buf + tailCapacity, buffer.begin() + this->end);
    if (tailCapacity < len) {
      std::copy(buf + tailCapacity, buf + len, buffer.begin());
    }
  } while (0);
  updateAfterWrite(len);
  return len;
}

void RingBuffer::updateAfterRead(int len) noexcept {
  if (this->size == len) {
    this->size = 0;
    begin = 0;
    end = 0;
  } else {
    this->size -= len;
    begin = (begin + len) % static_cast<int>(this->buffer.capacity());
  }
}

void RingBuffer::updateAfterWrite(int len) noexcept {
  if (len == 0) {
    return;
  }
  this->size += len;
  if (begin >= end) {
    end += len;
    return;
  }
  end = (end + len) % static_cast<int>(this->buffer.capacity());
}

void RingBuffer::enableWritableSpace(int targetSize) noexcept {
  if (this->buffer.capacity() >= targetSize + this->size) {
    return;
  }

  int needle = this->size + targetSize;
  int newCapacity = static_cast<int>(this->buffer.capacity());

  do {
    if (this->buffer.capacity() <= MultiplyExpansionLimit) {
      newCapacity <<= 1;
      continue;
    }
    newCapacity += SingleExpansion;
  } while (newCapacity < needle);

  std::vector<char> tempBuffer;
  tempBuffer.swap(buffer);
  buffer.resize(newCapacity);
  do {
    if (this->begin <= this->end) {
      std::copy(tempBuffer.begin() + begin, tempBuffer.begin() + end, buffer.begin());
      break;
    }
    std::copy(tempBuffer.begin() + begin, tempBuffer.end(), buffer.begin());
    std::copy(tempBuffer.begin(), tempBuffer.begin() + end, buffer.begin() + getTailContentLen());
  } while (0);
  this->begin = 0;
  this->end = this->size;
}

int RingBuffer::ReadFromFd(int fd) noexcept {
  char expansionSpace[MAXPACKAGELEN];
  struct iovec iov[3];
  int iovLen;
  int writable = getWritableCapacity();
  if (getTailSpaceLen() > 0) {
    iov[0].iov_base = &(*buffer.begin()) + end;
    iov[0].iov_len = getTailSpaceLen();
    iov[1].iov_base = &(*buffer.begin());
    iov[1].iov_len = begin;
    iov[2].iov_base = expansionSpace;
    iov[2].iov_len = MAXPACKAGELEN;
    iovLen = 3;
  } else {
    iov[0].iov_base = &(*buffer.begin()) + end;
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
    updateAfterWrite(writable);
    Append(expansionSpace, resSize - writable);
  } else {
    updateAfterWrite(resSize);
  }
  return resSize;
}

int RingBuffer::WriteToFd(int fd) noexcept {
  struct iovec iov[2];
  int iovLen;
  int tailContentLen = getTailContentLen();

  if (tailContentLen > 0) {
    iov[0].iov_base = &(*buffer.begin()) + begin;
    iov[0].iov_len = tailContentLen;
    iov[1].iov_base = &(*buffer.begin());
    iov[1].iov_len = size - tailContentLen;
    iovLen = 2;
  } else {
    iov[0].iov_base = &(*buffer.begin()) + begin;
    iov[0].iov_len = size;
    iovLen = 1;
  }

  auto writeSize = static_cast<int>(writev(fd, iov, iovLen));

  if (writeSize < 0) {
    return -1;
  }
  updateAfterRead(writeSize);
  return writeSize;
}

}  // namespace CLSN