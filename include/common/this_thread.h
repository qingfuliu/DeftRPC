//
// Created by lqf on 23-5-9.
//

#ifndef DEFTRPC_THIS_THREAD_H
#define DEFTRPC_THIS_THREAD_H

#include "coroutine/Task.h"
#include"coroutine/Poller.h"
#include "coroutine/Scheduler.h"

namespace CLSN::this_thread {

    inline void RegisterWrite(int fd, Task task) noexcept {
        Scheduler::GetThreadPoller()->RegisterWrite(fd, std::move(task));
    }

    inline void RegisterRead(int fd, Task task) noexcept {
        Scheduler::GetThreadPoller()->RegisterRead(fd, std::move(task));
    }

    inline void CancelRegister(int fd) noexcept {
        Scheduler::GetThreadPoller()->CancelRegister(fd);
    }
}
#endif //DEFTRPC_THIS_THREAD_H
