//
// Created by lqf on 23-4-30.
//
#include <gtest/gtest.h>
#include "coroutine/Poller.h"
#include "coroutine/Timer.h"
#include "log/Log.h"

static void test_timer() {
  clsn::Init<0>({clsn::CreateConsoleLogAppender("[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s", clsn::LogLevel::Debug)});
  {
    auto poller = clsn::CreateNewPoller();
    auto timerQueue = clsn::CreateNewTimerQueue();
    poller->RegisterRead(timerQueue->GetTimerFd(), *timerQueue);
    std::vector<clsn::FileDescriptor *> vec;
    int stop = false;

    timerQueue->AddTimer(std::chrono::seconds(5), std::function<void(void)>([&stop]() { stop = true; }));

    do {
      poller->EpollWait(vec, 10000);
      for (auto it : vec) {
        (*it)();
      }
    } while (!stop);
  }
}

TEST(test_timer, test_timer) {}
