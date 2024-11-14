//
// Created by lqf on 23-1-11.
//

#ifndef MCLOUDDISK_MCLOUDBUFFER_H
#define MCLOUDDISK_MCLOUDBUFFER_H

#include <arpa/inet.h>
#include <memory>
#include <string>
#include <vector>

namespace clsn {
class RingBuffer {
 public:
  RingBuffer() noexcept;

  ~RingBuffer() = default;

  [[nodiscard]] uint32_t GetPackageLength() const noexcept;

  char *ReadAll(int *len = nullptr) noexcept;

  char *Read(int size, int *len = nullptr) noexcept;

  int Read(char *buf, int len) noexcept;

  [[nodiscard]] int GetReadableCapacity() const noexcept { return m_size_; }

  int ReadFromFd(int fd) noexcept;

  int WriteToFd(int fd) noexcept;

  int Write(const char *buf, int len) noexcept;

  int Write(const std::string &buf) noexcept { return this->Write(buf.c_str(), static_cast<int>(buf.size())); }

  void Append(const std::string &buf) noexcept { Write(buf); }

  void Append(const char *buf, int len) noexcept { Write(buf, len); }

  [[nodiscard]] bool IsEmpty() const noexcept { return m_begin_ == m_end_ && m_size_ == 0; }

 private:
  [[nodiscard]] bool IsFull() const noexcept { return m_begin_ == m_end_ && m_size_ == m_buffer_.capacity(); }

  [[nodiscard]] int GetTailSpaceLen() const noexcept;

  [[nodiscard]] int GetTailContentLen() const noexcept;

  [[nodiscard]] int GetWritableCapacity() const noexcept;

  void UpdateAfterRead(int len) noexcept;

  void UpdateAfterWrite(int len) noexcept;

  void EnableWritableSpace(int targetSize) noexcept;

 private:
  int m_begin_;
  int m_end_;
  int m_size_;
  std::vector<char> m_buffer_;
  int m_temp_capacity_;
  std::unique_ptr<char[]> m_temp_;
};

}  // namespace clsn

#endif  // MCLOUDDISK_MCLOUDBUFFER_H
