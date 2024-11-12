//
// Created by lqf on 23-5-13.
//

#include "net/EVBuffer.h"

namespace clsn {
int EVBuffer::WriteToFd(int fd) noexcept {
  int res = static_cast<int>(writev(fd, iovecs.get(), static_cast<int>(end - begin)));
  Read(res);
  return res;
}
}  // namespace clsn