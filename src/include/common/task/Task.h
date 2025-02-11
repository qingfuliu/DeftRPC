//
// Created by lqf on 23-4-18.
//

#ifndef DEFTRPC_TASK_H
#define DEFTRPC_TASK_H

#include <sys/epoll.h>
#include <functional>
#include <variant>

namespace clsn {
using Task = std::function<void(void)>;

using Runnable = std::variant<std::monostate, void *, Task>;

struct RunnableContext {
  [[nodiscard]] bool IsNoneEvent() const noexcept { return m_event_ == 0; }

  [[nodiscard]] bool IsReading() const noexcept { return 0 != (m_event_ & EPOLLIN); }

  [[nodiscard]] bool IsWriting() const noexcept { return 0 != (m_event_ & EPOLLOUT); }

  [[nodiscard]] Runnable GetCallBack() const {
    if (IsReading()) {
      return m_read_callback_;
    }
    if (IsWriting()) {
      return m_write_callback_;
    }
    return {};
  }
  int m_fd_ = -1;
  Runnable m_read_callback_;
  Runnable m_write_callback_;
  uint32_t m_event_ = 0;
};
}  // namespace clsn

#endif  // DEFTRPC_TASK_H
