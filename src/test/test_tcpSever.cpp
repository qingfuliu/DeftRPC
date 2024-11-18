//
// Created by lqf on 23-5-10.
//
#include <gtest/gtest.h>
#include "hook/Hook.h"
#include "net/sever/TcpSever.h"

TEST(test_tcpSever, test_tcpSever) {
  EnableHook();
  clsn::TcpSever sever("0.0.0.0:5201", 1);
  sever.DoAfter(std::chrono::seconds{10}, [&sever]() { sever.Stop(); });
  sever.Start(1000);
};