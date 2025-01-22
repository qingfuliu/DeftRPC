//
// Created by lqf on 23-5-10.
//

#ifndef DEFTRPC_CODEC_H
#define DEFTRPC_CODEC_H

#include <arpa/inet.h>
#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include "common/buffer/Buffer.h"
#include "common/common.h"

// std::string m_func_(std::shared_ptr<conn>, std::string_view, TimeStamp);

namespace clsn {
class TcpConnection;

class CodeC {
 public:
  CodeC() noexcept = default;

  virtual ~CodeC() = default;

  virtual std::string Decode(Buffer *buffer) const = 0;

  virtual void Encode(Buffer *buffer, const char *data, std::uint32_t len) = 0;

  void Encode(Buffer *buffer, std::string &str) { Encode(buffer, str.data(), str.size()); }
};

class CodeCFactory {
 public:
  static CodeC *CreateCodeC() noexcept { return nullptr; }
};

class DefaultCodeC : public CodeC {
 public:
  DefaultCodeC() = default;

  ~DefaultCodeC() override = default;

  std::string Decode(Buffer *buffer) const override {
    if (buffer->Size() < sizeof(PackageLengthType)) {
      return {};
    }

    std::string peek = buffer->Peek(sizeof(PackageLengthType));
    PackageLengthType packageSize = 0;
    std::copy(peek.begin(), peek.end(), reinterpret_cast<char *>(&packageSize));
    packageSize = ntohl(packageSize);

    std::uint32_t size = buffer->Size();
    if (size <= sizeof(PackageLengthType) + packageSize) {
      return {};
    }
    buffer->Read(sizeof(PackageLengthType));
    return buffer->Read(packageSize);
  }

  void Encode(Buffer *buffer, const char *data, std::uint32_t len) override {
    PackageLengthType packageLength = htonl(len);
    buffer->Write(reinterpret_cast<char *>(&packageLength), sizeof(PackageLengthType));
    buffer->Write(data, len);
  }
};

class DefaultCodeCFactory : public CodeCFactory {
 public:
  static CodeC *CreateCodeC() noexcept { return new DefaultCodeC(); }
};
}  // namespace clsn

#endif  // DEFTRPC_CODEC_H
