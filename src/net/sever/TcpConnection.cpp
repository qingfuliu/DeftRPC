//
// Created by lqf on 23-5-9.
//

#include "net/sever/TcpConnection.h"
#include "net/sever/TcpSever.h"
#include "net/Codec.h"
#include "coroutine/Timer.h"

namespace CLSN {

    TcpConnection::TcpConnection(int f, const Addr &addr) noexcept:
            noncopyable(),
            sock(f),
            remote(addr),
            inputBuffer(std::make_unique<RingBuffer>()),
            outputBuffer(std::make_unique<EVBuffer>()) {}

    TcpConnection::~TcpConnection() = default;

    void TcpConnection::NewTcpConnectionArrive(int fd, const Addr &remote) noexcept {
        CLSN_LOG_DEBUG << "new connection arrive,remote address is " << remote.toString();
        TcpConnection connection(fd, remote);
        connection.processMag();
        CLSN_LOG_DEBUG << "connection close,coroutine deconstruct";
    }
    

    void TcpConnection::processMag() {
        auto mSever = dynamic_cast<TcpSever *>(mScheduler);
        std::string exception;
        assert(nullptr != mSever);
        do {
            int res;
            do {
                res = inputBuffer->ReadFromFd(sock.getFd());
                if (res <= 0) {
                    break;
                }
                try {
                    std::string_view view = mSever->GetCodeC()->Decode(*inputBuffer);
                    std::string responseMsg = mSever->GetMagCallback()(this, view, TimeStamp::Now());
                    mSever->GetCodeC()->Encode(*outputBuffer, responseMsg);
                    if (res < 0) {
                        break;
                    }
                } catch (std::exception &e) {
                    exception = e.what();
                    outputBuffer->Write(exception);
                }

                do {
                    res = outputBuffer->WriteToFd(sock.getFd());
                } while (!outputBuffer->IsEmpty() && res >= 0);

            } while (true);


            if (res == 0) {
                if (-1 == sock.Close()) {
                    CLSN_LOG_ERROR << "connection close error,error is " << strerror(errno);
                }
                int fd = sock.getFd();
                mSever->AddDefer([mSever, fd]() { mSever->CloseConnection(fd); });
                break;
            } else if (res == -1 && errno == EBADF) {
                CLSN_LOG_ERROR << "please make sure this message only appears when the server is shut down!";
                assert(mSever->IsStop());
            } else {
                CLSN_LOG_ERROR << "read from sock error,error is " << strerror(errno);
            }
        } while (!mSever->IsStop());
    }
}
