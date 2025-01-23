//
// Created by lqf on 23-5-5.
//

#ifndef DEFTRPC_LOCKFREEQUEUE_H
#define DEFTRPC_LOCKFREEQUEUE_H

#include <atomic>
#include <utility>

namespace clsn {
template <typename Element>
struct QueueNode {
  using Storage = typename std::aligned_storage_t<sizeof(Element), std::alignment_of_v<Element>>;
  std::atomic<QueueNode *> m_next_;
  Storage m_element_;
};

template <typename Element>
class LockFreeQueue {
  using ElementNodeType = QueueNode<Element>;

 public:
  LockFreeQueue() noexcept : m_head_(), m_tail_() {
    auto new_head = new QueueNode<Element>{};
    m_head_.store(new_head, std::memory_order_release);
    new_head->m_next_.store(nullptr, std::memory_order_release);
    m_tail_.store(new_head, std::memory_order_release);
  }

  ~LockFreeQueue() noexcept {
    while (0 != m_size_.load(std::memory_order_acquire)) {
      Dequeue();
    }
    delete m_head_.load(std::memory_order_acquire);
  }

  [[nodiscard]] std::uint32_t Size() const noexcept { return this->m_size_.load(std::memory_order_acquire); }

  [[nodiscard]] bool Empty() const noexcept { return 0 == this->m_size_.load(std::memory_order_acquire); }
  template <class T>
  int EnQueue(T &&element) noexcept {
    auto new_node = new ElementNodeType{};
    new_node->m_next_.store(nullptr, std::memory_order_release);
    new (&new_node->m_element_) Element(std::forward<T>(element));
    ElementNodeType *cur_tail;
    ElementNodeType *cur_tail_next;
    do {
      cur_tail = m_tail_.load(std::memory_order_acquire);
      cur_tail_next = cur_tail->m_next_.load();
      if (m_tail_.load(std::memory_order_acquire) == cur_tail) {
        if (nullptr == cur_tail_next) {
          ElementNodeType *temp = nullptr;
          if (cur_tail->m_next_.compare_exchange_strong(temp, new_node, std::memory_order_release)) {
            int res = m_size_.fetch_add(1, std::memory_order_release);
            m_tail_.compare_exchange_strong(cur_tail, new_node, std::memory_order_release);
            return res;
          }
        } else {
          m_tail_.compare_exchange_strong(cur_tail, cur_tail_next, std::memory_order_release);
        }
      }
    } while (true);
  }

  Element Dequeue() noexcept {
    do {
      ElementNodeType *cur_head = m_head_.load(std::memory_order_relaxed);
      ElementNodeType *next = cur_head->m_next_.load(std::memory_order_relaxed);
      ElementNodeType *cur_tail = m_tail_.load(std::memory_order_acquire);

      if (m_head_.load(std::memory_order_relaxed) == cur_head) {
        if (cur_tail == m_head_) {
          if (next != nullptr) {
            m_tail_.compare_exchange_strong(cur_tail, next, std::memory_order_release);
            continue;
          }
          return Element{};
        }
        if (m_head_.compare_exchange_strong(cur_head, next, std::memory_order_release)) {
          Element e = *reinterpret_cast<Element *>(&next->m_element_);
          m_size_.fetch_add(-1, std::memory_order_release);
          delete cur_head;
          return e;
        }
      }
    } while (true);
  }

 private:
  std::atomic<ElementNodeType *> m_head_;
  std::atomic<ElementNodeType *> m_tail_;
  std::atomic_uint32_t m_size_{0};
};

}  // namespace clsn

#endif  // DEFTRPC_LOCKFREEQUEUE_H
