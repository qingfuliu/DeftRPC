//
// Created by lqf on 23-4-18.
//

#ifndef DEFTRPC_COROUTINECONTEXT_H
#define DEFTRPC_COROUTINECONTEXT_H

#include"common/common.h"
#include "log/Log.h"
#include <unistd.h>
#include <algorithm>
#include <memory>

struct coctx_t {
#if defined(__i386__)
    void *regs[ 8 ];
#else
    void *regs[14];
#endif
};


struct stStackMem_t {

    void *savedStack;
    size_t validSize;
    size_t saveStackSize;

    size_t stack_size;
    char *stack_bp; //stack_buffer + stack_size
    char *stack_buffer;
};


namespace CLSN {

    class CoroutineContext;

    class SharedStack {
    public:
        explicit SharedStack(size_t size);

        ~SharedStack();

        CoroutineContext *GetOwner() noexcept {
            return owner;
        }

        void SetOwner(CoroutineContext *o) noexcept {
            owner = o;
        }

        size_t GetStackSize() const noexcept {
            return stackSize;
        }

        void *GetStack() noexcept {
            return stack;
        }

        void SaveCtxToStack(void *mStack, size_t size) const noexcept {
            std::copy(static_cast<const char *>(stack),
                      static_cast<const char *>(stack) + size,
                      static_cast< char *>(mStack));
        }

        void LoadCtxFromStack(const void *mStack, size_t size) noexcept {
            std::copy(static_cast<const char *>(mStack),
                      static_cast<const char *>(mStack) + size,
                      static_cast< char *>(stack));
        }

    private:
        CoroutineContext *owner;
        void *stack;
        size_t stackSize;
    };


    inline std::unique_ptr<SharedStack> MakeSharedStack(size_t size) {
        std::unique_ptr<SharedStack> res{nullptr};
        try {
            res = std::make_unique<SharedStack>(size);
        } catch (...) {
            res.reset(nullptr);
            CLSN_LOG_FATAL << "MakeSharedStack failed!";
        }
        return res;
    }

    class CoroutineContext : protected noncopyable {
    public:
        explicit CoroutineContext(CoroutineFunc Func, CoroutineArg Arg,
                                  SharedStack *sharedStack = nullptr) noexcept:
                hasCtx(false), func(Func), arg(Arg), sharedMem(sharedStack) {}

        explicit CoroutineContext(SharedStack *sharedStack = nullptr) noexcept:
                CoroutineContext(nullptr, nullptr, sharedStack) {}

        ~CoroutineContext() noexcept override {
            if (nullptr == sharedMem) {
                delete[] mem.stack_buffer;
            } else {
                delete[] static_cast<char *>(mem.savedStack);
            }
        }

        void SwapCtx(CoroutineContext *other) noexcept;

        coctx_t &GetCtxRef() noexcept {
            return ctx;
        }

        stStackMem_t &GetStackRef() noexcept {
            return mem;
        }

        /**
         * 分配内存
         */
        void Init() noexcept;


        bool HasCtx() const noexcept {
            return hasCtx;
        }

        void MakeSelfMainCtx() noexcept {
            hasCtx = true;
        }

        void reset() noexcept {
            hasCtx = false;
        }

    private:
        void makeCtx() noexcept;

        void saveOtherStack();

        void saveStackToSavedStack();

        void loadStackFromSavedStack();

    private:
        bool hasCtx;
        CoroutineFunc func;
        CoroutineArg arg;
        coctx_t ctx{};
        stStackMem_t mem{};
        SharedStack *sharedMem;
    };

}


#endif //DEFTRPC_COROUTINECONTEXT_H
