//
// Created by lqf on 23-4-18.
//

#include "coroutine/Scheduler.h"
#include "common/Exception.h"
#include "common/SharedFunc.h"
#include "common/SpanMutex.h"
#include "common/timer/Timer.h"
#include "coroutine/Coroutine.h"
#include "coroutine/CoroutineContext.h"
#include "coroutine/Poller.h"

namespace clsn {

static thread_local Scheduler *thread_scheduler{nullptr};

Scheduler::Scheduler(size_t sharedStackSize)
    : m_pid_(thread::ThisThreadId()),
      m_stop_(true),
      m_state_(kSchedulerState::DoNothing),
      m_main_coroutine_(clsn::CreateCoroutine(nullptr, nullptr, true)),
      m_shared_stack_(nullptr),
      m_poller_(CreateNewPoller()),
      m_event_fd_(CreateEventFd()),
      m_timer_queue_(CreateNewTimerQueue()) {
  m_coroutines_.push_back(m_main_coroutine_.get());

  if (thread_scheduler != nullptr) {
    throw SchedulingException("this thread already has a scheduler");
  }
  thread_scheduler = this;

  /******************************event m_socket_******************************/
  m_poller_->RegisterRead(m_event_fd_, std::function<void(void)>([this] { ReadEventFd(); }));
  /******************************timer  ******************************/
  m_poller_->RegisterRead(m_timer_queue_->GetTimerFd(),
                          std::function<void(void)>([this] { m_timer_queue_->HandleExpireEvent(); }));

  if (0 != sharedStackSize) {
    m_shared_stack_ = clsn::MakeSharedStack(sharedStackSize);
  }
}

Scheduler::~Scheduler() = default;

Scheduler *Scheduler::GetThreadScheduler() {
  if (nullptr == thread_scheduler) {
    throw SchedulingException("There is no running scheduler");
  }

  return thread_scheduler;
}

Coroutine *Scheduler::GetCurCoroutine() {
  if (nullptr == thread_scheduler) {
    throw SchedulingException("There is no running scheduler");
  }

  if (thread_scheduler->m_coroutines_.empty()) {
    throw SchedulingException("There is no running coroutine");
  }

  return thread_scheduler->m_coroutines_.back();
}

Coroutine *Scheduler::GetPrevCoroutine() {
  if (nullptr == thread_scheduler) {
    throw SchedulingException("There is no running scheduler");
  }

  if (thread_scheduler->m_coroutines_.size() < 2) {
    throw SchedulingException("There is no enough coroutine");
  }

  return *std::prev(thread_scheduler->m_coroutines_.end(), 2);
}

SharedStack *Scheduler::GetThreadSharedStack() {
  if (nullptr == thread_scheduler) {
    throw SchedulingException("There is no running scheduler");
  }

  return thread_scheduler->m_shared_stack_.get();
}

Poller *Scheduler::GetThreadPoller() {
  if (nullptr == thread_scheduler) {
    throw SchedulingException("There is no running scheduler");
  }

  return thread_scheduler->m_poller_.get();
}

void Scheduler::RegisterWrite(int fd, Task task) { Scheduler::GetThreadPoller()->RegisterWrite(fd, std::move(task)); }

void Scheduler::RegisterRead(int fd, Task task) { Scheduler::GetThreadPoller()->RegisterRead(fd, std::move(task)); }

void Scheduler::CancelRegister(int fd) { Scheduler::GetThreadPoller()->CancelRegister(fd); }

void Scheduler::SwapIn(clsn::Coroutine *coroutine) { coroutine->SwapIn(*Scheduler::GetCurCoroutine()); }

void Scheduler::Yield() {
  auto cur_coroutine = Scheduler::GetCurCoroutine();
  cur_coroutine->SwapOutWithYield(*Scheduler::GetPrevCoroutine());
}

void Scheduler::Terminal() {
  auto cur_coroutine = Scheduler::GetCurCoroutine();
  cur_coroutine->SwapOutWithTerminal(*Scheduler::GetPrevCoroutine());
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
  if (nullptr != m_stop_callback_) {
    m_stop_callback_();
  }

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

void MultiThreadScheduler::Start(int timeout) noexcept {
  {
    std::unique_lock<std::mutex> guard(m_mutex_);
    for (auto &scheduler : m_schedulers_) {
      scheduler = std::make_unique<SchedulerThread>();
      scheduler->Start(timeout);
    }
  }
  Scheduler::Start(timeout);
}

void MultiThreadScheduler::Stop() noexcept {
  for (auto &scheduler : m_schedulers_) {
    if (scheduler != nullptr) {
      scheduler->Stop();
    }
  }
  Scheduler::Stop();
}

}  // namespace clsn