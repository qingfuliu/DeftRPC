//
// Created by lqf on 23-6-25.
//
#include "hook/Hook.h"
#include "rpc/RpcClient.h"

int main() {
    Enable_Hook();
    Disable_Enable_Hook();
    CLSN::RpcClient client("0.0.0.0:5201");
    CLSN_LOG_DEBUG << "remote:" << client.GetRemote().toString();
    if (!client.Connect()) {
        CLSN_LOG_ERROR << "connect failed,error is "
                       << strerror(errno);
        return -1;
    }

    auto p = client.DoFuncAsync<int>("test_router", 1);

    CLSN_LOG_DEBUG << p.get();
}