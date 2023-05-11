//
// Created by lqf on 23-4-29.
//

#ifndef DEFTRPC_TIMER_H
#define DEFTRPC_TIMER_H

#include <chrono>
#include <set>
#include<functional>
#include<atomic>
#include <sys/timerfd.h>
#include<cerrno>

#include"Task.h"

namespace CLSN {

    template<typename Clock, typename Dur>
    using TimePoint = typename std::chrono::time_point<Clock, Dur>;

    class TimeStamp {
    public:
        using ClockType = std::chrono::system_clock;

        using DurationType = std::chrono::nanoseconds;

        using TimeType = TimePoint<ClockType, DurationType>;

        TimeStamp() noexcept: time() {}

        explicit TimeStamp(DurationType interval) noexcept: time(
                std::chrono::time_point_cast<DurationType>(ClockType::now() + interval)) {}

        explicit TimeStamp(TimeType time_) noexcept: time(time_) {}


        TimeStamp(const TimeStamp &) noexcept = default;

        TimeStamp &operator=(const TimeStamp &) noexcept = default;

        template<class Clock, class Duration>
        explicit TimeStamp(TimePoint<Clock, Duration> time_) noexcept: time(
                std::chrono::time_point_cast<DurationType>(time_)) {}

        void AddSecond(std::chrono::seconds seconds) noexcept {
            time += seconds;
        }

        template<typename Rep, typename Period>
        void AddDuration(const std::chrono::duration<Rep, Period> &duration) noexcept {
            time += duration;
        }

        template<typename Rep, typename Period>
        void ResetWithDuration(const std::chrono::duration<Rep, Period> &duration) noexcept {
            time = std::chrono::time_point_cast<DurationType>(ClockType::now()) + duration;
        }

        [[nodiscard]] time_t ToLinuxTime() const noexcept {
            return time.time_since_epoch().count();
        }

        bool operator<(const TimeStamp &other) const noexcept {
            return time < other.time;
        }

        [[nodiscard]] timespec HowMuchFromNow() const noexcept {
            struct timespec res{};
            auto duration = time - std::chrono::time_point_cast<DurationType>(ClockType::now());
            res.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
            res.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() % std::nano::den;
            return res;
        }

        static TimeStamp Max() noexcept {
            return TimeStamp{TimeType::max()};
        }

        static TimeStamp Now() noexcept {
            return TimeStamp{ClockType::now()};
        }

    private:
        TimeType time;
    };


    static inline void resetTimerFd(int timerFd, const TimeStamp &time) {
        struct itimerspec New{};
        struct itimerspec Old{};
        New.it_value = time.HowMuchFromNow();
        int res = timerfd_settime(timerFd, 0, &New, &Old);
        if (-1 == res) {
            CLSN_LOG_FATAL << "timerfd_settime failed,error is " << errno;
        }
    }

    using TimerIdType = int64_t;

    class Timer : public Task {
    public:

        template<typename Rep, typename Period>
        explicit
        Timer(const std::chrono::duration<Rep, Period> &interval_, Task task_, bool repeated_ = false) noexcept:
                Task(std::move(task_)),
                repeated(repeated_),
                interval(std::chrono::duration_cast<TimeStamp::DurationType>(interval_)),
                expireTime(interval),
                mId(Id.fetch_add(1, std::memory_order_release)) {}

        Timer() noexcept: Task(), mId(0), interval(0), expireTime(TimeStamp::ClockType::now()), repeated(false) {}

        Timer(const Timer &timer) = default;

        ~Timer() override = default;

        Timer &operator=(const Timer &) = default;

        void Reset() noexcept {
            expireTime.ResetWithDuration(interval);
        }

        [[nodiscard]] TimerIdType GetId() const noexcept {
            return mId;
        }

        [[nodiscard]] auto &GetTimeStamp() const noexcept {
            return expireTime;
        }

        [[nodiscard]] auto GetRepeated() const noexcept {
            return repeated;
        }

        bool operator<(const Timer &other) const noexcept {
            return expireTime < other.expireTime;
        }

        bool operator==(const Timer &other) const noexcept {
            return other.mId == mId;
        }

    private:
        bool repeated;
        TimeStamp::DurationType interval;

        TimeStamp expireTime;

        TimerIdType mId;
        static inline std::atomic<TimerIdType> Id{1};
    };


    class TimerQueue : public Task {
    public:
        TimerQueue();

        ~TimerQueue() override {
            if (-1 != timerFd) {
                close(timerFd);
            }
        }

        template<typename Rep, typename Period>
        TimerIdType AddTimer(const std::chrono::duration<Rep, Period> &interval, Task task,
                             bool repeated = false
        ) noexcept {
            TimeStamp curHeader = GetEarliestTime();

            Timer timer = Timer(interval, std::move(task), repeated);
            auto pair = timers.insert(std::make_pair(timer.GetId(), timer));
            activeTimers.insert(&pair.first->second);
            TimeStamp afterHeader = GetEarliestTime();

            if (afterHeader < curHeader) {
                resetTimerFd(timerFd, afterHeader);
            }
            return pair.first->second.GetId();
        }

        void CancelTimer(TimerIdType Id) noexcept;

        [[nodiscard]] int GetTimerFd() const noexcept {
            return timerFd;
        }

    private:
        void HandleExpireEvent();

        [[nodiscard]] TimeStamp GetEarliestTime() const noexcept {
            TimeStamp curHeader = TimeStamp::Max();
            if (!activeTimers.empty()) {
                curHeader = (*activeTimers.begin())->GetTimeStamp();
            }
            return curHeader;
        }

    private:
        static inline auto PtrCompare = [](
                const Timer *a,
                const Timer *b
        ) -> bool {
            return *a < *b;
        };
        int timerFd;
        bool handlingEvent;
        std::unordered_map<TimerIdType, Timer> timers;
        std::unordered_set<TimerIdType> cancleTimer;
        std::set<Timer *, decltype(PtrCompare)> activeTimers{PtrCompare};
    };

    inline std::unique_ptr<TimerQueue> CreateNewTimerQueue() {
        return std::make_unique<TimerQueue>();
    }

}

#endif //DEFTRPC_TIMER_H
