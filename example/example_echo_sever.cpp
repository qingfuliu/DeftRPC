//
// Created by lqf on 23-5-10.
//
#include "hook/Hook.h"
#include "log/Log.h"
#include "net/sever/TcpConnection.h"
#include "net/sever/TcpSever.h"

std::string Echo(clsn::TcpConnection *, std::string_view msg, clsn::TimeStamp) { return {msg.data(), msg.size()}; }

int main() {
  EnableHook();
  clsn::TcpSever sever("0.0.0.0:5201", 1);
  sever.SetMagCallback(Echo);
  sever.Start(100000);
  return 0;
}