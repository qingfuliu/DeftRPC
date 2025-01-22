//
// Created by lqf on 23-1-11.
//

#ifndef MCLOUDDISK_MCLOUDBUFFER_H
#define MCLOUDDISK_MCLOUDBUFFER_H

#include <arpa/inet.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "Buffer.h"

namespace clsn {

static inline constexpr std::int32_t MAX_PACKAGE_LEN = 1024;
static inline constexpr std::int32_t SINGLE_EXPANSION = 1024;
static inline constexpr std::int32_t MULTIPLY_EXPANSION_LIMIT = 2048;
static inline constexpr std::int32_t DEFAULT_BUFFER_LEN = 2048;

class RingBuffer : public Buffer {
 public:
  RingBuffer() = default;

  ~RingBuffer() override = default;

  std::string Read(std::uint32_t len) override;

  std::string Peek(std::uint32_t len) override;

  [[nodiscard]] std::uint32_t Size() const noexcept override { return m_size_; }

  [[nodiscard]] bool Empty() const noexcept override { return m_begin_ == m_end_ && m_size_ == 0; }

  [[nodiscard]] std::uint32_t Capacity() const noexcept override { return m_buffer_.capacity(); }

  void Clear() noexcept override {
    m_begin_ = 0;
    m_end_ = 0;
    m_size_ = 0;
  }

  std::uint32_t Write(const char *data, std::uint32_t len) override;

  int FetchDataFromFd(int fd) override;

  int FlushDataToFd(int fd) noexcept override;

 private:
  [[nodiscard]] bool IsFull() const noexcept { return m_begin_ == m_end_ && m_size_ == m_buffer_.capacity(); }

  [[nodiscard]] std::uint32_t GetTailSpaceLen() const noexcept;

  [[nodiscard]] std::uint32_t GetTailContentLen() const noexcept;

  [[nodiscard]] std::uint32_t GetWritableCapacity() const noexcept;

  void UpdateAfterRead(std::uint32_t len) noexcept;

  void UpdateAfterWrite(std::uint32_t len) noexcept;

  void EnableWritableSpace(std::uint32_t targetSize) noexcept;

 private:
  std::uint32_t m_begin_{0};
  std::uint32_t m_end_{0};
  std::uint32_t m_size_{0};
  std::vector<char> m_buffer_{std::vector<char>(DEFAULT_BUFFER_LEN)};
};

}  // namespace clsn

#endif  // MCLOUDDISK_MCLOUDBUFFER_H
