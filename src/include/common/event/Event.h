//
// Created by lqf on 24-2-09.
//

#ifndef DEFTRPC_EVENT_H
#define DEFTRPC_EVENT_H

#include <sys/epoll.h>
#include <functional>

namespace clsn {
enum class kEvent : uint32_t { Read = EPOLLIN, Write = EPOLLOUT, Error = EPOLLERR };

}  // namespace clsn

#endif  // DEFTRPC_EVENT_H
