//
// Created by lqf on 23-4-21.
//
#include<benchmark/benchmark.h>
#include <iostream>
#include "common/SpanMutex.h"
static int a=0;
static CLSN::SpanMutex spanM;
void test_spanMutex(benchmark::State &state){
    for(auto _:state){
        spanM.Lock();
//        std::cout<<"Lock"<<std::endl;
        ++a;
        spanM.UnLock();
    }
}

void test_spanMutex_Reentrant(benchmark::State &state){
    CLSN::ReentrantMutex<CLSN::SpanMutex>mutexRe{};
    for(auto _:state){
        mutexRe.Lock();
        std::cout<<CLSN::Thread::thisThreadId()<<std::endl;
        std::cout<<"Lock"<<std::endl;
        mutexRe.Lock();
        std::cout<<"Lock1"<<std::endl;
        mutexRe.Lock();
        std::cout<<"Lock2"<<std::endl;
        ++a;
        spanM.UnLock();
    }
}


void test_spanMutex_res(benchmark::State &state){
    for(auto _:state){
        std::cout<<a<<std::endl;
    }
}


BENCHMARK(test_spanMutex_Reentrant)->Threads(1000)->Iterations(1);
BENCHMARK(test_spanMutex_res)->Threads(1)->Iterations(1);
BENCHMARK_MAIN();