#ifndef DEFTRPC_BUFFER_H
#define DEFTRPC_BUFFER_H
#include <cstdint>
#include <string>
namespace clsn {
class Buffer {
 public:
  Buffer() = default;

  virtual ~Buffer() = default;

  virtual std::string Read(std::uint32_t len) = 0;

  std::string ReadAll() { return Read(Size()); }

  virtual std::string Peek(std::uint32_t len) = 0;

  std::string CPeek(std::uint32_t len) {
    std::string peek = Peek(Size());
    if (peek.size() > len) {
      peek = peek.substr(peek.size() - len);
    }
    return peek;
  }

  [[nodiscard]] virtual std::uint32_t Size() const noexcept = 0;

  [[nodiscard]] virtual bool Empty() const noexcept = 0;

  [[nodiscard]] virtual std::uint32_t Capacity() const noexcept = 0;

  virtual void Clear() noexcept = 0;

  virtual std::uint32_t Write(const char *data, std::uint32_t len) = 0;

  std::uint32_t Write(const std::string &data) { return Write(data.data(), data.size()); }

  std::uint32_t Append(const std::string &data) { return Write(data.data(), data.size()); }

  std::uint32_t Append(const char *data, std::uint32_t len) { return Write(data, len); }

  virtual int FetchDataFromFd(int fd) = 0;

  virtual int FlushDataToFd(int fd) noexcept = 0;
};
}  // namespace clsn

#endif
