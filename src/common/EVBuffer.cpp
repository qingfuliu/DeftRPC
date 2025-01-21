//
// Created by lqf on 23-5-13.
//

#include "common/buffer/EVBuffer.h"

namespace clsn {
int EVBuffer::FlushDataToFd(int fd) noexcept {
  int res = static_cast<int>(writev(fd, m_iovecs_.get(), static_cast<int>(m_end_ - m_begin_)));
  Read(res);
  return res;
}
}  // namespace clsn
