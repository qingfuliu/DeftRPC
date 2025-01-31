//
// Created by lqf on 23-4-18.
//
#include "coroutine/Coroutine.h"

namespace clsn {

void Coroutine::SwapIn(Coroutine &cur) noexcept { m_ctx_->SwapCtx(cur.m_ctx_.get()); }

Coroutine::Coroutine(Task t, SharedStack *sharedStack, bool main_coroutine) noexcept
    : m_task_(std::move(t)),
      m_ctx_(std::make_unique<CoroutineContext>(&Coroutine::CoroutineFunc, this, sharedStack, main_coroutine)) {}

void Coroutine::CoroutineFunc(void *arg) {
  auto cur = static_cast<Coroutine *>(arg);
  cur->operator()();
  //  cur->m_state_ = kCoroutineState::executing;
  //  if (cur->m_task_ != nullptr) {
  //    cur->m_task_();
  //  }
}

void Coroutine::Reset(Task f) noexcept {
  m_task_ = std::move(f);
  if (kCoroutineState::construct != m_state_) {
    m_ctx_->Reset();
    m_state_ = kCoroutineState::construct;
  }
}

void Coroutine::operator()() {
  this->m_state_ = kCoroutineState::executing;
  if (this->m_task_ != nullptr) {
    this->m_task_();
  }
}

void Coroutine::SwapOut(Coroutine &next) { next.m_ctx_->SwapCtx(m_ctx_.get()); }

}  // namespace clsn
