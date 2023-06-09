//
// Created by lqf on 23-5-10.
//

#ifndef DEFTRPC_TCPCLIENT_H
#define DEFTRPC_TCPCLIENT_H

#include "net/Addr.h"
#include "net/Socket.h"
#include "net/RingBuffer.h"
#include "net/EVBuffer.h"
#include "net/Codec.h"
#include "log/Log.h"
#include <string_view>
#include <cerrno>
#include <cstring>

namespace CLSN {

    class TcpClient {
    public:
        enum class State : short {
            Construct,
            Connecting,
            Connected,
            ConnectFailed,
            ErrorOccurred,
            Closed
        };
    public:
        explicit TcpClient(const std::string &ipPort) noexcept;

        ~TcpClient() noexcept;

        bool Connect(int timeOut = 0) noexcept {
            if (state >= State::Connected) {
                return state == State::Connected;
            }
            if ((timeOut > 0 && 0 != SetReadTimeOut(timeOut))
                || 0 != sock.Connect(&remote)) {
                state = State::ConnectFailed;
                return false;
            }
            if (timeOut > 0) {
                return SetReadTimeOut(readTimeout);
            }
            return true;
        }

        bool ConnectWithAddr(const Addr &addr, int timeOut = 0) noexcept {
            if (state >= State::Connected) {
                return state == State::Connected;
            }
            if ((timeOut > 0 && 0 != SetReadTimeOut(timeOut))
                || 0 != sock.Connect(&addr)) {
                state = State::ConnectFailed;
                return false;
            }
            if (timeOut > 0) {
                return SetReadTimeOut(readTimeout);
            }
            return true;
        }

        int Send(const std::string &str) noexcept {
            return SendAll(str.data(), str.size());
        }

        int SendAll(const char *msg, size_t len) noexcept {
            if (len == 0) {
                return 0;
            }
            codeC->Encode(*outputBuffer, msg, len);
            int res = 0;
            int temp;
            do {
                temp = outputBuffer->WriteToFd(sock.getFd());
                res += temp;
            } while (temp >= 0 && !outputBuffer->IsEmpty());
            if (temp < 0) {
                return -1;
            }
            return res;
        }

        size_t Receive(char *data, size_t len) noexcept {
            if (len <= 0) {
                return 0;
            }
            int res = inputBuffer->ReadFromFd(sock.getFd());
            if (res > 0) {
                len = res > len ? len : res;
                inputBuffer->Read(data, static_cast<int>(len));
            }
            return res;
        }

        std::string_view Receive() {
            std::string_view view;
            do {
                int res = inputBuffer->ReadFromFd(sock.getFd());
                if (res < 0) {
                    break;
                }
                view = codeC->Decode(*inputBuffer);
            } while (view.empty());
            return view;
        }

        [[nodiscard]] int MakeSelfNonblock(bool val) const noexcept {
            return sock.SetNoBlock(val);
        }

        [[nodiscard]] int Close() noexcept {
            state = State::Closed;
            return sock.Close();
        }

        [[nodiscard]] State GetClientState() const noexcept {
            return state;
        }

        int SetReadTimeOut(int timeOut) noexcept {
            readTimeout = timeOut;
            int res = sock.SetReadTimeout(timeOut);
            if (res != 0) {
                state = State::ErrorOccurred;
                CLSN_LOG_ERROR << "set read timeout failed after connected,error is "
                               << strerror(errno);
            }
            return res;
        }

        int SetWriteTimeOut(int timeOut) noexcept {
            writeTimeout = timeOut;
            int res = sock.SetWriteTimeout(timeOut);
            if (res != 0) {
                state = State::ErrorOccurred;
                CLSN_LOG_ERROR << "set write timeout failed,error is "
                               << strerror(errno);
            }
            return res;
        }

        Socket &GetSocket() noexcept {
            return sock;
        }

        Addr &GetRemote() noexcept {
            return remote;
        }

        void SetCodeC(CodeC *enCodeC) noexcept {
            codeC.reset(enCodeC);
        }

    private:
        Socket sock;
        Addr remote;
        State state;
        int readTimeout;
        int writeTimeout;
        std::unique_ptr<CodeC> codeC;
        std::unique_ptr<RingBuffer> inputBuffer;
        std::unique_ptr<EVBuffer> outputBuffer;

    };

} // CLSN

#endif //DEFTRPC_TCPCLIENT_H
