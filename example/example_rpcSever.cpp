//
// Created by lqf on 23-6-24.
//
#include "log/Log.h"
#include "rpc/Router.h"
#include "rpc/RPCSever.h"
#include "hook/Hook.h"
int test_router(int a) {
    return a + 1;
}

int test_rpc(int a) {
    return a + 1;
}

int test_exception(int a, int b) {
    throw std::runtime_error("test exception");
}

int main() {
    Enable_Hook();
    
    auto r = new CLSN::RpcRouter("test");
    r->InsertFunc("test_router", test_router);
    r->InsertFunc("test_rpc", test_rpc);
    r->InsertFunc("test_exception", test_exception);

    auto rpcSever = CLSN::CreateRpcSever("0.0.0.0:5201", 1);

    rpcSever->SetRouter(r);

    rpcSever->Start(100000);
}


