//
// Created by lqf on 23-4-25.
//

#ifndef DEFTRPC_MEMORY_H
#define DEFTRPC_MEMORY_H

#include <cstdint>
#include <memory>
#include <utility>
#include "../detail/construct.h"
#include "../detail/helper.h"

namespace clsn {
template <typename Sr, typename T>
std::enable_if_t<!has_load_and_construct_v<Sr, T>> DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(
    Sr &sr, const std::shared_ptr<T> &ptr) noexcept {
  id_type id = sr.RegisterPrt(ptr);
  sr(MakeSizeTag(id));
  if (static_cast<bool>(id & PTR_ID_MASK)) {
    sr(*ptr);
  }
}

template <typename Sr, typename T>
std::enable_if_t<!has_load_and_construct_v<Sr, T>> DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(
    Sr &sr, std::shared_ptr<T> &ptr) noexcept(noexcept(access::Construct<T>())) {
  id_type id = 0;
  sr(MakeSizeTag(id));
  if (static_cast<bool>(id & PTR_ID_MASK)) {
    ptr.reset(access::Construct<T>());
    sr.RegisterPrt(ptr);
    sr(*ptr);
    return;
  }
  ptr = std::move(std::static_pointer_cast<T>(sr.GetPrt(id)));
}

template <typename Sr, typename T>
std::enable_if_t<has_load_and_construct_v<Sr, T>> DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(
    Sr &sr, std::shared_ptr<T> &ptr) noexcept(ConstructNavigation(std::declval<Sr>(), std::declval<Construct<T> &>())) {
  id_type id = 0;
  sr(MakeSizeTag(id));
  if (static_cast<bool>(id & PTR_ID_MASK)) {
    using RemoveCVT = std::remove_cv_t<T>;
    using Storage = std::aligned_storage_t<sizeof(RemoveCVT), std::alignment_of_v<RemoveCVT>>;

    auto storage_ptr = new Storage;
    Construct<T> construct(reinterpret_cast<T *>(storage_ptr));
    ConstructNavigation(sr, construct);

    ptr = std::shared_ptr<T>(construct.Get(), [](T *t) {
      t->~T();
      delete reinterpret_cast<Storage *>(t);
    });
    sr.RegisterPtr(ptr);
    return;
  }
  ptr = std::move(std::static_pointer_cast<T>(sr.GetPrt(id)));
}

template <typename Sr, typename T>
void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr, const std::unique_ptr<T> &ptr) noexcept {
  if (!static_cast<bool>(ptr)) {
    sr(MakeSizeTag(static_cast<std::int16_t>(0)));
    return;
  }
  sr(MakeSizeTag(static_cast<std::int16_t>(1)));
  sr(*ptr);
}

template <typename Sr, typename T>
std::enable_if_t<!has_load_and_construct_v<Sr, T>> DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(
    Sr &sr, std::unique_ptr<T> &ptr) noexcept(noexcept(access::Construct<T>())) {
  std::int16_t id;
  sr(MakeSizeTag(id));
  if (static_cast<std::int16_t>(0) != id) {
    ptr.reset(access::Construct<T>());
    sr(*ptr);
    return;
  }
  ptr.reset(nullptr);
}

template <typename Sr, typename T>
std::enable_if_t<has_load_and_construct_v<Sr, T>> DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(
    Sr &sr, std::unique_ptr<T> &ptr) noexcept(ConstructNavigation(std::declval<Sr>(), std::declval<Construct<T> &>())) {
  id_type id = 0;
  sr(MakeSizeTag(id));
  if (static_cast<id_type>(0) != id) {
    using RemoveCVT = std::remove_cv_t<T>;
    using Storage = std::aligned_storage_t<sizeof(RemoveCVT), std::alignment_of_v<RemoveCVT>>;

    auto storage_ptr = new Storage;
    Construct<T> construct(reinterpret_cast<T *>(storage_ptr));
    ptr = std::unique_ptr<T>(construct.Get());
    ConstructNavigation(sr, construct);
    return;
  }
  ptr.reset(nullptr);
}

template <typename Sr, typename T>
void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr, const std::weak_ptr<T> &ptr) noexcept(
    DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(std::declval<Sr &>(), std::declval<std::shared_ptr<T> &>())) {
  auto shared_ptr = ptr.lock();
  DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(sr, shared_ptr);
}

template <typename Sr, typename T>
void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr, std::weak_ptr<T> &ptr) noexcept(
    DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(std::declval<Sr &>(), std::declval<std::shared_ptr<T> &>())) {
  auto shared_ptr = ptr.lock();
  DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(sr, shared_ptr);
}

};  // namespace clsn

#endif  // DEFTRPC_MEMORY_H
