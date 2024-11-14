//
// Created by lqf on 23-5-11.
//
#include "hook/Hook.h"
#include "log/Log.h"
#include "net/client/TcpClient.h"

#include <gtest/gtest.h>

TEST(test_tcpClient, test_tcpClient) {
  EnableHook();
  DisableEnableHook();

  clsn::TcpClient client("0.0.0.0:5201");
  CLSN_LOG_DEBUG << "remote:" << client.GetRemote().toString();
  if (!client.Connect()) {
    CLSN_LOG_ERROR << "connect failed,error is " << strerror(errno);
  }
  std::string str = "test str";
  if (0 >= client.Send(str)) {
    CLSN_LOG_ERROR << "Send failed,error is " << strerror(errno);
  }
  int res = 0;

  std::string_view temp = client.Receive();
  if (temp.empty()) {
    CLSN_LOG_ERROR << "Receive failed,error is " << strerror(errno);
  }

  CLSN_LOG_DEBUG << "receive str is " << temp;
  client.Close();
};