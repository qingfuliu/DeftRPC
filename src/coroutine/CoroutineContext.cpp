//
// Created by lqf on 23-4-18.
//

#include "coroutine/CoroutineContext.h"
#include <sys/mman.h>
#include "log/Log.h"

extern "C" {
extern void CoctxSwap(Coctx *, Coctx *) asm("coctx_swap");
};

namespace clsn {

SharedStack::SharedStack(size_t size) {
  std::uint64_t page_size = sysconf(_SC_PAGESIZE);
  if (size < page_size) {
    size = page_size;
  }
  if (static_cast<bool>(size & (page_size - 1))) {
    size = (size & (page_size - 1)) + (page_size);
  } else {
    size += page_size;
  }
  CLSN_LOG_DEBUG << "share stack size is " << size;
  m_stack_ = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (-1 == reinterpret_cast<std::int64_t>(m_stack_)) {
    CLSN_LOG_FATAL << "mmap failed!";
    throw "";
  }
  m_stack_size_ = size;
}

SharedStack::~SharedStack() {
  if (nullptr != m_stack_) {
    if (-1 == munmap(m_stack_, m_stack_size_)) {
      CLSN_LOG_FATAL << "munmap failed!";
    }
    m_stack_ = nullptr;
    m_stack_size_ = 0;
  }
}

void CoroutineContext::Init() noexcept {
  if (nullptr == m_shared_mem_) {
    if (nullptr == m_mem_.m_stack_buffer_) {
      char *s = reinterpret_cast<char *>(&m_mem_);
      std::fill(s, s + sizeof(StStackMem), 0);
      m_mem_.m_stack_buffer_ = new char[DEFAULT_STACK_SIZE];
      m_mem_.m_stack_size_ = DEFAULT_STACK_SIZE;
      m_mem_.m_stack_bp_ = m_mem_.m_stack_buffer_ + m_mem_.m_stack_size_;
    }
  } else {
    m_mem_.m_stack_buffer_ = static_cast<char *>(m_shared_mem_->GetStack());
    m_mem_.m_stack_size_ = m_shared_mem_->GetStackSize();
    m_mem_.m_stack_bp_ = m_mem_.m_stack_buffer_ + m_mem_.m_stack_size_;
  }
  MakeCtx();
}

void CoroutineContext::MakeCtx() noexcept {
  if (m_shared_mem_ != nullptr) {
    SaveOtherStack();
    m_shared_mem_->SetOwner(this);
  }
  char *s = reinterpret_cast<char *>(&m_ctx_);
  std::fill(s, s + sizeof(Coctx), 0);

#if defined(__i386__)
  char *bp = m_mem_.m_stack_bp_ - sizeof(void *);

  bp = (char *)((unsigned long)bp & -16L);

  void **ret = reinterpret_cast<void **>(bp - (sizeof(void *) << 1));

  *ret = reinterpret_cast<void *>(m_func_);

  *reinterpret_cast<void **>(bp + sizeof(void *)) = reinterpret_cast<void *>(m_func_);

  m_ctx_.m_regs_[ESP] = bp - (sizeof(void *) << 1);
#elif defined(__x86_64__)
  char *sp = m_mem_.m_stack_bp_ - sizeof(void *);

  sp = reinterpret_cast<char *>(reinterpret_cast<std::uint64_t>(sp) & -16LL);

  void **ret_addr = reinterpret_cast<void **>(sp);
  *ret_addr = reinterpret_cast<void *>(m_func_);

  m_ctx_.m_regs_[kRSP] = sp;

  m_ctx_.m_regs_[kRETAddr] = reinterpret_cast<char *>(m_func_);

  m_ctx_.m_regs_[kRDI] = static_cast<void *>(m_arg_);
#endif
}

void CoroutineContext::SaveOtherStack() {
  CoroutineContext *owner = m_shared_mem_->GetOwner();
  if (nullptr != owner && this != owner) {
    owner->SaveStackToSavedStack();
  }
}

void CoroutineContext::SaveStackToSavedStack() {
  assert(m_shared_mem_->GetOwner() == this);
  m_mem_.m_valid_size_ = static_cast<size_t>(m_mem_.m_stack_bp_ - static_cast<char *>(m_ctx_.m_regs_[kRSP]));
  if (m_mem_.m_valid_size_ >= m_mem_.m_save_stack_size_) {
    if (0 == m_mem_.m_save_stack_size_) {
      m_mem_.m_save_stack_size_ = 2;
    }
    do {
      m_mem_.m_save_stack_size_ = m_mem_.m_save_stack_size_ << 1;
    } while (m_mem_.m_valid_size_ >= m_mem_.m_save_stack_size_);

    if (nullptr != m_mem_.m_saved_stack_) {
      delete[] reinterpret_cast<char *>(m_mem_.m_saved_stack_);
    }
    m_mem_.m_saved_stack_ = new char[m_mem_.m_save_stack_size_];
  }
  m_shared_mem_->SaveCtxToStack(m_mem_.m_saved_stack_, m_mem_.m_valid_size_);
  m_shared_mem_->SetOwner(nullptr);
}

void CoroutineContext::LoadStackFromSavedStack() {
  m_shared_mem_->LoadCtxFromStack(m_mem_.m_saved_stack_, m_mem_.m_valid_size_);
}

void CoroutineContext::SwapCtx(CoroutineContext *other) noexcept {
  if (!m_has_ctx_) {
    Init();
    m_has_ctx_ = true;
  } else if (m_shared_mem_ != nullptr) {
    SaveOtherStack();
  }

  if (nullptr != m_shared_mem_ && this != m_shared_mem_->GetOwner()) {
    LoadStackFromSavedStack();
  }
  CoctxSwap(&other->m_ctx_, &m_ctx_);
}
}  // namespace clsn
