//
// Created by lqf on 23-5-10.
//

#ifndef DEFTRPC_CODEC_H
#define DEFTRPC_CODEC_H

#include <string_view>
#include "common/common.h"
#include "net/EVBuffer.h"
#include "net/RingBuffer.h"

// std::string m_func_(std::shared_ptr<conn>, std::string_view, TimeStamp);

namespace clsn {
class TcpConnection;

class CodeC {
 public:
  CodeC() noexcept = default;

  virtual ~CodeC() = default;

  virtual std::string_view Decode(RingBuffer &buffer) const = 0;

  virtual void Encode(EVBuffer &buffer, const char *data, size_t len) = 0;

  void Encode(EVBuffer &buffer, std::string &str) { Encode(buffer, str.data(), str.size()); }
};

class CodeCFactory {
 public:
  static CodeC *CreateCodeC() noexcept { return nullptr; }
};

class DefaultCodeC : public CodeC {
 public:
  DefaultCodeC() = default;

  ~DefaultCodeC() override = default;

  std::string_view Decode(RingBuffer &buffer) const override {
    if (DecodeCondition(buffer)) {
      PackageLengthType size = buffer.GetPackageLength();
      if (size <= buffer.GetReadableCapacity()) {
        return {buffer.ReadAll() + sizeof(PackageLengthType), size - sizeof(PackageLengthType)};
      }
    }
    return {};
  }

  void Encode(EVBuffer &buffer, const char *data, size_t len) override {
    WriteSizeToPackage(len + sizeof(PackageLengthType));
    buffer.Write(m_cache_.get(), sizeof(PackageLengthType));
    buffer.Write(data, len);
  }

 protected:
  virtual bool DecodeCondition(RingBuffer &buffer) const noexcept {
    return buffer.GetReadableCapacity() >= sizeof(PackageLengthType);
  }

  virtual void WriteSizeToPackage(size_t size) noexcept {
    PackageLengthType be = htonl(size);
    std::copy(reinterpret_cast<char *>(&be), reinterpret_cast<char *>(&be) + sizeof(PackageLengthType), m_cache_.get());
  }

 private:
  std::unique_ptr<char[]> m_cache_{new char[sizeof(PackageLengthType)]};
};

class DefaultCodeCFactory : public CodeCFactory {
 public:
  static CodeC *CreateCodeC() noexcept { return new DefaultCodeC(); }
};
}  // namespace clsn

#endif  // DEFTRPC_CODEC_H
