//
// Created by lqf on 23-6-24.
//
#include "rpc/RpcClient.h"
#include "rpc/RpcCodeC.h"

namespace CLSN {

RpcClient::RpcClient(const std::string &ipPort) noexcept : TcpClient(ipPort) { TcpClient::SetCodeC(new RpcCodeC); }

}  // namespace CLSN