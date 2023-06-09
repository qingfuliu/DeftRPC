//
// Created by lqf on 23-5-10.
//
#include "net/sever/TcpSever.h"
#include "net/sever/TcpConnection.h"
#include "log/Log.h"
#include "hook/Hook.h"

std::string Echo(CLSN::TcpConnection *, std::string_view msg, CLSN::TimeStamp) {
    return {msg.data(), msg.size()};
}

int main() {
    Enable_Hook();
    CLSN::TcpSever sever("0.0.0.0:5201", 1);
    sever.SetMagCallback(Echo);
    sever.Start(100000);
    return 0;
}