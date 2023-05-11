//
// Created by lqf on 23-5-10.
//

#ifndef DEFTRPC_CODEC_H
#define DEFTRPC_CODEC_H

#include<string_view>
#include"net/RingBuffer.h"

//std::string func(std::shared_ptr<conn>, std::string_view, TimeStamp);

namespace CLSN {
    class TcpConnection;

    class CodeC {
    public:
        CodeC() noexcept = default;

        virtual ~CodeC() = default;

        virtual std::string_view Decode(RingBuffer &buffer) const noexcept = 0;

        virtual void Encode(RingBuffer &buffer, const std::string &) const noexcept = 0;
    };

    class CodeCFactory {
    public:
        static CodeC *CreateCodeC() noexcept {
            return nullptr;
        }
    };

    class DefaultCodeC : public CodeC {
    public:
        DefaultCodeC() noexcept = default;

        ~DefaultCodeC() override = default;

        std::string_view Decode(RingBuffer &buffer) const noexcept override {
            size_t size = buffer.GetReadableCapacity();
            return {buffer.ReadAll(), size};
        }

        void Encode(RingBuffer &buffer, const std::string &) const noexcept override;
    };

    class DefaultCodeCFactory : public CodeCFactory {
    public:
        static CodeC *CreateCodeC() noexcept {
            return new DefaultCodeC;
        }
    };
}


#endif //DEFTRPC_CODEC_H
