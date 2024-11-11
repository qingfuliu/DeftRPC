//
// Created by lqf on 23-4-25.
//

#ifndef DEFTRPC_MEMORY_H
#define DEFTRPC_MEMORY_H

#include <memory>
#include "../detail/construct.h"
#include "../detail/helper.h"

namespace CLSN {
template <typename Sr, typename T>
std::enable_if_t<!has_load_and_construct_v<Sr, T>> DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(
    Sr &sr, const std::shared_ptr<T> &ptr) noexcept {
  IdType id = sr.RegisterPrt(ptr);
  sr(make_size_tag(id));
  if (static_cast<bool>(id & PtrIdMask)) {
    sr(*ptr);
  }
}

template <typename Sr, typename T>
std::enable_if_t<!has_load_and_construct_v<Sr, T>> DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(
    Sr &sr, std::shared_ptr<T> &ptr) noexcept(noexcept(access::construct<T>())) {
  IdType id = 0;
  sr(make_size_tag(id));
  if (static_cast<bool>(id & PtrIdMask)) {
    ptr.reset(access::construct<T>());
    sr.RegisterPrt(ptr);
    sr(*ptr);
    return;
  }
  ptr = std::move(std::static_pointer_cast<T>(sr.GetPrt(id)));
}

template <typename Sr, typename T>
std::enable_if_t<has_load_and_construct_v<Sr, T>> DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(
    Sr &sr, std::shared_ptr<T> &ptr) noexcept(ConstructNavigation(std::declval<Sr>(), std::declval<Construct<T> &>())) {
  IdType id = 0;
  sr(make_size_tag(id));
  if (static_cast<bool>(id & PtrIdMask)) {
    using RemoveCVT = std::remove_cv_t<T>;
    using Storage = std::aligned_storage_t<sizeof(RemoveCVT), std::alignment_of_v<RemoveCVT>>;

    auto storagePtr = new Storage;
    Construct<T> mConstruct(reinterpret_cast<T *>(storagePtr));
    ConstructNavigation(sr, mConstruct);

    ptr = std::shared_ptr<T>(mConstruct.get(), [](T *t) {
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
    sr(make_size_tag(static_cast<short>(0)));
    return;
  }
  sr(make_size_tag(static_cast<short>(1)));
  sr(*ptr);
}

template <typename Sr, typename T>
std::enable_if_t<!has_load_and_construct_v<Sr, T>> DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(
    Sr &sr, std::unique_ptr<T> &ptr) noexcept(noexcept(access::construct<T>())) {
  short id;
  sr(make_size_tag(id));
  if (static_cast<short>(0) != id) {
    ptr.reset(access::construct<T>());
    sr(*ptr);
    return;
  }
  ptr.reset(nullptr);
}

template <typename Sr, typename T>
std::enable_if_t<has_load_and_construct_v<Sr, T>> DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(
    Sr &sr, std::unique_ptr<T> &ptr) noexcept(ConstructNavigation(std::declval<Sr>(), std::declval<Construct<T> &>())) {
  IdType id = 0;
  sr(make_size_tag(id));
  if (static_cast<IdType>(0) != id) {
    using RemoveCVT = std::remove_cv_t<T>;
    using Storage = std::aligned_storage_t<sizeof(RemoveCVT), std::alignment_of_v<RemoveCVT>>;

    auto storagePtr = new Storage;
    Construct<T> mConstruct(reinterpret_cast<T *>(storagePtr));
    ptr = std::unique_ptr<T>(mConstruct.get());
    ConstructNavigation(sr, mConstruct);
    return;
  }
  ptr.reset(nullptr);
}

template <typename Sr, typename T>
void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr, const std::weak_ptr<T> &ptr) noexcept(
    DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(std::declval<Sr &>(), std::declval<std::shared_ptr<T> &>())) {
  auto sharedPtr = ptr.lock();
  DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(sr, sharedPtr);
}

template <typename Sr, typename T>
void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr, std::weak_ptr<T> &ptr) noexcept(
    DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(std::declval<Sr &>(), std::declval<std::shared_ptr<T> &>())) {
  auto sharedPtr = ptr.lock();
  DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(sr, sharedPtr);
}

};  // namespace CLSN

#endif  // DEFTRPC_MEMORY_H
