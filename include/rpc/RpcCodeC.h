//
// Created by lqf on 23-6-23.
//

#ifndef DEFTRPC_RPCCODEC_H
#define DEFTRPC_RPCCODEC_H

#include <string>
#include "common/common.h"
#include "net/Codec.h"

namespace CLSN {

class RpcCodeC : public CodeC {
 public:
  RpcCodeC() = default;

  ~RpcCodeC() override = default;

  std::string_view Decode(RingBuffer &buffer) const override;

  void Encode(EVBuffer &buffer, const char *data, size_t len) override;

 protected:
  virtual bool DecodeCondition(RingBuffer &buffer) const noexcept {
    return buffer.GetReadableCapacity() >= sizeof(PackageLengthType);
  }

  virtual void WriteSizeToPackage(size_t size) noexcept {
    PackageLengthType be = htonl(size);
    std::copy(reinterpret_cast<char *>(&be), reinterpret_cast<char *>(&be) + sizeof(PackageLengthType), cache.get());
  }

 private:
  inline static size_t cacheLength = sizeof(PackageLengthType) + sizeof(Crc32Type);
  std::unique_ptr<char[]> cache{new char[cacheLength]};
};

}  // namespace CLSN

#endif  // DEFTRPC_RPCCODEC_H
