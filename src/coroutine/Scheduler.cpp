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

static thread_local std::unique_ptr<Scheduler> thread_scheduler{nullptr};

Scheduler::Scheduler(size_t sharedStackSize, bool UserCall)
    : m_user_call_(UserCall),
      m_pid_(Thread::thisThreadId()),
      m_stop_(true),
      m_state_(kSchedulerState::DoNothing),
      m_main_coroutine_(clsn::CreateCoroutine()),
      m_shared_stack_(nullptr),
      m_poller_(CreateNewPoller()),
      m_event_fd_(CreateEventFd()),
      m_timer_queue_(CreateNewTimerQueue()) {
  m_coroutines_.push_back(m_main_coroutine_.get());
  m_main_coroutine_->SetIsMain(true);
  if (m_user_call_) {
    if (thread_scheduler != nullptr) {
      throw SchedulingException("this thread already has a scheduler");
    }
    thread_scheduler.reset(this);

    /******************************event m_socket_******************************/
    m_poller_->RegisterRead(m_event_fd_, std::function<void(void)>([this] { ReadEventFd(); }));
    /******************************timer  ******************************/
    m_poller_->RegisterRead(m_timer_queue_->GetTimerFd(), *m_timer_queue_);
  }

  if (0 != sharedStackSize) {
    m_shared_stack_ = clsn::MakeSharedStack(sharedStackSize);
  }
}

Scheduler::~Scheduler() {
  if (m_user_call_) {
    (void)thread_scheduler.release();
  }
}

Scheduler *Scheduler::GetThreadScheduler() noexcept {
  if (!static_cast<bool>(thread_scheduler)) {
    thread_scheduler = std::make_unique<Scheduler>(-1, false);
  }
  return thread_scheduler.get();
}

Coroutine *Scheduler::GetCurCoroutine() {
  Scheduler *scheduler = GetThreadScheduler();

  if (scheduler->m_coroutines_.empty()) {
    throw SchedulingException("There is no running coroutine");
  }

  return scheduler->m_coroutines_.back();
}

Coroutine *Scheduler::GetFatherAndPopCur() noexcept {
  auto scheduler = GetThreadScheduler();
  scheduler->m_coroutines_.pop_back();
  return scheduler->m_coroutines_.back();
}

Poller *Scheduler::GetThreadPoller() noexcept { return GetThreadScheduler()->m_poller_.get(); }

SharedStack *Scheduler::GetThreadSharedStack() noexcept {
  auto scheduler = GetThreadScheduler();
  return scheduler->m_shared_stack_.get();
}

void Scheduler::Start(int timeout) noexcept {
  m_stop_.store(false, std::memory_order_release);
  m_state_.store(kSchedulerState::EpollWait, std::memory_order_release);

  while (!m_stop_.load(std::memory_order_acquire)) {
    if (0 < m_poller_->EpollWait(m_active_fds_, timeout)) {
      m_state_.store(kSchedulerState::HandleActiveFd, std::memory_order_release);
      for (auto active : m_active_fds_) {
        (*active)();
      }
      m_state_.store(kSchedulerState::HandleExtraState, std::memory_order_release);
      while (0 < m_extra_tasks_.Size()) {
        auto task = m_extra_tasks_.Dequeue();
        task();
      }
      m_state_.store(kSchedulerState::EpollWait, std::memory_order_release);
    }
  }
  m_state_.store(kSchedulerState::DoNothing, std::memory_order_release);
}

void Scheduler::Stop() noexcept {
  if (!m_stop_.load(std::memory_order_release)) {
    m_stop_.store(true, std::memory_order_release);
    if (m_state_.load(std::memory_order_acquire) == kSchedulerState::EpollWait) {
      Notify();
    }
  }
}

void Scheduler::ReadEventFd() const noexcept {
  uint64_t v;
  if (sizeof v != read(m_event_fd_, &v, sizeof v)) {
    CLSN_LOG_ERROR << "ReadEventFd error!";
  }
}

void Scheduler::WriteEventFd() const noexcept {
  uint64_t v = 1;
  if (sizeof v != write(m_event_fd_, &v, sizeof v)) {
    CLSN_LOG_ERROR << "WriteEventFd error!";
  }
}

}  // namespace clsn