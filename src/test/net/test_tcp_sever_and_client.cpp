//
// Created by lqf on 25-1-23.
//
#include <gtest/gtest.h>
#include <map>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include "hook/Hook.h"
#include "net/client/TcpClient.h"
#include "net/sever/TcpSever.h"

TEST(test_tcp_sever_and_client, testTcpSeverAndClient) {
  EnableHook();
  DisableEnableHook();
  clsn::TcpSever *exter_sever;
  std::thread severThread{[&exter_sever] {
    EnableHook();
    std::string ipPort = "0.0.0.0:7901";
    clsn::TcpSever sever(ipPort, 8, 0);
    exter_sever = &sever;
    sever.SetMagCallback(
        [](clsn::TcpConnection *, std::string message, clsn::TimeStamp) -> std::string { return message; });
    sever.Start(100000);
  }};
  sleep(3);
  std::map<std::unique_ptr<clsn::TcpClient>, std::string> clients;

  std::random_device randomDevice;
  unsigned int seed = randomDevice();  // 生成一个随机的种子值
  std::uniform_int_distribution<std::uint32_t> uniform_client_number(1024, 4096);
  std::uniform_int_distribution<char> uniformChar;
  std::mt19937 r(seed);

  std::uint32_t client_number = 10000;  // uniform_client_number(r);

  for (std::uint32_t i = 0; i < client_number; ++i) {
    std::uint32_t size = uniform_client_number(r);
    std::string str;
    str.resize(size, '\0');
    for (int index = 0; index < size; ++index) {
      str[index] = uniformChar(r);
    }
    std::string ipPort = "127.0.0.1:7901";
    clients[std::make_unique<clsn::TcpClient>(ipPort)] = str;
  }

  for (auto &[client, str] : clients) {
    ASSERT_EQ(true, client->Connect(1000));
    ASSERT_EQ(str.size(), client->Send(str));
    auto reveive_str = client->Receive();
    ASSERT_EQ(str, reveive_str);
    client->Close();
  }
  CLSN_LOG_DEBUG << "client size :" << clients.size();

  exter_sever->Stop();
  severThread.join();
}