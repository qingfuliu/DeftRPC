//
// Created by lqf on 23-4-18.
//

#ifndef DEFTRPC_COROUTINECONTEXT_H
#define DEFTRPC_COROUTINECONTEXT_H

#include <unistd.h>
#include <algorithm>
#include <memory>
#include "common/common.h"
#include "log/Log.h"

struct Coctx {
#if defined(__i386__)
  void *m_regs_[8];
#else
  void *m_regs_[14];
#endif
};

struct StStackMem {
  void *m_saved_stack_;
  size_t m_valid_size_;
  size_t m_save_stack_size_;

  size_t m_stack_size_;
  char *m_stack_bp_;  // m_stack_buffer_ + m_stack_size_
  char *m_stack_buffer_;
};

namespace clsn {

class CoroutineContext;

class SharedStack {
 public:
  explicit SharedStack(size_t size);

  ~SharedStack();

  CoroutineContext *GetOwner() noexcept { return m_owner_; }

  void SetOwner(CoroutineContext *o) noexcept { m_owner_ = o; }

  size_t GetStackSize() const noexcept { return m_stack_size_; }

  void *GetStack() noexcept { return m_stack_; }

  void SaveCtxToStack(void *mStack, size_t size) const noexcept {
    std::copy(static_cast<const char *>(m_stack_), static_cast<const char *>(m_stack_) + size,
              static_cast<char *>(mStack));
  }

  void LoadCtxFromStack(const void *mStack, size_t size) noexcept {
    std::copy(static_cast<const char *>(mStack), static_cast<const char *>(mStack) + size,
              static_cast<char *>(m_stack_));
  }

  CoroutineContext *m_owner_{nullptr};
  void *m_stack_{nullptr};
  size_t m_stack_size_;
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

class CoroutineContext : protected Noncopyable {
 public:
  explicit CoroutineContext(CoroutineFunc Func, CoroutineArg Arg, SharedStack *sharedStack = nullptr) noexcept
      : m_has_ctx_(false), m_func_(Func), m_arg_(Arg), m_shared_mem_(sharedStack) {}

  explicit CoroutineContext(SharedStack *sharedStack = nullptr) noexcept
      : CoroutineContext(nullptr, nullptr, sharedStack) {}

  ~CoroutineContext() noexcept override {
    if (nullptr == m_shared_mem_) {
      delete[] m_mem_.m_stack_buffer_;
    } else {
      delete[] static_cast<char *>(m_mem_.m_saved_stack_);
    }
  }

  void SwapCtx(CoroutineContext *other) noexcept;

  Coctx &GetCtxRef() noexcept { return m_ctx_; }

  StStackMem &GetStackRef() noexcept { return m_mem_; }

  /**
   * 分配内存
   */
  void Init() noexcept;

  bool HasCtx() const noexcept { return m_has_ctx_; }

  void MakeSelfMainCtx() noexcept { m_has_ctx_ = true; }

  void Reset() noexcept { m_has_ctx_ = false; }

 private:
  void MakeCtx() noexcept;

  void SaveOtherStack();

  void SaveStackToSavedStack();

  void LoadStackFromSavedStack();

 private:
  bool m_has_ctx_;
  CoroutineFunc m_func_;
  CoroutineArg m_arg_;
  Coctx m_ctx_{};
  StStackMem m_mem_{};
  SharedStack *m_shared_mem_;
};

}  // namespace clsn

#endif  // DEFTRPC_COROUTINECONTEXT_H
