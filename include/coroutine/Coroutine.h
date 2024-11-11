//
// Created by lqf on 23-4-18.
//

#ifndef DEFTRPC_COROUTINE_H
#define DEFTRPC_COROUTINE_H

#include <functional>
#include <memory>
#include "CoroutineContext.h"
#include "common/common.h"

namespace CLSN {

class Coroutine : protected noncopyable {
 public:
  using Task = std::function<void(void)>;

  friend std::unique_ptr<Coroutine> CreateCoroutine(Task t, SharedStack *sharedStack);

  void setIsMain(bool v) noexcept {
    if (v) {
      isMain = v;
      ctx->MakeSelfMainCtx();
    }
  }

  [[nodiscard]] bool getIsMain() const noexcept { return isMain; }

  void setTask(Task t) noexcept { task = std::move(t); }

  static Coroutine *GetCurCoroutine();

  static void Yield();

  void swapIn() noexcept;

  void swapOutWithExecuting() {
    state = CoroutineState::executing;
    swapOut();
  }

  void swapOutWithYield() {
    state = CoroutineState::yield;
    swapOut();
  }

  void swapOutWithFinished() {
    state = CoroutineState::finished;
    swapOut();
  }

  void swapOutWithTerminal() {
    state = CoroutineState::terminal;
    swapOut();
  }

  void reset(Task f) noexcept;

  void operator()() noexcept { swapIn(); }

  bool operator==(std::nullptr_t) noexcept { return nullptr == task; }

 protected:
  explicit Coroutine(Task t = nullptr, SharedStack *sharedStack = nullptr) noexcept
      : isMain(false),
        state(CoroutineState::construct),
        task(std::move(t)),
        ctx(std::make_unique<CoroutineContext>(&Coroutine::CoroutineFunc, this, sharedStack)){};

 private:
  static void CoroutineFunc(void *);

  void swapOut();

 private:
  bool isMain;
  CoroutineState state;
  std::unique_ptr<CoroutineContext> ctx;
  Task task;
};

inline std::unique_ptr<Coroutine> CreateCoroutine(Coroutine::Task t = nullptr, SharedStack *sharedStack = nullptr) {
  return std::unique_ptr<Coroutine>(new Coroutine(std::move(t), sharedStack));
}
}  // namespace CLSN

#endif  // DEFTRPC_COROUTINE_H
