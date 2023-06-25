//
// Created by lqf on 23-4-30.
//
#include <benchmark/benchmark.h>
#include "coroutine/Timer.h"
#include "coroutine/Poller.h"
#include "log/Log.h"

static void test_timer(benchmark::State &state) {

    CLSN::init<0>({
                          CLSN::createConsoleLogAppender(
                                  "[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s",
                                  CLSN::LogLevel::Debug)});
    for (auto _: state) {
        auto poller = CLSN::CreateNewPoller();
        auto timerQueue = CLSN::CreateNewTimerQueue();
        poller->RegisterRead(timerQueue->GetTimerFd(), *timerQueue);
        std::vector<CLSN::FdDescriptor *> vec;
        int stop = false;

        timerQueue->AddTimer(std::chrono::seconds(5),
                             std::function<void(void)>([&stop]() {
                                 stop = true;
                             }));

        do {
            poller->EpollWait(vec, 10000);
            for (auto it: vec) {
                (*it)();
            }
        } while (!stop);
    }
}

BENCHMARK(test_timer)->Threads(1)->Iterations(1);
BENCHMARK_MAIN();
