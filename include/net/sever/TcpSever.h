//
// Created by lqf on 23-5-10.
//

#ifndef DEFTRPC_TCPSEVER_H
#define DEFTRPC_TCPSEVER_H

#include "net/Socket.h"
#include "net/Addr.h"
#include "coroutine/Scheduler.h"
#include <unordered_map>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>

namespace CLSN {

    class TcpConnection;


    class CodeC;

    class TcpSever : public Scheduler {
    public:
        using MagCallback = std::function<std::string(TcpConnection *, std::string_view, CLSN::TimeStamp)>;


        explicit TcpSever(const std::string &ipPort, size_t sharedStackSize = 0, bool UserCall = true) noexcept;

        ~TcpSever() override;

        [[nodiscard]] CodeC *GetCodeC() noexcept {
            return mCodeC.get();
        }

        void SetMagCallback(MagCallback callback) noexcept {
            magCallback = std::move(callback);
        }

        MagCallback &GetMagCallback() noexcept {
            return magCallback;
        }

        void CloseConnection(int fd, bool activelyClose = false) noexcept;

        void Start(int timeout) noexcept override;

        void Stop() noexcept override;

    private:
        void acceptTask() noexcept;

        void newConnectionArrives(int fd, const Addr &remote) noexcept;

        void closeAllConnection() noexcept;

        void closeAcceptor() const noexcept;

    private:
        Socket acceptSock;
        const Addr local;
        std::unique_ptr<CodeC> mCodeC;
        std::unique_ptr<Coroutine> acceptCoroutine;
        std::unordered_map<int, std::unique_ptr<Coroutine>> connections;
        MagCallback magCallback;
    };

} // CLSN

#endif //DEFTRPC_TCPSEVER_H
