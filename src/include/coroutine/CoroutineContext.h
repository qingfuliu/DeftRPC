//
// Created by lqf on 23-4-18.
//

#ifndef DEFTRPC_COROUTINECONTEXT_H
#define DEFTRPC_COROUTINECONTEXT_H

#include <unistd.h>
//#include <valgrind/memcheck.h>
#include <algorithm>
#include <memory>
#include "log/Log.h"
class Coctx {
 public:
#if defined(__i386__)
  void *m_regs_[8];
#else
  void *m_regs_[14];
#endif
  [[nodiscard]] bool Empty() const noexcept {
#if defined(__i386__)
    return std::all_of(m_regs_, m_regs_ + 8, [](void *ptr) { return nullptr == ptr; });
#else
    return std::all_of(m_regs_, m_regs_ + 14, [](const void *ptr) { return nullptr == ptr; });
#endif
  }

  void Clear() noexcept {
#if defined(__i386__)
    std::fill(m_regs_, m_regs_ + 8, nullptr);
#else
    std::fill(m_regs_, m_regs_ + 14, nullptr);
#endif
  }
};

extern "C" {
extern void CoctxSwap(Coctx *, Coctx *) asm("CoctxSwap");
};

class StStackMem {
 public:
  StStackMem() = default;

  ~StStackMem() {
    if (m_share_stack_) {
      (void)m_stack_buffer_.release();
    }
  }
  std::unique_ptr<char[]> m_saved_stack_{nullptr};
  size_t m_valid_size_{0};
  size_t m_save_stack_size_{0};

  size_t m_stack_size_{0};                           // 栈大小
  char *m_stack_bp_{nullptr};                        // 栈顶指针 m_stack_buffer_ + m_stack_size_
  std::unique_ptr<char[]> m_stack_buffer_{nullptr};  // 栈内存指针
  bool m_share_stack_{false};

  [[nodiscard]] bool Empty() const noexcept { return nullptr == m_saved_stack_ && nullptr == m_stack_buffer_; }

  void Clear() noexcept {
    m_share_stack_ = false;
    m_saved_stack_.reset(nullptr);
    m_valid_size_ = 0;
    m_save_stack_size_ = 0;
    m_stack_size_ = 0;
    m_stack_bp_ = nullptr;
    m_stack_buffer_.reset(nullptr);
  }
};

namespace clsn {

class CoroutineContext;

class SharedStack {
 public:
  explicit SharedStack(size_t size);

  ~SharedStack();

  [[nodiscard]] CoroutineContext *GetOwner() const noexcept { return m_owner_; }

  void SetOwner(CoroutineContext *c) noexcept { m_owner_ = c; }

  [[nodiscard]] size_t GetStackSize() const noexcept { return m_stack_size_; }

  [[nodiscard]] char *GetStack() const noexcept { return m_stack_; }

  void SaveStack(void *stack, size_t size) const noexcept {
    std::copy(static_cast<const char *>(m_stack_), static_cast<const char *>(m_stack_) + size,
              static_cast<char *>(stack));
  }

  inline void LoadStack(const void *stack, size_t size) const noexcept {
    std::copy(static_cast<const char *>(stack), static_cast<const char *>(stack) + size, m_stack_);
  }

  CoroutineContext *m_owner_{nullptr};
  char *m_stack_{nullptr};
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
  explicit CoroutineContext(CoroutineFunc func, CoroutineArg arg, SharedStack *share_stack = nullptr,
                            bool main_coroutine = false) noexcept;

  explicit CoroutineContext(SharedStack *sharedStack = nullptr) noexcept
      : CoroutineContext(nullptr, nullptr, sharedStack) {}

  ~CoroutineContext() noexcept override;

  inline void Reset() noexcept { m_ctx_.Clear(); }

  void MakeMem();

  void MakeCtx() noexcept;

  inline void SaveOwnerStack() {
    CoroutineContext *owner = m_shared_mem_->GetOwner();
    if (nullptr != owner && this != owner) {
      owner->SaveSharedStack();
    }
  }

  void SaveSharedStack();

  inline void LoadSharedStack() { m_shared_mem_->LoadStack(m_mem_.m_saved_stack_.get(), m_mem_.m_valid_size_); }

  void SwapCtx(CoroutineContext *cur) noexcept;

 private:
  bool m_main_ctx_{false};
  CoroutineFunc m_func_;
  CoroutineArg m_arg_;
  Coctx m_ctx_{};
  StStackMem m_mem_{};
  SharedStack *m_shared_mem_;
  std::int64_t m_valgrind_stack_id_{-1};
};

}  // namespace clsn

#endif  // DEFTRPC_COROUTINECONTEXT_H
