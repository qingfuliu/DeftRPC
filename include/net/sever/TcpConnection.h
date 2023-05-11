//
// Created by lqf on 23-5-9.
//

#ifndef DEFTRPC_TCPCONNECTION_H
#define DEFTRPC_TCPCONNECTION_H

#include "log/Log.h"
#include "common/common.h"
#include "coroutine/Coroutine.h"
#include "net/RingBuffer.h"
#include "coroutine/Timer.h"
#include "coroutine/Scheduler.h"
#include "net/Socket.h"
#include "net/Addr.h"
#include <memory>
#include <unistd.h>
#include <cerrno>

namespace CLSN {
    class TcpConnection;

    using MagCallback = std::function<std::string(TcpConnection *, std::string_view, CLSN::TimeStamp)>;

    class TcpConnection : public noncopyable {
    public:
        explicit TcpConnection(int f, const Addr &addr) noexcept;

        ~TcpConnection() override;

        void Write(const std::string &msg) noexcept {
            writeInThread(msg);
        }

        void Write(const char *msg, size_t len) noexcept {
            writeInThread(msg, len);
        }

        static void NewTcpConnectionArrive(int fd, const Addr &remote) noexcept;

    private:

        void writeInThread(const std::string &msg) noexcept {
            assert(mScheduler->IsInLoopThread());
            writeInThread(msg.c_str(), msg.size());
        }

        void writeInThread(const char *msg, size_t len) noexcept {
            assert(mScheduler->IsInLoopThread());
            outputBuffer->Write(msg, len);
        }

        void processMag();

    private:
        const Socket sock;
        const Addr remote;
        std::unique_ptr<RingBuffer> inputBuffer;
        std::unique_ptr<RingBuffer> outputBuffer;
        Scheduler *const mScheduler = Scheduler::GetThreadScheduler();
    };
}


#endif //DEFTRPC_TCPCONNECTION_H
