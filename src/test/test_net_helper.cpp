//
// Created by lqf on 23-5-9.
//
//
// Created by lqf on 23-1-15.
//
#include<unistd.h>
#include<iostream>
#include<arpa/inet.h>
#include<string.h>
#include <cerrno>
#include "hook/Hook.h"
#include "net/Socket.h"
#include "net/Addr.h"

int main() {
    Enable_Hook();
    Disable_Enable_Hook();
//    int sock;
//    struct sockaddr_in serv_addr;
//
//    int str_len;
//    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//    if (sock == -1) {
//        std::cout << "Create socket error!!" << std::endl;
//        return 0;
//    }
//    memset(&serv_addr, 0, sizeof(serv_addr));
//    serv_addr.sin_family = AF_INET;
//    serv_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
//    serv_addr.sin_port = htons(5201);
    CLSN::Socket socket1(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    CLSN::Addr addr("0.0.0.0:5201");
//    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
//    if (0 != connect(socket1.getFd(), addr.getSockAddr(), addr.getSockAddrSize())) {
    if (0 != socket1.Connect(&addr)) {
        std::cout << "connect socket error!!" << strerror(errno) << std::endl;
        return 0;
    }

    sleep(1000);

//    close(sock);
}