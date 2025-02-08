//
// Created by lqf on 23-5-8.
//

#ifndef DEFTRPC_HOOK_H
#define DEFTRPC_HOOK_H

#include <sys/socket.h>
#include <sys/types.h>
#include "common/event/Event.h"
#define HOOK_FUNC_DEFINE(ResType, FuncName, ...)    \
  using FuncName##_func = ResType (*)(__VA_ARGS__); \
  extern FuncName##_func FuncName##_t;

HOOK_FUNC_DEFINE(int, socket, int, int, int);

struct sockaddr;

HOOK_FUNC_DEFINE(int, connect, int, const struct sockaddr *, socklen_t)

HOOK_FUNC_DEFINE(int, accept, int, struct sockaddr *, socklen_t *)

HOOK_FUNC_DEFINE(int, accept4, int, struct sockaddr *, socklen_t *, int flags)

HOOK_FUNC_DEFINE(ssize_t, read, int, void *, size_t)

HOOK_FUNC_DEFINE(ssize_t, write, int, const void *, size_t)

HOOK_FUNC_DEFINE(int, close, int)

HOOK_FUNC_DEFINE(ssize_t, readv, int, const struct iovec *, int)

HOOK_FUNC_DEFINE(ssize_t, writev, int, const struct iovec *, int)

HOOK_FUNC_DEFINE(unsigned int, sleep, unsigned int)

HOOK_FUNC_DEFINE(int, usleep, useconds_t)

HOOK_FUNC_DEFINE(int, nanosleep, const struct timespec *, struct timespec *)

// namespace clsn {
bool IsEnableHook() noexcept;

void EnableHook() noexcept;

void DisableEnableHook() noexcept;
//}

#endif  // DEFTRPC_HOOK_H
