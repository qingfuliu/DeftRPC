//
// Created by lqf on 23-6-23.
//

#include "rpc/RpcCodeC.h"
#include "common/Crc32.h"
#include "rpc/RpcException.h"

namespace clsn {

std::string_view RpcCodeC::Decode(RingBuffer &buffer) const {
  if (DecodeCondition(buffer)) {
    PackageLengthType size = buffer.GetPackageLength();
    size = ntohl(size);
    if (size <= buffer.GetReadableCapacity()) {
      std::string_view res{buffer.Read(static_cast<int>(size)) + sizeof(PackageLengthType),
                           size - sizeof(PackageLengthType)};
      if (0 == GenerateCrc32(res.data(), res.size())) {
        throw std::logic_error(package_is_invalid);
      }
      res = res.substr(0, res.size() - sizeof(Crc32Type));
      return res;
    }
  }
  return {};
}

void RpcCodeC::Encode(EVBuffer &buffer, const char *data, size_t len) {
  WriteSizeToPackage(htonl(len + m_cache_length));
  buffer.Write(m_cache_.get(), sizeof(PackageLengthType));
  buffer.Write(data, len);

  Crc32Type check_code = clsn::GenerateCrc32(data, len);
  for (int i = -1 + sizeof check_code; i >= 0; --i) {
    int index = static_cast<int>(m_cache_length) - i - 1;
    std::copy(reinterpret_cast<char *>(&check_code) + i, reinterpret_cast<char *>(&check_code) + i + 1,
              m_cache_.get() + index + sizeof(PackageLengthType));
  }
  buffer.Write(m_cache_.get() + sizeof(PackageLengthType), sizeof(Crc32Type));
}

}  // namespace clsn