//
// Created by lqf on 23-4-25.
//

#ifndef DEFTRPC_CONSTRUCT_H
#define DEFTRPC_CONSTRUCT_H

#include <bits/move.h>
#include "helper.h"

namespace CLSN {

template <typename T>
class Construct {
 public:
  explicit Construct(T *p) noexcept : ptr(p) {}

  ~Construct() = default;

  template <typename... Args>
  T *operator()(Args &&...args) noexcept(access::construct(std::declval<T *>(), std::declval<Args>()...)) {
    access::construct(ptr, std::forward<Args>(args)...);
    return ptr;
  }

  T *operator->() noexcept { return ptr; }

  T *get() noexcept { return ptr; }

 private:
  T *ptr;
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

}  // namespace CLSN

#endif  // DEFTRPC_CONSTRUCT_H
