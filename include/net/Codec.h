//
// Created by lqf on 23-5-10.
//

#ifndef DEFTRPC_CODEC_H
#define DEFTRPC_CODEC_H

#include<string_view>
#include"net/RingBuffer.h"
#include"net/EVBuffer.h"
#include "common/common.h"
//std::string func(std::shared_ptr<conn>, std::string_view, TimeStamp);

namespace CLSN {
    class TcpConnection;

    class CodeC {
    public:
        CodeC() noexcept = default;

        virtual ~CodeC() = default;

        virtual std::string_view Decode(RingBuffer &buffer) const noexcept = 0;

        virtual void Encode(EVBuffer &buffer, const char *data, size_t len) noexcept = 0;

        virtual void Encode(EVBuffer &buffer, std::string &str) noexcept {
            Encode(buffer, str.data(), str.size());
        }
    };

    class CodeCFactory {
    public:
        static CodeC *CreateCodeC() noexcept {
            return nullptr;
        }
    };

    class DefaultCodeC : public CodeC {
    public:
        DefaultCodeC() = default;

        ~DefaultCodeC() override = default;

        std::string_view Decode(RingBuffer &buffer) const noexcept override {
            if (DecodeCondition(buffer)) {
                PackageLengthType size = buffer.GetPackageLength();
                if (size <= buffer.GetReadableCapacity()) {
                    return {buffer.ReadAll() + sizeof(PackageLengthType), size - sizeof(PackageLengthType)};
                }
            }
            return {};
        }

        void Encode(EVBuffer &buffer, const char *data, size_t len) noexcept override {
            WriteSizeToPackage(len + sizeof(PackageLengthType));
            buffer.Write(cache.get(), sizeof(PackageLengthType));
            buffer.Write(data, len);
        }

    protected:
        virtual bool DecodeCondition(RingBuffer &buffer) const noexcept {
            return buffer.GetReadableCapacity() >= sizeof(PackageLengthType);
        }

        virtual void WriteSizeToPackage(size_t size) noexcept {
            PackageLengthType be = htonl(size);
            std::copy(reinterpret_cast<char *>(&be), reinterpret_cast<char *>(&be) + sizeof(PackageLengthType),
                      cache.get());
        }

    private:
        std::unique_ptr<char[]> cache{new char[sizeof(PackageLengthType)]};
    };

    class DefaultCodeCFactory : public CodeCFactory {
    public:
        static CodeC *CreateCodeC() noexcept {
            return new DefaultCodeC();
        }
    };
}


#endif //DEFTRPC_CODEC_H
