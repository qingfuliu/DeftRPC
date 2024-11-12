//
// Created by lqf on 23-4-27.
//

#ifndef DEFTRPC_TASK_H
#define DEFTRPC_TASK_H

#include <functional>
#include <type_traits>
#include <variant>
#include "Coroutine.h"
#include "common/common.h"

namespace clsn {

class Task {
  using TaskType = std::function<void(void)>;

 public:
  Task() noexcept = default;

  Task(TaskType t) noexcept : task(std::move(t)) {}

  Task(Coroutine *t) noexcept : task(t) {}

  Task(const Task &) = default;

  Task(Task &&other) noexcept : task(std::move(other.task)) {}

  template <class Lambda, typename = std::enable_if_t<!std::is_base_of_v<Task, Lambda>>>
  Task(Lambda t) noexcept : Task(std::move(TaskType(std::move(t)))) {}

  virtual ~Task() = default;

  Task &operator=(const Task &) = default;

  Task &operator=(std::nullptr_t) noexcept { task = nullptr; }

  Task &operator=(Task &&other) noexcept { task = std::move(other.task); }

  void operator()() noexcept {
    switch (task.index()) {
      case 0: {
        auto routine = std::get<Coroutine *>(task);
        if (nullptr != routine) {
          routine->swapIn();
        }
        break;
      }
      case 1:
        std::get<TaskType>(task)();
        break;
    }
  }

  bool operator==(std::nullptr_t) const noexcept {
    if (0 == task.index()) {
      return nullptr == std::get<Coroutine *>(task);
    }
    return nullptr == std::get<TaskType>(task);
  }

  bool operator!=(std::nullptr_t) const noexcept {
    if (0 == task.index()) {
      return nullptr != std::get<Coroutine *>(task);
    }
    return nullptr != std::get<TaskType>(task);
  }

 private:
  std::variant<Coroutine *, TaskType> task{nullptr};
};

inline bool operator==(std::nullptr_t, Task &t) noexcept { return t == nullptr; }

class FdDescriptor {
 public:
  explicit FdDescriptor(int fd) noexcept : fd(fd), mEvent(0), curEvent(0) {}

  FdDescriptor() noexcept = default;

  ~FdDescriptor() = default;

  FdDescriptor(const FdDescriptor &other) = default;

  FdDescriptor(FdDescriptor &&other) noexcept = default;

  FdDescriptor &operator=(const FdDescriptor &other) noexcept = default;

  FdDescriptor &operator=(FdDescriptor &&other) noexcept = default;

  FdDescriptor &operator=(std::nullptr_t) noexcept {
    fd = 0;
    mEvent = 0;
    curEvent = 0;
    task = nullptr;
  }

  void operator()() noexcept {
    if (curEvent != mEvent) {
      CLSN_LOG_ERROR << "task should not be execute!";
      return;
    }
    if (task != nullptr) {
      task();
    }
  }

  void setCurEvent(uint32_t event) noexcept { curEvent = event; }

  void SetRead(Task t) noexcept {
    mEvent = static_cast<uint32_t>(Event::Read);
    task = std::move(t);
  }

  void SetWrite(Task t) noexcept {
    mEvent = static_cast<uint32_t>(Event::Write);
    task = std::move(t);
  }

  void SetFd(int f) noexcept { fd = f; }

  [[nodiscard]] int GetFd() const noexcept { return fd; }

  [[nodiscard]] uint32_t GetEvent() const noexcept { return mEvent; }

  [[nodiscard]] bool IsNoneEvent() const noexcept { return 0 == mEvent; }

  [[nodiscard]] bool IsReading() const noexcept { return 0 != (mEvent & static_cast<uint32_t>(Event::Read)); }

  [[nodiscard]] bool IsWrite() const noexcept { return 0 != (mEvent & static_cast<uint32_t>(Event::Write)); }

 private:
  int fd{0};
  uint32_t mEvent{0};
  uint32_t curEvent{0};
  Task task;
};

//    class FdDescriptor {
//    public:
//        explicit FdDescriptor(int sock) noexcept: sock(sock), mEvent(0), curEvent(0) {}
//
//        FdDescriptor() noexcept: FdDescriptor(0) {}
//
//        FdDescriptor(const FdDescriptor &other) = default;
//
//        ~FdDescriptor() = default;
//
//        FdDescriptor &operator=(const FdDescriptor &other) noexcept = default;
//
//        FdDescriptor &operator=(std::nullptr_t) noexcept {
//            sock = 0;
//            mEvent = 0;
//            curEvent = 0;
//            readTask = nullptr;
//            writeTask = nullptr;
//            errorTask = nullptr;
//        }
//
//        FdDescriptor(FdDescriptor &&other) noexcept = default;
//
//        FdDescriptor &operator=(FdDescriptor &&other) noexcept = default;
//
//        template<class T>
//        FdDescriptor &CombineWith(T &&other) noexcept {
//            uint32_t temp = mEvent | other.mEvent;
//            operator=(std::forward<T>(other));
//            mEvent = temp;
//            return *this;
//        }
//
//        void operator()() noexcept {
//            uint32_t event = mEvent & curEvent;
//            if (event & static_cast<uint32_t>(Event::Read) && !(nullptr == readTask)) {
//                readTask();
//            }
//
//            if (event & static_cast<uint32_t>(Event::Write) && !(nullptr == writeTask)) {
//                writeTask();
//            }
//        }
//
//        void setCurEvent(uint32_t event) noexcept {
//            curEvent = event;
//        }
//
//        void setReadEvent(Task task) noexcept {
//            readTask = std::move(task);
//            mEvent |= static_cast<uint32_t>(Event::Read);
//        }
//
//        void setWriteEvent(Task task) noexcept {
//            writeTask = std::move(task);
//            mEvent |= static_cast<uint32_t>(Event::Write);
//        }
//
//        void setErrorEvent(Task task) noexcept {
//            errorTask = std::move(task);
//        }
//
//        [[nodiscard]] int GetFd() const noexcept {
//            return sock;
//        }
//
//        [[nodiscard]] uint32_t GetEvent() const noexcept {
//            return mEvent;
//        }
//
//        [[nodiscard]] bool IsNoneEvent() const noexcept {
//            return 0 == mEvent;
//        }
//
//        [[nodiscard]] bool IsReading() const noexcept {
//            return 0 != (mEvent & static_cast<uint32_t>(Event::Read));
//        }
//
//        [[nodiscard]] bool IsWrite() const noexcept {
//            return 0 != (mEvent & static_cast<uint32_t>(Event::Write));
//        }
//
//    private:
//        int sock{0};
//        uint32_t mEvent{0};
//        uint32_t curEvent;
//        Task readTask;
//        Task writeTask;
//        Task errorTask;
//    };

}  // namespace clsn

#endif  // DEFTRPC_TASK_H
