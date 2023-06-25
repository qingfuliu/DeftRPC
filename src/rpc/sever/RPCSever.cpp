//
// Created by lqf on 23-6-23.
//
#include "log/Log.h"
#include "rpc/RPCSever.h"
#include "rpc/RpcCodeC.h"

namespace CLSN {

    RPCSever::RPCSever(const std::string &ipPort, size_t sharedStackSize) noexcept:
            TcpSever(ipPort, sharedStackSize),
            router(nullptr) {
        TcpSever::SetMagCallback([this](auto &&PH1, auto &&PH2, auto &&PH3) {
            return MessageCallBack(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2),
                                   std::forward<decltype(PH3)>(PH3));
        });
        TcpSever::SetCodeC(new RpcCodeC);
    }

    void RPCSever::Start(int timeout) noexcept {
#ifdef DEBUG
        auto it = router->GetIterator();
        CLSN_LOG_DEBUG
                    << "============================================rpc functions============================================";
        while (it->IsValid()) {
            CLSN_LOG_DEBUG << HashTable::GetKeyByIterator(it.get());
            it->Next();
        }
        CLSN_LOG_DEBUG
                    << "============================================rpc functions============================================";
#endif
        TcpSever::Start(timeout);
    }

    std::string RPCSever::MessageCallBack(CLSN::TcpConnection *connection, std::string_view arg, CLSN::TimeStamp t) {
        RPCResponse response;

        RPCRequest request;
        CLSN::StringDeSerialize deCode(arg);
        try {
            deCode(request);
            if (request.async == static_cast<short>(RpcType::Sync)) {
                response.res = router->CallFuncSync(request.funcName, request.args);
                response.succeed = true;
            }
        } catch (std::exception &e) {
            response.succeed = false;
            response.res = e.what();
        }
        std::string res;
        CLSN::StringSerialize enCode(res);
        enCode(response);
        return res;
    }

} // CLSN