//
// Created by lqf on 23-4-18.
//

#include "coroutine/CoroutineContext.h"
#include <sys/mman.h>
#include "log/Log.h"

extern "C" {
extern void coctx_swap(coctx_t *, coctx_t *) asm("coctx_swap");
};

namespace clsn {

SharedStack::SharedStack(size_t size) : owner(nullptr), stack(nullptr) {
  unsigned long pageSize = sysconf(_SC_PAGESIZE);
  if (size < pageSize) {
    size = pageSize;
  }
  if (static_cast<bool>(size & (pageSize - 1))) {
    size = (size & (pageSize - 1)) + (pageSize);
  } else {
    size += pageSize;
  }
  CLSN_LOG_DEBUG << "share stack size is " << size;
  stack = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (-1 == reinterpret_cast<long>(stack)) {
    CLSN_LOG_FATAL << "mmap failed!";
    throw "";
  }
  stackSize = size;
}

SharedStack::~SharedStack() {
  if (nullptr != stack) {
    if (-1 == munmap(stack, stackSize)) {
      CLSN_LOG_FATAL << "munmap failed!";
    }
    stack = nullptr;
    stackSize = 0;
  }
}

void CoroutineContext::Init() noexcept {
  if (nullptr == sharedMem) {
    if (nullptr == mem.stack_buffer) {
      char *s = reinterpret_cast<char *>(&mem);
      std::fill(s, s + sizeof(stStackMem_t), 0);
      mem.stack_buffer = new char[DefaultStackSize];
      mem.stack_size = DefaultStackSize;
      mem.stack_bp = mem.stack_buffer + mem.stack_size;
    }
  } else {
    mem.stack_buffer = static_cast<char *>(sharedMem->GetStack());
    mem.stack_size = sharedMem->GetStackSize();
    mem.stack_bp = mem.stack_buffer + mem.stack_size;
  }
  makeCtx();
}

void CoroutineContext::makeCtx() noexcept {
  if (sharedMem != nullptr) {
    saveOtherStack();
    sharedMem->SetOwner(this);
  }
  char *s = reinterpret_cast<char *>(&ctx);
  std::fill(s, s + sizeof(coctx_t), 0);

#if defined(__i386__)
  char *bp = mem.stack_bp - sizeof(void *);

  bp = (char *)((unsigned long)bp & -16L);

  void **ret = reinterpret_cast<void **>(bp - (sizeof(void *) << 1));

  *ret = reinterpret_cast<void *>(func);

  *reinterpret_cast<void **>(bp + sizeof(void *)) = reinterpret_cast<void *>(func);

  ctx.regs[ESP] = bp - (sizeof(void *) << 1);
#elif defined(__x86_64__)
  char *sp = mem.stack_bp - sizeof(void *);

  sp = (char *)((unsigned long)sp & -16LL);

  void **ret_addr = (void **)(sp);
  *ret_addr = (void *)func;

  ctx.regs[kRSP] = sp;

  ctx.regs[kRETAddr] = reinterpret_cast<char *>(func);

  ctx.regs[kRDI] = (char *)arg;
#endif
}

void CoroutineContext::saveOtherStack() {
  CoroutineContext *owner = sharedMem->GetOwner();
  if (nullptr != owner && this != owner) {
    owner->saveStackToSavedStack();
  }
}

void CoroutineContext::saveStackToSavedStack() {
  assert(sharedMem->GetOwner() == this);
  mem.validSize = static_cast<size_t>(mem.stack_bp - static_cast<char *>(ctx.regs[kRSP]));
  if (mem.validSize >= mem.saveStackSize) {
    if (0 == mem.saveStackSize) {
      mem.saveStackSize = 2;
    }
    do {
      mem.saveStackSize = mem.saveStackSize << 1;
    } while (mem.validSize >= mem.saveStackSize);

    if (nullptr != mem.savedStack) delete[] reinterpret_cast<char *>(mem.savedStack);
    mem.savedStack = new char[mem.saveStackSize];
  }
  sharedMem->SaveCtxToStack(mem.savedStack, mem.validSize);
  sharedMem->SetOwner(nullptr);
}

void CoroutineContext::loadStackFromSavedStack() { sharedMem->LoadCtxFromStack(mem.savedStack, mem.validSize); }

void CoroutineContext::SwapCtx(CoroutineContext *other) noexcept {
  if (!hasCtx) {
    Init();
    hasCtx = true;
  } else if (sharedMem != nullptr)
    saveOtherStack();

  if (nullptr != sharedMem && this != sharedMem->GetOwner()) {
    loadStackFromSavedStack();
  }
  coctx_swap(&other->ctx, &ctx);
}
}  // namespace clsn
