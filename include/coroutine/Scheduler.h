//
// Created by lqf on 23-4-18.
//

#ifndef DEFTRPC_SCHEDULER_H
#define DEFTRPC_SCHEDULER_H

#include "common/common.h"
#include "common/thread.h"
#include"common/mutex.h"
#include "coroutine/Timer.h"
#include "coroutine/Task.h"
#include "common/LockFreeQueue.h"
#include<memory>
#include<list>
#include <functional>
#include <unistd.h>
#include <atomic>
#include <chrono>

namespace CLSN {
    class Mutex;

    class SharedStack;

    class Coroutine;

    class Poller;

    class FdDescriptor;

    class Coroutine;

    /**
     * Read  Accept
     * 加入到当前线程的 poller 设置读事件
     * 阻塞 with yield
     *
     * epollWait
     *
     * 解包
     *
     *执行回调
     *
     *
     * Write
     *
     */
    class Scheduler : protected noncopyable {
    public:
        using ExtraTask = Task;

        enum class SchedulerState : short {
            DoNothing = 0,
            EpollWait,
            HandleActiveFd,
            HandleExtraState
        };

        explicit Scheduler(size_t sharedStackSize, bool UserCall = true);

        Scheduler() noexcept: Scheduler(0) {}

        ~Scheduler() override;

        static Scheduler *GetThreadScheduler() noexcept;

        static Coroutine *GetCurCoroutine();

        static Coroutine *GetFatherAndPopCur() noexcept;

        static Poller *GetThreadPoller() noexcept;

        static SharedStack *GetThreadSharedStack() noexcept;

        static bool InsertToTail(Coroutine *routine) noexcept {
            if (nullptr != routine) {
                Scheduler *scheduler = Scheduler::GetThreadScheduler();
                scheduler->coroutines.emplace_back(routine);
                return true;
            }
            return false;
        }

        void AddTask(ExtraTask f) noexcept {
            if (IsInLoopThread()) {
                f();
                return;
            }
            extraTasks.EnQueue(std::move(f));
            if (auto curState = state.load(std::memory_order_acquire);
                    curState == SchedulerState::EpollWait || SchedulerState::DoNothing == curState) {
                notify();
            }
        }

        void AddDefer(ExtraTask f) noexcept {
            extraTasks.EnQueue(std::move(f));
            if (auto curState = state.load(std::memory_order_acquire);
                    curState == SchedulerState::EpollWait || SchedulerState::DoNothing == curState) {
                notify();
            }
        }

        virtual void Start(int timeout = 10000) noexcept;

        virtual void Stop() noexcept;

        [[nodiscard]] bool IsStop() const noexcept {
            return stop.load(std::memory_order_acquire);
        }

        template<typename Rep, typename Period>
        TimerIdType DoAfter(const std::chrono::duration<Rep, Period> &interval, ExtraTask f) {
            return DoEvery(interval, f, false);
        }

        template<typename Clock, typename Dur>
        uint64_t DoUntil(const std::chrono::time_point<Clock, Dur> &timePoint, ExtraTask f) noexcept {
            typename Clock::time_point nowClock = Clock::now();
            auto delta = nowClock - timePoint;
            if (IsInLoopThread()) {
                return timerQueue->AddTimer(delta, std::move(f));
            }

            AddTask([this, delta, f]()mutable {
                timerQueue->AddTimer(delta, std::move(f));
            });
            return -1;
        }


        template<typename Rep, typename Period>
        TimerIdType DoEvery(const std::chrono::duration<Rep, Period> &interval, ExtraTask f, bool repeated = true) {
            if (IsInLoopThread()) {
                return timerQueue->AddTimer(interval, std::move(f), repeated);
            }
            AddTask([this, interval, f, repeated]()mutable {
                timerQueue->AddTimer(interval, std::move(f), repeated);
            });
            return -1;
        }

        void CancelTimer(TimerIdType id) noexcept {
            if (IsInLoopThread()) {
                timerQueue->CancelTimer(id);
                return;
            }
            AddTask([this, id]()mutable {
                timerQueue->CancelTimer(id);
            });
        }

        [[nodiscard]] bool IsInLoopThread() const noexcept {
            return Thread::thisThreadId() == mPid;
        }

    private:

        void notify() noexcept {
            writeEventFd();
        }

        void readEventFd() const noexcept;

        void writeEventFd() const noexcept;

    private:
        const bool userCall;
        const pid_t mPid;
    protected:
        std::atomic_bool stop;
    private:
        std::atomic<SchedulerState> state;

        std::unique_ptr<Coroutine> mainCoroutines;//header of coroutines
        std::list<Coroutine *> coroutines;
        //ExtraTask
        LockFreeQueue<ExtraTask> extraTasks;
        //共享栈
        std::unique_ptr<SharedStack> sharedStack;
        //poller
        std::unique_ptr<Poller> poller;
        std::vector<FdDescriptor *> activeFds;
        //event sock
        int eventFd;
        //timer queue
        std::unique_ptr<TimerQueue> timerQueue;
    };


    class multiThreadScheduler : protected Scheduler {
    private:

    };

} // CLSN

#endif //DEFTRPC_SCHEDULER_H
