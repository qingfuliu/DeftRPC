//
// Created by lqf on 23-5-5.
//

#ifndef DEFTRPC_LOCKFREEQUEUE_H
#define DEFTRPC_LOCKFREEQUEUE_H

#include <atomic>

namespace clsn {
template <typename Element>
struct QueueNode {
  using Storage = typename std::aligned_storage<sizeof(Element), std::alignment_of_v<Element>>::type;
  std::atomic<QueueNode *> next;
  Storage element;
};

template <typename Element>
class LockFreeQueue {
  using ElementNodeType = QueueNode<Element>;

 public:
  LockFreeQueue() noexcept : head(), tail() {
    auto newHead = new QueueNode<Element>{};
    head.store(newHead, std::memory_order_release);
    newHead->next.store(nullptr, std::memory_order_release);
    tail.store(newHead, std::memory_order_release);
  }

  ~LockFreeQueue() noexcept {
    while (0 != size.load(std::memory_order_acquire)) {
      Dequeue();
    }
    delete head.load(std::memory_order_acquire);
  }

  size_t Size() const noexcept { return this->size.load(std::memory_order_acquire); }

  template <class T>
  int EnQueue(T &&element) noexcept {
    auto newNode = new ElementNodeType{};
    newNode->next.store(nullptr, std::memory_order_release);
    new (&newNode->element) Element(std::forward<T>(element));

    ElementNodeType *curTail = tail.load(std::memory_order_acquire);
    ElementNodeType *curTailNext = curTail->next.load();
    do {
      if (tail.load(std::memory_order_acquire) == curTail) {
        if (nullptr == curTailNext) {
          ElementNodeType *temp = nullptr;
          if (curTail->next.compare_exchange_strong(temp, newNode, std::memory_order_release)) {
            int res = size.fetch_add(1, std::memory_order_release);
            tail.compare_exchange_strong(curTail, newNode, std::memory_order_release);
            return res;
          }
        } else {
          tail.compare_exchange_strong(curTail, curTailNext, std::memory_order_release);
        }
      }
    } while (true);
  }

  Element Dequeue() noexcept {
    do {
      ElementNodeType *curHead = head.load(std::memory_order_relaxed);
      ElementNodeType *next = curHead->next.load(std::memory_order_relaxed);
      ElementNodeType *curTail = tail.load(std::memory_order_acquire);

      if (head.load(std::memory_order_relaxed) == curHead) {
        if (curTail == head) {
          if (next != nullptr) {
            tail.compare_exchange_strong(curTail, next, std::memory_order_release);
            continue;
          }
          return Element{};
        }
        if (head.compare_exchange_strong(curHead, next, std::memory_order_release)) {
          Element e = *reinterpret_cast<Element *>(&next->element);
          size.fetch_add(-1, std::memory_order_release);
          delete curHead;
          return e;
        }
      }
    } while (true);
  }

 private:
  std::atomic<ElementNodeType *> head;
  std::atomic<ElementNodeType *> tail;
  std::atomic_uint size{0};
};

}  // namespace clsn

#endif  // DEFTRPC_LOCKFREEQUEUE_H
