//
// Created by lqf on 23-4-25.
//

#ifndef DEFTRPC_CONSTRUCT_H
#define DEFTRPC_CONSTRUCT_H

#include <bits/move.h>
#include "helper.h"

namespace clsn {

template <typename T>
class Construct {
 public:
  explicit Construct(T *p) noexcept : m_ptr_(p) {}

  ~Construct() = default;

  template <typename... Args>
  T *operator()(Args &&...args) noexcept(access::Construct(std::declval<T *>(), std::declval<Args>()...)) {
    access::Construct(m_ptr_, std::forward<Args>(args)...);
    return m_ptr_;
  }

  T *operator->() noexcept { return m_ptr_; }

  T *Get() noexcept { return m_ptr_; }

 private:
  T *m_ptr_;
};

template <typename Sr, typename T>
std::enable_if_t<has_no_member_load_and_construct_func_v<Sr, T>, void> ConstructNavigation(
    Sr &sr, Construct<T> &mConstruct) noexcept(DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(std::declval<Sr &>(),
                                                                                 std::declval<Construct<T> &>())) {
  DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(sr, mConstruct);
}

template <typename Sr, typename T>
std::enable_if_t<has_member_load_and_construct_func_v<Sr, T>, void> ConstructNavigation(
    Sr &sr,
    Construct<T> &mConstruct) noexcept(access::DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(std::declval<Sr &>(),
                                                                                 std::declval<Construct<T> &>())) {
  access::DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(sr, mConstruct);
}

}  // namespace clsn

#endif  // DEFTRPC_CONSTRUCT_H
