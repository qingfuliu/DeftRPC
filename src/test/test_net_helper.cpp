//
// Created by lqf on 23-5-9.
//
//
// Created by lqf on 23-1-15.
//
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include "hook/Hook.h"
#include "net/Addr.h"
#include "net/Socket.h"

#include <gtest/gtest.h>

TEST(test_net_helper, test_net_helper) {
  EnableHook();
  DisableEnableHook();
  //    int m_socket_;
  //    struct sockaddr_in serv_addr;
  //
  //    int str_len;
  //    m_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  //    if (m_socket_ == -1) {
  //        std::cout << "Create socket error!!" << std::endl;
  //        return 0;
  //    }
  //    memset(&serv_addr, 0, sizeof(serv_addr));
  //    serv_addr.sin_family = AF_INET;
  //    serv_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
  //    serv_addr.sin_port = htons(5201);
  clsn::Socket socket1(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
  clsn::Addr addr("0.0.0.0:5201");
  //    if (connect(m_socket_, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
  //    if (0 != connect(socket1.getFd(), addr.getSockAddr(), addr.getSockAddrSize())) {
  if (0 != socket1.Connect(&addr)) {
    std::cout << "connect socket error!!" << strerror(errno) << std::endl;
  }

  sleep(10);

  //    close(m_socket_);
}