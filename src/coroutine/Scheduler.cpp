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
    : m_stop_(true),
      m_pid_(thread::ThisThreadId()),
      m_state_(kSchedulerState::DoNothing),
      m_main_coroutine_(clsn::CreateCoroutine(nullptr, nullptr, true)),
      m_coroutines_(1024),
      m_shared_stack_(nullptr),
      m_poller_(CreateNewPoller()),
      m_event_fd_(CreateEventFd()),
      m_timer_queue_(CreateNewTimerQueue()) {
  m_coroutines_list_.push_back(m_main_coroutine_.get());

  if (thread_scheduler != nullptr) {
    throw SchedulingException("this thread already has a scheduler");
  }
  thread_scheduler = this;

  if (0 != sharedStackSize) {
    m_shared_stack_ = clsn::MakeSharedStack(sharedStackSize);
  }

  /******************************event fd******************************/
  m_poller_->RegisterRead(m_event_fd_, Task([this] { ReadEventFd(); }));
  /******************************timer  fd******************************/
  m_poller_->RegisterRead(m_timer_queue_->GetTimerFd(), Task([this] { m_timer_queue_->HandleExpireEvent(); }));
}

Scheduler::~Scheduler() {
  m_poller_->CancelRegister(m_event_fd_);
  close(m_event_fd_);
  m_poller_->CancelRegister(m_timer_queue_->GetTimerFd());
}

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

  if (thread_scheduler->m_coroutines_list_.empty()) {
    throw SchedulingException("There is no running coroutine");
  }

  return thread_scheduler->m_coroutines_list_.back();
}

Coroutine *Scheduler::GetPrevCoroutine() {
  if (nullptr == thread_scheduler) {
    throw SchedulingException("There is no running scheduler");
  }

  if (thread_scheduler->m_coroutines_list_.size() < 2) {
    throw SchedulingException("There is no enough coroutine");
  }

  return *std::prev(thread_scheduler->m_coroutines_list_.end(), 2);
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

void Scheduler::RegisterWrite(int fd, Coroutine *coroutine) {
  Scheduler::GetThreadPoller()->RegisterWrite(fd, coroutine);
}

void Scheduler::RegisterRead(int fd, Coroutine *coroutine) {
  Scheduler::GetThreadPoller()->RegisterRead(fd, coroutine);
}

void Scheduler::CancelRegister(int fd) { Scheduler::GetThreadPoller()->CancelRegister(fd); }

void Scheduler::SwapIn(clsn::Coroutine *coroutine) {
  auto scheduler = Scheduler::GetThreadScheduler();
  auto cur_coroutine = scheduler->m_coroutines_list_.back();
  scheduler->m_coroutines_list_.push_back(coroutine);
  coroutine->SwapIn(*cur_coroutine);
}

void Scheduler::Yield() {
  auto scheduler = Scheduler::GetThreadScheduler();
  auto cur_coroutine = scheduler->m_coroutines_list_.back();
  scheduler->m_coroutines_list_.pop_back();
  auto prev_coroutine = scheduler->m_coroutines_list_.back();
  cur_coroutine->SwapOutWithYield(*prev_coroutine);
}

void Scheduler::Terminal() {
  auto scheduler = Scheduler::GetThreadScheduler();
  auto cur_coroutine = scheduler->m_coroutines_list_.back();
  scheduler->m_coroutines_list_.pop_back();
  auto prev_coroutine = scheduler->m_coroutines_list_.back();
  cur_coroutine->SwapOutWithTerminal(*prev_coroutine);
}

void Scheduler::Start(int timeout) noexcept {
  CLSN_LOG_INFO << "Scheduler start!";
  m_stop_.store(false, std::memory_order_release);
  m_state_.store(kSchedulerState::EpollWait, std::memory_order_release);

  while (!m_stop_.load(std::memory_order_acquire)) {
    m_active_runnable_.clear();
    if (0 < m_poller_->EpollWait(m_active_runnable_, timeout)) {
      m_state_.store(kSchedulerState::HandleActiveFd, std::memory_order_release);
      for (auto runnable : m_active_runnable_) {
        switch (runnable->index()) {
          case 1:
            Scheduler::SwapIn(static_cast<Coroutine *>(std::get<1>(*runnable)));
            break;
          case 2:
            std::get<2>(*runnable).operator()();
            break;
          default:
            CLSN_LOG_ERROR << "RunnableContext index is " << runnable->index()
                           << ", which is not registered correctly!";
        }
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

  CLSN_LOG_INFO << "Scheduler exit!";
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

Coroutine *Scheduler::CreateCoroutineEvent(int event_fd, Task task) {
  while (event_fd >= m_coroutines_.size()) {
    m_coroutines_.resize(m_coroutines_.size() << 1);
  }
  m_coroutines_[event_fd] = CreateCoroutine(std::move(task), m_shared_stack_.get());
  return m_coroutines_[event_fd].get();
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
