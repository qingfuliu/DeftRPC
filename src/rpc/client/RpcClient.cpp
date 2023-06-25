//
// Created by lqf on 23-6-24.
//
#include "rpc/RpcCodeC.h"
#include "rpc/RpcClient.h"


namespace CLSN {

    RpcClient::RpcClient(const std::string &ipPort) noexcept:
            TcpClient(ipPort) {
        TcpClient::SetCodeC(new RpcCodeC);
    }

} // CLSN