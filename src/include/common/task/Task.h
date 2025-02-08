//
// Created by lqf on 23-4-18.
//

#ifndef DEFTRPC_TASK_H
#define DEFTRPC_TASK_H

#include <functional>
#include <variant>
namespace clsn {
using Task = std::function<void(void)>;

struct Runnable {
  int m_fd_ = -1;
  std::variant<std::monostate, void *, Task> m_runnable_;
  uint32_t m_event_ = 0;
};
}  // namespace clsn

#endif  // DEFTRPC_TASK_H
