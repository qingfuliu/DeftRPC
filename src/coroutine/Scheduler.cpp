//
// Created by lqf on 23-4-18.
//

#include "coroutine/Scheduler.h"
#include "common/Exception.h"
#include "common/SharedFunc.h"
#include "common/SpanMutex.h"
#include "coroutine/Coroutine.h"
#include "coroutine/CoroutineContext.h"
#include "coroutine/Poller.h"
#include "coroutine/Timer.h"

namespace clsn {

static thread_local std::unique_ptr<Scheduler> threadScheduler{nullptr};

Scheduler::Scheduler(size_t sharedStackSize, bool UserCall)
    : userCall(UserCall),
      mPid(Thread::thisThreadId()),
      stop(true),
      state(SchedulerState::DoNothing),
      mainCoroutines(clsn::CreateCoroutine()),
      extraTasks(),
      sharedStack(nullptr),
      poller(CreateNewPoller()),
      activeFds(),
      eventFd(CreateEventFd()),
      timerQueue(CreateNewTimerQueue()) {
  coroutines.push_back(mainCoroutines.get());
  mainCoroutines->setIsMain(true);
  if (userCall) {
    if (threadScheduler != nullptr) {
      throw SchedulingException("this thread already has a scheduler");
    }
    threadScheduler.reset(this);

    /******************************event sock******************************/
    poller->RegisterRead(eventFd, std::function<void(void)>([this] { readEventFd(); }));
    /******************************timer  ******************************/
    poller->RegisterRead(timerQueue->GetTimerFd(), *timerQueue);
  }

  if (0 != sharedStackSize) {
    sharedStack = clsn::MakeSharedStack(sharedStackSize);
  }
}

Scheduler::~Scheduler() {
  if (userCall) {
    (void)threadScheduler.release();
  }
  Stop();
}

Scheduler *Scheduler::GetThreadScheduler() noexcept {
  if (!static_cast<bool>(threadScheduler)) {
    threadScheduler = std::make_unique<Scheduler>(-1, false);
  }
  return threadScheduler.get();
}

Coroutine *Scheduler::GetCurCoroutine() {
  Scheduler *scheduler = GetThreadScheduler();

  if (scheduler->coroutines.empty()) {
    throw SchedulingException("There is no running coroutine");
  }

  return scheduler->coroutines.back();
}

Coroutine *Scheduler::GetFatherAndPopCur() noexcept {
  auto scheduler = GetThreadScheduler();
  scheduler->coroutines.pop_back();
  return scheduler->coroutines.back();
}

Poller *Scheduler::GetThreadPoller() noexcept { return GetThreadScheduler()->poller.get(); }

SharedStack *Scheduler::GetThreadSharedStack() noexcept {
  auto scheduler = GetThreadScheduler();
  return scheduler->sharedStack.get();
}

void Scheduler::Start(int timeout) noexcept {
  stop.store(false, std::memory_order_release);
  state.store(SchedulerState::EpollWait, std::memory_order_release);

  while (!stop.load(std::memory_order_acquire)) {
    if (0 < poller->EpollWait(activeFds, timeout)) {
      state.store(SchedulerState::HandleActiveFd, std::memory_order_release);
      for (auto active : activeFds) {
        (*active)();
      }
      state.store(SchedulerState::HandleExtraState, std::memory_order_release);
      while (0 < extraTasks.Size()) {
        auto task = extraTasks.Dequeue();
        task();
      }
      state.store(SchedulerState::EpollWait, std::memory_order_release);
    }
  }
  state.store(SchedulerState::DoNothing, std::memory_order_release);
}

void Scheduler::Stop() noexcept {
  if (!stop.load(std::memory_order_release)) {
    stop.store(true, std::memory_order_release);
    if (state.load(std::memory_order_acquire) == SchedulerState::EpollWait) {
      notify();
    }
  }
}

void Scheduler::readEventFd() const noexcept {
  uint64_t v;
  if (sizeof v != read(eventFd, &v, sizeof v)) {
    CLSN_LOG_ERROR << "readEventFd error!";
  }
}

void Scheduler::writeEventFd() const noexcept {
  uint64_t v = 1;
  if (sizeof v != write(eventFd, &v, sizeof v)) {
    CLSN_LOG_ERROR << "writeEventFd error!";
  }
}

}  // namespace clsn