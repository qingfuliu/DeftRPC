//
// Created by lqf on 23-5-6.
//
#include "coroutine/Scheduler.h"
#include <gtest/gtest.h>

static void scheduler_test() noexcept {
    CLSN::init<0>({
                          CLSN::createConsoleLogAppender(
                                  "[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s",
                                  CLSN::LogLevel::Debug)});
    CLSN::Scheduler s;
    auto now = std::chrono::system_clock::now();
    auto id = s.DoEvery(std::chrono::seconds{5}, [now, &s]() {
        std::cout << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - now).count()
                  << std::endl;
        s.AddTask([]() {
            std::cout << "AddTask!" << std::endl;
        });
    });
    if (-1 != id) {
        s.DoEvery(std::chrono::seconds{20}, [id, &s]() {
            std::cout << "Cancel timer" << std::endl;
            s.CancelTimer(id);
        }, false);
    }

    s.DoEvery(std::chrono::seconds{30}, [&s]() {
        s.Stop();
    }, false);

    s.AddDefer([=]() {
        CLSN_LOG_DEBUG << "do defer";
    });
    s.Start();
}

TEST(test_scheduler, test_scheduler) {
    scheduler_test();
}