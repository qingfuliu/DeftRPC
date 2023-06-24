//
// Created by lqf on 23-6-22.
//
#include "log/Log.h"
#include "rpc/Router.h"

int test_router(int a) {
    return a + 1;
}

int main() {
    CLSN::init<0>({
                          CLSN::createConsoleLogAppender(
                                  "[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s",
                                  CLSN::LogLevel::Debug)});

    CLSN::RpcRouter r("test");
    r.InsertFunc("test", test_router);
//    r.InsertFunc("test1", test_router);

    std::string res;
    CLSN::StringSerialize encode(res);
    encode(std::tuple<int>(100));

    auto p = r.CallFuncSync("test", res);

//    std::string p;
    CLSN::StringDeSerialize decode(p);
    int aa;
    decode(aa);

    CLSN_LOG_DEBUG << aa;

}