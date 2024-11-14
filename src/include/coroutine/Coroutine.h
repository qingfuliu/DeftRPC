//
// Created by lqf on 23-4-18.
//

#ifndef DEFTRPC_COROUTINE_H
#define DEFTRPC_COROUTINE_H

#include <functional>
#include <memory>
#include "CoroutineContext.h"
#include "common/common.h"

namespace clsn {

class Coroutine : protected Noncopyable {
 public:
  using Task = std::function<void(void)>;

  friend std::unique_ptr<Coroutine> CreateCoroutine(Task t, SharedStack *sharedStack);

  void SetIsMain(bool v) noexcept {
    if (v) {
      m_main_ = v;
      m_ctx_->MakeSelfMainCtx();
    }
  }

  [[nodiscard]] bool GetIsMain() const noexcept { return m_main_; }

  void SetTask(Task t) noexcept { m_task_ = std::move(t); }

  static Coroutine *GetCurCoroutine();

  static void Yield();

  void SwapIn() noexcept;

  void SwapOutWithExecuting() {
    m_state_ = kCoroutineState::executing;
    SwapOut();
  }

  void SwapOutWithYield() {
    m_state_ = kCoroutineState::yield;
    SwapOut();
  }

  void SwapOutWithFinished() {
    m_state_ = kCoroutineState::finished;
    SwapOut();
  }

  void SwapOutWithTerminal() {
    m_state_ = kCoroutineState::terminal;
    SwapOut();
  }

  void Reset(Task f) noexcept;

  void operator()() noexcept { SwapIn(); }

  bool operator==(std::nullptr_t) noexcept { return nullptr == m_task_; }

 protected:
  explicit Coroutine(Task t = nullptr, SharedStack *sharedStack = nullptr) noexcept
      : m_task_(std::move(t)),
        m_ctx_(std::make_unique<CoroutineContext>(&Coroutine::CoroutineFunc, this, sharedStack)){};

 private:
  static void CoroutineFunc(void *);

  void SwapOut();

 private:
  bool m_main_{false};
  kCoroutineState m_state_{kCoroutineState::construct};
  std::unique_ptr<CoroutineContext> m_ctx_;
  Task m_task_;
};

inline std::unique_ptr<Coroutine> CreateCoroutine(Coroutine::Task t = nullptr, SharedStack *sharedStack = nullptr) {
  return std::unique_ptr<Coroutine>(new Coroutine(std::move(t), sharedStack));
}
}  // namespace clsn

#endif  // DEFTRPC_COROUTINE_H
