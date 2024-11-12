//
// Created by lqf on 23-5-6.
//

#ifndef DEFTRPC_SHAREDFUNC_H
#define DEFTRPC_SHAREDFUNC_H

#include <sys/eventfd.h>
#include "log/Log.h"

/******************************event sock******************************/
inline int CreateEventFd() noexcept {
  int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (-1 == fd) {
    CLSN_LOG_DEBUG << "CreateEventFd failed!";
  }
  return fd;
}

#endif  // DEFTRPC_SHAREDFUNC_H
