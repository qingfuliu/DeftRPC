//
// Created by lqf on 23-4-18.
//

#ifndef DEFTRPC_COROUTINE_H
#define DEFTRPC_COROUTINE_H

#include <functional>
#include <memory>
#include <utility>
#include "CoroutineContext.h"

namespace clsn {

class Coroutine : protected Noncopyable {
 public:
  using Task = std::function<void(void)>;

  friend std::unique_ptr<Coroutine> CreateCoroutine(Task t, SharedStack *sharedStack, bool main_coroutine);

  //  void SetIsMain(bool v) noexcept {
  //    if (v) {
  //      m_main_ = v;
  //      m_ctx_->MakeSelfMainCtx();
  //    }
  //  }
  //
  //  [[nodiscard]] bool GetIsMain() const noexcept { return m_main_; }

  void SetTask(Task t) noexcept { m_task_ = std::move(t); }

  //  static Coroutine *GetCurCoroutine();

  //  static void Yield();

  void SwapIn(Coroutine &cur) noexcept;

  //  void SwapOutWithExecuting() {
  //    m_state_ = kCoroutineState::executing;
  //    SwapOut();
  //  }
  //
  //  void SwapOutWithYield() {
  //    m_state_ = kCoroutineState::yield;
  //    SwapOut();
  //  }

  void SwapOutWithFinished(Coroutine &next) {
    m_state_ = kCoroutineState::finished;
    SwapOut(next);
  }

  void SwapOutWithTerminal(Coroutine &next) {
    m_state_ = kCoroutineState::terminal;
    SwapOut(next);
  }

  void Reset(Task f) noexcept;

  //  void operator()() noexcept { SwapIn(); }

  bool operator==(std::nullptr_t) noexcept { return nullptr == m_task_; }

 protected:
  explicit Coroutine(Task t = nullptr, SharedStack *sharedStack = nullptr, bool main_coroutine = false) noexcept
      : m_task_(std::move(t)),
        m_ctx_(std::make_unique<CoroutineContext>(&Coroutine::CoroutineFunc, this, sharedStack, main_coroutine)) {}

 private:
  static void CoroutineFunc(void *arg);

  void SwapOut(Coroutine &c);

 private:
  kCoroutineState m_state_{kCoroutineState::construct};
  std::unique_ptr<CoroutineContext> m_ctx_;
  Task m_task_;
};

inline std::unique_ptr<Coroutine> CreateCoroutine(Coroutine::Task t = nullptr, SharedStack *sharedStack = nullptr,
                                                  bool main_coroutine = false) {
  return std::unique_ptr<Coroutine>(new Coroutine(std::move(t), sharedStack, main_coroutine));
}
}  // namespace clsn

#endif  // DEFTRPC_COROUTINE_H
