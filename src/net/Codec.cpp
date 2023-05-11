//
// Created by lqf on 23-5-10.
//

#include "net/Codec.h"
#include "net/sever/TcpConnection.h"

namespace CLSN {

    void DefaultCodeC::Encode(RingBuffer &buffer, const std::string &msg) const noexcept {
        buffer.Write(msg);
    }

}