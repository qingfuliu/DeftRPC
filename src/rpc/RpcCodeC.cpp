//
// Created by lqf on 23-6-23.
//

#include "rpc/RpcCodeC.h"
#include "common/Crc32.h"
#include "rpc/RpcException.h"

namespace CLSN {

    std::string_view RpcCodeC::Decode(RingBuffer &buffer) const {
        if (DecodeCondition(buffer)) {
            PackageLengthType size = buffer.GetPackageLength();
            size = ntohl(size);
            if (size <= buffer.GetReadableCapacity()) {
                std::string_view res{buffer.Read(static_cast<int>(size)) + sizeof(PackageLengthType),
                                     size - sizeof(PackageLengthType)};
                if (!GenerateCrc32(res.data(), res.size())) {
                    throw std::logic_error(PackageIsInvalid);
                }
                res = res.substr(0, res.size() - sizeof(Crc32Type));
                return res;
            }
        }
        return {};
    }

    void RpcCodeC::Encode(EVBuffer &buffer, const char *data, size_t len) {
        WriteSizeToPackage(len + cacheLength);
        buffer.Write(cache.get(), sizeof(PackageLengthType));

        Crc32Type checkCode = CLSN::GenerateCrc32(data, len);

        for (int i = -1 + sizeof checkCode; i >= 0; --i) {
            int index = static_cast<int>(cacheLength) - i - 1;
            std::copy(reinterpret_cast<char *>(&checkCode) + i,
                      reinterpret_cast<char *>(&checkCode) + i + 1,
                      cache.get() + index + sizeof(PackageLengthType));
        }

        buffer.Write(cache.get() + sizeof(PackageLengthType), sizeof(Crc32Type));

        buffer.Write(data, len);
    }

} // CLSN