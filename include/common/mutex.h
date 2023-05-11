//
// Created by lqf on 23-4-21.
//

#ifndef DEFTRPC_MUTEX_H
#define DEFTRPC_MUTEX_H

#include<type_traits>
#include<atomic>
#include <cassert>
#include "thread.h"

namespace CLSN {

    class SpanMutex;

    class Mutex {
    public:
        Mutex() noexcept = default;

        virtual ~Mutex() = default;

        virtual void Lock() = 0;

        virtual void UnLock() noexcept = 0;

        virtual bool TryLock() noexcept = 0;
    };

    class MutexGuard {
    public:
        explicit MutexGuard(Mutex *m) noexcept:
                mutex(m) {
            assert(nullptr != m);
            mutex->Lock();
        }

        ~MutexGuard() {
            mutex->UnLock();
        }

    public:
        Mutex *mutex;
    };

    template<typename mutex,
            typename v=std::enable_if_t<std::is_base_of_v<Mutex, mutex> > >
    class ReentrantMutex : public mutex {
        using Base = std::decay_t<mutex>;
    public:
        ReentrantMutex() noexcept = default;

        ~ReentrantMutex() override = default;

        void Lock() noexcept override {
            auto temp = threadId.load(std::memory_order_acquire);
            auto thisId = Thread::thisThreadId();
            if (temp == thisId) {
                return;
            }

            if (-1 == temp) {
                Base::Lock();
                threadId.store(thisId, std::memory_order_release);
                return;
            }
        }

        void UnLock() noexcept override {
            threadId.store(-1, std::memory_order_release);
            Base::UnLock();
        }

        bool TryLock() noexcept override {
            auto thisId = Thread::thisThreadId();
            auto temp = threadId.load(std::memory_order_acquire);
            if (temp == thisId) {
                return true;
            }
            if (temp == -1 && Base::TryLock()) {
                threadId.store(thisId, std::memory_order_release);
                return true;
            }
            return false;
        }

    private:
        std::atomic_int threadId{-1};
    };

} // CLSN

#endif //DEFTRPC_MUTEX_H
