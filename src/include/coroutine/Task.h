//
// Created by lqf on 23-4-27.
//

#ifndef DEFTRPC_TASK_H
#define DEFTRPC_TASK_H

#include <functional>
#include <type_traits>
#include <utility>
#include <variant>

#include "Coroutine.h"
#include "common/common.h"

namespace clsn {

class Task {
  using TaskType = std::function<void(void)>;

 public:
  Task() noexcept = default;

  Task(TaskType t) noexcept : m_task_(std::move(t)) {}

  Task(Coroutine *t) noexcept : m_task_(t) {}

  Task(const Task &) = default;

  Task(Task &&other) noexcept : m_task_(std::move(other.m_task_)) {}

  template <class Lambda, typename = std::enable_if_t<!std::is_base_of_v<Task, Lambda>>>
  Task(Lambda t) noexcept : Task(std::move(TaskType(std::move(t)))) {}

  virtual ~Task() = default;

  Task &operator=(const Task &) = default;

  Task &operator=(std::nullptr_t) noexcept {
    m_task_ = nullptr;
    return *this;
  }

  Task &operator=(Task &&other) noexcept {
    m_task_ = std::move(other.m_task_);
    return *this;
  }

  void operator()() noexcept {
    switch (m_task_.index()) {
      case 0: {
        auto routine = std::get<Coroutine *>(m_task_);
        if (nullptr != routine) {
          routine->SwapIn();
        }
        break;
      }
      case 1:
        std::get<TaskType>(m_task_)();
        break;
      default:
        break;
    }
  }

  bool operator==(std::nullptr_t) const noexcept {
    if (0 == m_task_.index()) {
      return nullptr == std::get<Coroutine *>(m_task_);
    }
    return nullptr == std::get<TaskType>(m_task_);
  }

  bool operator!=(std::nullptr_t) const noexcept {
    if (0 == m_task_.index()) {
      return nullptr != std::get<Coroutine *>(m_task_);
    }
    return nullptr != std::get<TaskType>(m_task_);
  }

 private:
  std::variant<Coroutine *, TaskType> m_task_{nullptr};
};

inline bool operator==(std::nullptr_t, Task &t) noexcept { return t == nullptr; }

class FileDescriptor {
 public:
  explicit FileDescriptor(int fd) noexcept : m_fd_(fd) {}

  FileDescriptor() noexcept = default;

  ~FileDescriptor() = default;

  FileDescriptor(const FileDescriptor &other) = default;

  FileDescriptor(FileDescriptor &&other) noexcept = default;

  FileDescriptor &operator=(const FileDescriptor &other) noexcept = default;

  FileDescriptor &operator=(FileDescriptor &&other) noexcept = default;

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

  void SetFd(int f) noexcept { m_fd_ = f; }

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

//    class FileDescriptor {
//    public:
//        explicit FileDescriptor(int m_socket_) noexcept: m_socket_(m_socket_), m_event_(0), m_cur_event_(0) {}
//
//        FileDescriptor() noexcept: FileDescriptor(0) {}
//
//        FileDescriptor(const FileDescriptor &other) = default;
//
//        FileDescriptoror() = default;
//
//        FileDescriptor &operator=(const FileDescriptor &other) noexcept = default;
//
//        FileDescriptor &operator=(std::nullptr_t) noexcept {
//            m_socket_ = 0;
//            m_event_ = 0;
//            m_cur_event_ = 0;
//            readTask = nullptr;
//            writeTask = nullptr;
//            errorTask = nullptr;
//        }
//
//        FileDescriptor(FileDescriptor &&other) noexcept = default;
//
//        FileDescriptor &operator=(FileDescriptor &&other) noexcept = default;
//
//        template<class T>
//        FileDescriptor &CombineWith(T &&other) noexcept {
//            uint32_t m_temp_ = m_event_ | other.m_event_;
//            operator=(std::forward<T>(other));
//            m_event_ = m_temp_;
//            return *this;
//        }
//
//        void operator()() noexcept {
//            uint32_t event = m_event_ & m_cur_event_;
//            if (event & static_cast<uint32_t>(kEvent::Read) && !(nullptr == readTask)) {
//                readTask();
//            }
//
//            if (event & static_cast<uint32_t>(kEvent::Write) && !(nullptr == writeTask)) {
//                writeTask();
//            }
//        }
//
//        void SetCurEvent(uint32_t event) noexcept {
//            m_cur_event_ = event;
//        }
//
//        void setReadEvent(Task m_task_) noexcept {
//            readTask = std::move(m_task_);
//            m_event_ |= static_cast<uint32_t>(kEvent::Read);
//        }
//
//        void setWriteEvent(Task m_task_) noexcept {
//            writeTask = std::move(m_task_);
//            m_event_ |= static_cast<uint32_t>(kEvent::Write);
//        }
//
//        void setErrorEvent(Task m_task_) noexcept {
//            errorTask = std::move(m_task_);
//        }
//
//        [[nodiscard]] int GetFd() const noexcept {
//            return m_socket_;
//        }
//
//        [[nodiscard]] uint32_t GetEvent() const noexcept {
//            return m_event_;
//        }
//
//        [[nodiscard]] bool IsNoneEvent() const noexcept {
//            return 0 == m_event_;
//        }
//
//        [[nodiscard]] bool IsReading() const noexcept {
//            return 0 != (m_event_ & static_cast<uint32_t>(kEvent::Read));
//        }
//
//        [[nodiscard]] bool IsWrite() const noexcept {
//            return 0 != (m_event_ & static_cast<uint32_t>(kEvent::Write));
//        }
//
//    private:
//        int m_socket_{0};
//        uint32_t m_event_{0};
//        uint32_t m_cur_event_;
//        Task readTask;
//        Task writeTask;
//        Task errorTask;
//    };

}  // namespace clsn

#endif  // DEFTRPC_TASK_H
