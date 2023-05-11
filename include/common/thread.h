//
// Created by lqf on 23-4-21.
//

#ifndef DEFTRPC_THREAD_H
#define DEFTRPC_THREAD_H

#include <unistd.h>
#include <sys/syscall.h>

namespace CLSN::Thread {
    inline pid_t thisThreadId() noexcept {
        return static_cast<pid_t>(syscall(SYS_gettid));
    }
}

#endif //DEFTRPC_THREAD_H
