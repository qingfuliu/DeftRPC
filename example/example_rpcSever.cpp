//
// Created by lqf on 23-6-24.
//
#include "log/Log.h"
#include "rpc/Router.h"
#include "rpc/RPCSever.h"


int test_router(int a) {
    return a + 1;
}

int test_rpc(int a) {
    return a + 1;
}

int main() {
    CLSN::init<0>({
                          CLSN::createConsoleLogAppender(
                                  "[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s",
                                  CLSN::LogLevel::Debug)});

    auto r = new CLSN::RpcRouter("test");
    r->InsertFunc("test_router", test_router);
    r->InsertFunc("test_rpc", test_rpc);

    auto rpcSever = CLSN::CreateRpcSever("0.0.0.0:5201", 1);

    rpcSever->SetRouter(r);

    rpcSever->Start(10000);
}


int main1() {


    CLSN::RpcRouter router;

    router.InsertFunc("test_rpc", test_rpc);


}