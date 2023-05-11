//
// Created by lqf on 23-4-18.
//

#include "coroutine/Coroutine.h"
#include "coroutine/Task.h"
#include <benchmark/benchmark.h>

#include <iostream>

thread_local auto sharedMem = CLSN::MakeSharedStack(0);

void otherFunc1() {
    std::cout << "otherFunc1!!" << std::endl;
}


void otherFunc() {
    int a = 1;
    int b = 10000;
    int p[20];
    for (int i = 0; i < 20; i++) {
        p[i] = i;
    }
    std::cout << "before otherFunc11!!" << std::endl;

    auto routine = CLSN::CreateCoroutine();
    routine->setTask(otherFunc1);
    routine->swapIn();
    std::cout << "after otherFunc11!!" << std::endl;
    assert(a == 1);
    routine->reset(otherFunc1);
    std::cout << "before otherFunc122!!" << std::endl;
    routine->swapIn();
    std::cout << "after otherFunc122!!" << std::endl;
    assert(b == 10000);
    for (int i = 0; i < 20; i++) {
        assert(p[i] == i);
    }
}


static void test_SwapIn() {

    std::cout << "before test_SwapIn!!" << std::endl;
    auto routine = CLSN::CreateCoroutine();
    routine->setTask(otherFunc);
    routine->swapIn();
    std::cout << "after test_SwapIn!!" << std::endl;

}


void otherFuncShared() {
    int a = 1;
    int b = 10000;
    int sa[60000];
    int sa1[60000];
    int sa2[60000];
    int p[20];
    for (int i = 0; i < 20; i++) {
        p[i] = i;
    }
    std::cout << "before otherFunc11!!" << std::endl;

    auto routine = CLSN::CreateCoroutine();
    routine->swapIn();
    std::cout << "after otherFunc11!!" << std::endl;
    assert(a == 1);
    routine->reset(otherFunc1);
    std::cout << "before otherFunc122!!" << std::endl;
    routine->swapIn();
    std::cout << "after otherFunc122!!" << std::endl;
    assert(b == 10000);
    for (int i = 0; i < 20; i++) {
        assert(p[i] == i);
    }
}


static void test_sharedPtr_SwapIn() {

    std::cout << "before test_SwapIn!!" << std::endl;
    auto routine = CLSN::CreateCoroutine(otherFuncShared, sharedMem.get());
    routine->swapIn();
    std::cout << "after test_SwapIn!!" << std::endl;
    std::cout << "success!!" << std::endl;

}

int main() {
//    test_sharedPtr_SwapIn();
    test_SwapIn();
}
//BENCHMARK(test_SwapIn)->Iterations(1);
//BENCHMARK(test_sharedPtr_SwapIn)->Iterations(10)->Threads(10);
//BENCHMARK_MAIN();