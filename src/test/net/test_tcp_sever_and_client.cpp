//
// Created by lqf on 25-1-23.
//
#include <gtest/gtest.h>
#include <map>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include "net/client/TcpClient.h"
#include "net/sever/TcpSever.h"

TEST(test_tcp_sever_and_client, testTcpSeverAndClient) {
  std::string ipPort = "127.0.0.0:7900";
  std::thread severThread{[&ipPort] {
    clsn::TcpSever sever(ipPort);
    sever.SetMagCallback(
        [](clsn::TcpConnection *, std::string message, clsn::TimeStamp) -> std::string { return message; });
    sever.Start(1000);
  }};
  std::map<std::unique_ptr<clsn::TcpClient>, std::string> clients;

  std::random_device randomDevice;
  unsigned int seed = randomDevice();  // 生成一个随机的种子值
  std::uniform_int_distribution<std::uint32_t> uniform_client_number(1024, 4096);
  std::uniform_int_distribution<char> uniformChar;
  std::mt19937 r(seed);

  std::uint32_t client_number = uniform_client_number(r);
  for (std::uint32_t i = 0; i < client_number; ++i) {
    std::uint32_t size = uniform_client_number(r);
    std::string str;
    str.resize(size, '\0');
    for (int index = 0; index < size; ++index) {
      str[index] = uniformChar(r);
    }
    clients[std::make_unique<clsn::TcpClient>(ipPort)] = str;
  }

  sleep(3);
  for (auto &[client, str] : clients) {
    ASSERT_EQ(true, client->Connect(1000));
    ASSERT_EQ(str.size(), client->Send(str));
  }
  for (auto &[client, str] : clients) {
    ASSERT_EQ(str, client->Receive());
  }
  severThread.join();
}