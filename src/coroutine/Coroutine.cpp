//
// Created by lqf on 23-4-18.
//
#include "coroutine/Coroutine.h"
#include <cassert>
#include "coroutine/Scheduler.h"

namespace clsn {

Coroutine *Coroutine::GetCurCoroutine() { return Scheduler::GetCurCoroutine(); }

void Coroutine::Yield() {
  auto cur_coroutine = Scheduler::GetCurCoroutine();
  assert(!cur_coroutine->GetIsMain());
  cur_coroutine->SwapOutWithYield();
}

void Coroutine::SwapIn() noexcept {
  Coroutine *cur = Scheduler::GetCurCoroutine();
  Scheduler::InsertToTail(this);

  m_ctx_->SwapCtx(cur->m_ctx_.get());
}

void Coroutine::CoroutineFunc(void *arg) {
  auto cur = static_cast<Coroutine *>(arg);
  assert(kCoroutineState::construct == cur->m_state_);
  try {
    cur->m_state_ = kCoroutineState::executing;
    if (static_cast<bool>(cur->m_task_)) {
      cur->m_task_();
      //                cur->m_task_ = nullptr;
    }
  } catch (std::exception &e) {
    cur->SwapOutWithTerminal();
  } catch (...) {
    cur->SwapOutWithTerminal();
  }
  cur->m_task_ = nullptr;
  cur->SwapOutWithFinished();
}

void Coroutine::Reset(Task f) noexcept {
  m_task_ = std::move(f);
  if (kCoroutineState::construct != m_state_) {
    m_ctx_->Reset();
    m_state_ = kCoroutineState::construct;
  }
}

/** 1. save the context to this
 *      1.1 if has none context,create it
 *      1.2 save the register variables
 *  2. get the scheduler coroutine
 *  3. swap to the scheduler coroutine
 */
void Coroutine::SwapOut() {
  Coroutine *cur = Scheduler::GetCurCoroutine();
  assert(this == cur);
  Coroutine *father = Scheduler::GetFatherAndPopCur();
  father->m_ctx_->SwapCtx(cur->m_ctx_.get());
}

//    void Coroutine::makeCtxInit() noexcept {
//        if (kCoroutineState::construct == state) {
//            m_ctx_.Init();
//            state = kCoroutineState::Init;
//        }
//    }

}  // namespace clsn