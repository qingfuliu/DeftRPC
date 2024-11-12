//
// Created by lqf on 23-4-21.
//

#ifndef DEFTRPC_THREAD_H
#define DEFTRPC_THREAD_H

#include <sys/syscall.h>
#include <unistd.h>

namespace clsn::Thread {
inline pid_t thisThreadId() noexcept { return static_cast<pid_t>(syscall(SYS_gettid)); }
}  // namespace clsn::Thread

#endif  // DEFTRPC_THREAD_H
