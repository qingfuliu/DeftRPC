//
// Created by lqf on 23-5-9.
//

#include "log/Log.h"
#include"hook/Hook.h"
#include "coroutine/Scheduler.h"
#include "coroutine/Coroutine.h"
#include <netinet/in.h>
#include <chrono>

void test_hook() {

    int acceptFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addr{};
    addr.sin_port = htons(5201);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (0 != bind(acceptFd, reinterpret_cast<sockaddr *>(&addr), sizeof(sockaddr))) {
        CLSN_LOG_DEBUG << "bind failed,error is:" << errno;
        return;
    }

    if (0 != listen(acceptFd, 100)) {
        CLSN_LOG_DEBUG << "listen failed,error is:" << errno;
        return;
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

int main() {
    Enable_Hook();
    CLSN::Scheduler s;
    auto cotoutine = CLSN::CreateCoroutine(test_hook);
    s.DoAfter(std::chrono::seconds{5}, cotoutine.get());

    s.DoAfter(std::chrono::seconds{300}, [&s]() -> void {
        s.Stop();
    });
    s.Start();
    return 0;
}