//
// Created by lqf on 23-5-9.
//
#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <string.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include "hook/Hook.h"
#include "log/Log.h"

TEST(test_net_sever, test_net_sever) {
  EnableHook();
  int acceptFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  sockaddr_in addr{};
  addr.sin_port = htons(5201);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (0 != bind(acceptFd, reinterpret_cast<sockaddr *>(&addr), sizeof(sockaddr))) {
    CLSN_LOG_DEBUG << "bind failed,error is:" << errno;
  }

  if (0 != listen(acceptFd, 100)) {
    CLSN_LOG_DEBUG << "listen failed,error is:" << errno;
  } else {
    CLSN_LOG_DEBUG << "listen success";
  }

  sockaddr_in caddr{};
  socklen_t len = sizeof(sockaddr_in);
  int fd = accept(acceptFd, reinterpret_cast<sockaddr *>(&caddr), &len);

  if (fd != -1) {
    CLSN_LOG_DEBUG << "accept success";
    close(fd);
  } else {
    CLSN_LOG_DEBUG << "accept error is:" << errno;
  }
}