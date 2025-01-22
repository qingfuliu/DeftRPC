//
// Created by root on 1/20/25.
//

//
// Created by lqf on 23-4-30.
//
#include <gtest/gtest.h>
#include "common/codeC/Codec.h"
#include "log/Log.h"

static void test_codeC() {
  clsn::Init<0>({clsn::CreateConsoleLogAppender("[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s", clsn::LogLevel::Debug)});
  std::vector<std::string> messages(1024);
}

TEST(test_codeC, test_codeC) {}
