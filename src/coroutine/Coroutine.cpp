//
// Created by lqf on 23-4-18.
//
#include "coroutine/Coroutine.h"
#include <cassert>
#include "coroutine/Scheduler.h"

namespace CLSN {

Coroutine *Coroutine::GetCurCoroutine() { return Scheduler::GetCurCoroutine(); }

void Coroutine::Yield() {
  auto curCoroutine = Scheduler::GetCurCoroutine();
  assert(!curCoroutine->getIsMain());
  curCoroutine->swapOutWithYield();
}

void Coroutine::swapIn() noexcept {
  Coroutine *cur = Scheduler::GetCurCoroutine();
  Scheduler::InsertToTail(this);

  ctx->SwapCtx(cur->ctx.get());
}

void Coroutine::CoroutineFunc(void *arg) {
  auto cur = static_cast<Coroutine *>(arg);
  assert(CoroutineState::construct == cur->state);
  try {
    cur->state = CoroutineState::executing;
    if (static_cast<bool>(cur->task)) {
      cur->task();
      //                cur->task = nullptr;
    }
  } catch (std::exception &e) {
    cur->swapOutWithTerminal();
  } catch (...) {
    cur->swapOutWithTerminal();
  }
  cur->task = nullptr;
  cur->swapOutWithFinished();
}

void Coroutine::reset(Task t) noexcept {
  task = std::move(t);
  if (CoroutineState::construct != state) {
    ctx->reset();
    state = CoroutineState::construct;
  }
}

/** 1. save the context to this
 *      1.1 if has none context,create it
 *      1.2 save the register variables
 *  2. get the scheduler coroutine
 *  3. swap to the scheduler coroutine
 */
void Coroutine::swapOut() {
  Coroutine *cur = Scheduler::GetCurCoroutine();
  assert(this == cur);
  Coroutine *father = Scheduler::GetFatherAndPopCur();
  father->ctx->SwapCtx(cur->ctx.get());
}

//    void Coroutine::makeCtxInit() noexcept {
//        if (CoroutineState::construct == state) {
//            ctx.Init();
//            state = CoroutineState::init;
//        }
//    }

}  // namespace CLSN