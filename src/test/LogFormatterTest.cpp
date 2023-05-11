#include<iostream>
#include <benchmark/benchmark.h>
#include"log/Log.h"
//#include"

class Init{
public:
    Init(){
        CLSN::init<0>({
                              CLSN::createFileLogAppender("/home/lqf/project/DeftRPC/src/test/log.txt",
                                                          "[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s",
                                                          CLSN::LogLevel::Debug)});
    }
};

static void test_log(benchmark::State&state){
    static Init a;
    for(auto _:state){
        CLSN_LOG_DEBUG << "message";
    }
}

//BENCHMARK(init)->Iterations(1)->Threads(1);

BENCHMARK(test_log)->Threads(10)->Iterations(1000);
BENCHMARK_MAIN();