//
// Created by lqf on 23-4-18.
//
#include "coroutine/CoroutineContext.h"
#include <sys/mman.h>
#include "log/Log.h"

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
  m_stack_ = static_cast<char *>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  if (-1 == reinterpret_cast<std::int64_t>(m_stack_)) {
    CLSN_LOG_FATAL << "mmap failed!";
    throw std::runtime_error("mmap failed");
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

CoroutineContext::CoroutineContext(CoroutineFunc func, CoroutineArg arg, SharedStack *share_stack,
                                   bool main_coroutine) noexcept
    : m_main_ctx_(main_coroutine),
      m_func_(func),
      m_arg_(arg),
      m_ctx_(),
      m_mem_(),
      m_shared_mem_(share_stack),
      m_valgrind_stack_id_(-1) {}

CoroutineContext::~CoroutineContext() noexcept {
  if (-1 != m_valgrind_stack_id_) {
    VALGRIND_STACK_DEREGISTER(m_valgrind_stack_id_);
    m_valgrind_stack_id_ = -1;
  }
}

void CoroutineContext::MakeMem() {
  if (nullptr == m_shared_mem_) {
    if (m_mem_.Empty()) {
      // clear
      m_mem_.Clear();
      // malloc
      m_mem_.m_stack_buffer_.reset(new char[DEFAULT_STACK_SIZE]);
      m_mem_.m_stack_size_ = DEFAULT_STACK_SIZE;
      m_mem_.m_stack_bp_ = m_mem_.m_stack_buffer_.get() + m_mem_.m_stack_size_;
      std::fill(m_mem_.m_stack_buffer_.get(), m_mem_.m_stack_buffer_.get() + m_mem_.m_stack_size_, 0);
      // register stack for valgrind
      if (-1 == m_valgrind_stack_id_) {
        m_valgrind_stack_id_ =
            VALGRIND_STACK_REGISTER(m_mem_.m_stack_buffer_.get(), m_mem_.m_stack_buffer_.get() + m_mem_.m_stack_size_);
      } else {
        VALGRIND_STACK_CHANGE(m_valgrind_stack_id_, m_mem_.m_stack_buffer_.get(),
                              m_mem_.m_stack_buffer_.get() + m_mem_.m_stack_size_);
      }
    }
  } else {
    m_mem_.m_stack_buffer_.reset(m_shared_mem_->GetStack());
    m_mem_.m_stack_size_ = m_shared_mem_->GetStackSize();
    m_mem_.m_stack_bp_ = m_mem_.m_stack_buffer_.get() + m_mem_.m_stack_size_;
    m_mem_.m_share_stack_ = true;
  }
}

void CoroutineContext::MakeCtx() noexcept {
  if (m_shared_mem_ != nullptr) {
    SaveOwnerStack();
    m_shared_mem_->SetOwner(this);
  }
  char *s = reinterpret_cast<char *>(&m_ctx_);
  std::fill(s, s + sizeof(Coctx), 0);

#if defined(__i386__)
  char *bp = m_mem_.m_stack_bp_ - sizeof(void *);

  reinterpret_cast<std::uint64_t &>(bp) &= -16L;

  void **ret = reinterpret_cast<void **>(bp - (sizeof(void *) << 1));

  *ret = reinterpret_cast<void *>(m_func_);

  *reinterpret_cast<void **>(bp + sizeof(void *)) = reinterpret_cast<void *>(m_func_);

  m_ctx_.m_regs_[ESP] = bp - (sizeof(void *) << 1);
#elif defined(__x86_64__)
  char *sp = m_mem_.m_stack_bp_ - sizeof(void *);

  reinterpret_cast<std::uint64_t &>(sp) &= -16LL;

  void **ret_addr = reinterpret_cast<void **>(sp);
  *ret_addr = reinterpret_cast<void *>(m_func_);

  m_ctx_.m_regs_[kRSP] = sp;

  m_ctx_.m_regs_[kRETAddr] = reinterpret_cast<char *>(m_func_);

  m_ctx_.m_regs_[kRDI] = static_cast<void *>(m_arg_);
#endif
}

void CoroutineContext::SaveSharedStack() {
  assert(m_shared_mem_->GetOwner() == this);
  m_mem_.m_valid_size_ = static_cast<size_t>(m_mem_.m_stack_bp_ - static_cast<char *>(m_ctx_.m_regs_[kRSP]));
  if (m_mem_.m_valid_size_ >= m_mem_.m_save_stack_size_) {
    if (0 == m_mem_.m_save_stack_size_) {
      m_mem_.m_save_stack_size_ = 2;
    }
    do {
      m_mem_.m_save_stack_size_ = m_mem_.m_save_stack_size_ << 1;
    } while (m_mem_.m_valid_size_ >= m_mem_.m_save_stack_size_);

    m_mem_.m_saved_stack_.reset(new char[m_mem_.m_save_stack_size_]);
    std::fill(m_mem_.m_saved_stack_.get(), m_mem_.m_saved_stack_.get() + m_mem_.m_save_stack_size_, 0);
    VALGRIND_STACK_REGISTER(0, 100);
  }
  m_shared_mem_->SaveStack(m_mem_.m_saved_stack_.get(), m_mem_.m_valid_size_);
  m_shared_mem_->SetOwner(nullptr);
}

void CoroutineContext::SwapCtx(CoroutineContext *cur) noexcept {
  if (m_shared_mem_ != nullptr) {
    SaveOwnerStack();
  }
  // 如果不是主协程，需要创建mem和ctx
  if (!m_main_ctx_) {
    if (m_mem_.Empty()) {
      MakeMem();
    }

    if (m_ctx_.Empty()) {
      MakeCtx();
    }

    if (nullptr != m_shared_mem_ && m_mem_.m_valid_size_ != 0 && this != m_shared_mem_->GetOwner()) {
      LoadSharedStack();
    }
  }
  CoctxSwap(&cur->m_ctx_, &m_ctx_);
}

}  // namespace clsn
