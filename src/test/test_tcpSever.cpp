//
// Created by lqf on 23-5-10.
//
#include "net/sever/TcpSever.h"
#include"hook/Hook.h"
#include<gtest/gtest.h>

TEST(test_tcpSever, test_tcpSever) {
    Enable_Hook();
    CLSN::TcpSever sever("0.0.0.0:5201", 1);
//    sever.DoAfter(std::chrono::seconds{10}, [&sever]() { sever.Stop(); }
//    );
    sever.Start(10000);
};