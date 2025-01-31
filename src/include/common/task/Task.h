//
// Created by root on 1/31/25.
//

#ifndef DEFTRPC_TASK_H
#define DEFTRPC_TASK_H

#include <functional>
#include <type_traits>
#include "log/Log.h"
namespace clsn {

// using Task = std::function<void(void)>;

class Task {
  template <typename T, typename = void>
  struct ISRunnable : public std::false_type {};

  template <typename T>
  struct ISRunnable<T, std::void_t<decltype(&T::operator())>>
      : std::is_member_function_pointer<decltype(&T::operator())> {};

 public:
  Task() = default;

  Task(const Task &task) noexcept : m_task_(task.m_task_) {}

  Task(Task &&task) noexcept : m_task_(std::move(task.m_task_)) {}

  Task(std::function<void(void)> task) : m_task_(std::move(task)) {}

  Task(std::nullptr_t) : m_task_(nullptr) {}

  template <class T, typename = std::enable_if_t<ISRunnable<std::remove_pointer_t<T>>::value, void>>
  Task(T task) : m_task_(std::bind(&std::remove_pointer_t<T>::operator(), task)) {}

  ~Task() = default;

  Task &operator=(const Task &) = default;

  Task &operator=(Task &&task) noexcept {
    m_task_ = std::move(task.m_task_);
    return *this;
  }

  Task &operator=(std::nullptr_t) noexcept {
    m_task_ = nullptr;
    return *this;
  }

  bool operator==(std::nullptr_t) noexcept { return nullptr == m_task_; }

  bool operator!=(std::nullptr_t) noexcept { return nullptr != m_task_; }

  void operator()() { m_task_(); }

 private:
  std::function<void(void)> m_task_;
};

class FileDescriptor {
 public:
  explicit FileDescriptor(int fd) noexcept : m_fd_(fd) {}

  FileDescriptor() noexcept = default;

  ~FileDescriptor() = default;

  FileDescriptor(const FileDescriptor &other) = default;

  FileDescriptor(FileDescriptor &&other) noexcept = default;

  FileDescriptor &operator=(const FileDescriptor &other) noexcept = default;

  FileDescriptor &operator=(FileDescriptor &&other) noexcept {
    m_fd_ = other.m_fd_;
    m_event_ = other.m_event_;
    m_cur_event_ = other.m_cur_event_;
    m_task_ = std::move(other.m_task_);
    return *this;
  }

  FileDescriptor &operator=(std::nullptr_t) noexcept {
    m_fd_ = 0;
    m_event_ = 0;
    m_cur_event_ = 0;
    m_task_ = nullptr;
    return *this;
  }

  void operator()() noexcept {
    if (m_cur_event_ != m_event_) {
      CLSN_LOG_ERROR << "m_task_ should not be execute!";
      return;
    }
    if (m_task_ != nullptr) {
      m_task_();
    }
  }

  void SetCurEvent(uint32_t event) noexcept { m_cur_event_ = event; }

  void SetRead(Task t) noexcept {
    m_event_ = static_cast<uint32_t>(kEvent::Read);
    m_task_ = std::move(t);
  }

  void SetWrite(Task t) noexcept {
    m_event_ = static_cast<uint32_t>(kEvent::Write);
    m_task_ = std::move(t);
  }

  [[nodiscard]] int GetFd() const noexcept { return m_fd_; }

  [[nodiscard]] uint32_t GetEvent() const noexcept { return m_event_; }

  [[nodiscard]] bool IsNoneEvent() const noexcept { return 0 == m_event_; }

  [[nodiscard]] bool IsReading() const noexcept { return 0 != (m_event_ & static_cast<uint32_t>(kEvent::Read)); }

  [[nodiscard]] bool IsWrite() const noexcept { return 0 != (m_event_ & static_cast<uint32_t>(kEvent::Write)); }

 private:
  int m_fd_{0};
  uint32_t m_event_{0};
  uint32_t m_cur_event_{0};
  Task m_task_;
};
}  // namespace clsn

#endif  // DEFTRPC_TASK_H
