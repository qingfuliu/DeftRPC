//
// Created by lqf on 23-5-6.
//
#include <gtest/gtest.h>
#include "coroutine/Scheduler.h"

static void scheduler_test() noexcept {
  clsn::Init<0>({clsn::CreateConsoleLogAppender("[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s", clsn::LogLevel::Debug)});
  clsn::Scheduler s;
  auto now = std::chrono::system_clock::now();
  auto id = s.DoEvery(std::chrono::seconds{5}, [now, &s]() {
    std::cout << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - now).count()
              << std::endl;
    s.AddTask([]() { std::cout << "AddTask!" << std::endl; });
  });
  if (-1 != id) {
    s.DoEvery(
        std::chrono::seconds{20},
        [id, &s]() {
          std::cout << "Cancel timer" << std::endl;
          s.CancelTimer(id);
        },
        false);
  }

  s.DoEvery(
      std::chrono::seconds{30}, [&s]() { s.Stop(); }, false);

  s.AddDefer([=]() { CLSN_LOG_DEBUG << "do defer"; });
  s.Start(10000);
}

TEST(test_scheduler, test_scheduler) { scheduler_test(); }