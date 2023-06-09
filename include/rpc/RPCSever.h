//
// Created by lqf on 23-6-23.
//

#ifndef DEFTRPC_RPCSEVER_H
#define DEFTRPC_RPCSEVER_H

#include <memory>
#include "exception"
#include "rpc/Router.h"
#include "rpc/RpcProtocol.h"
#include "rpc/RpcException.h"
#include "net/sever/TcpSever.h"
#include "net/sever/TcpConnection.h"
#include "serialize/BinarySerialize.h"
#include "serialize/StringSerializer.h"

namespace CLSN {

    class RPCSever : public TcpSever {
    public:
        explicit RPCSever(const std::string &ipPort, size_t sharedStackSize = 0) noexcept;

        void SetRouter(RpcRouter *rpcRouter) noexcept {
            router.reset(rpcRouter);
        }

        template<class Func, class... Args>
        void InsertFunc(const std::string &funcName, Func f, Args &&...args) {
            if (router == nullptr) {
                throw std::logic_error(RouterIsInvalid);
            }
            router->InsertFunc(funcName, std::move(f), std::forward<Args>(args)...);
        }

        void DeleteFunc(const std::string &funcName) {
            if (router == nullptr) {
                throw std::logic_error(RouterIsInvalid);
            }
            router->DeleteFunc(funcName);
        }

        void Start(int timeout) noexcept override;

    private:
        std::string MessageCallBack(CLSN::TcpConnection *connection, std::string_view arg, CLSN::TimeStamp t);

    private:
        std::unique_ptr<RpcRouter> router;
    };

    std::unique_ptr<RPCSever> CreateRpcSever(const std::string &ipPort, size_t sharedStackSize = 0) noexcept {
        return std::make_unique<RPCSever>(ipPort, sharedStackSize);
    }

} // CLSN

#endif //DEFTRPC_RPCSEVER_H
