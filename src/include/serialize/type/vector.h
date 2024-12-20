//
// Created by lqf on 23-4-24.
//

#ifndef DEFTRPC_VECTOR_H
#define DEFTRPC_VECTOR_H

#include <type_traits>
#include <vector>
#include "../detail/helper.h"

namespace clsn {

template <typename Sr, typename T, typename A>
inline std::enable_if_t<CONDITIONAL_AND_V<std::is_arithmetic<T>, IsBinarySerialize<Sr> >, void>
DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr, const std::vector<T, A> &vec) noexcept {
  sr(MakeSizeTag(static_cast<size_t>(vec.size())));
  if (!vec.empty()) {
    sr.OutPut(static_cast<const void *>(vec.data()), vec.size() * sizeof(T));
  }
}

template <typename Sr, typename T, typename A>
inline std::enable_if_t<CONDITIONAL_AND_V<std::is_arithmetic<T>, IsBinaryDeserialize<Sr> >, void>
DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr, std::vector<T, A> &vec) noexcept {
  size_t size = 0;
  sr(MakeSizeTag(size));
  if (size == 0) {
    return;
  }
  vec.resize(size);
  sr.Input(static_cast<void *>(vec.data()), size * sizeof(T));
}

template <typename Sr, typename T, typename A>
inline std::enable_if_t<!CONDITIONAL_OR_V<IsBinarySerialize<Sr>, std::is_same<bool, T> >, void>
DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr, const std::vector<T, A> &vec) noexcept {
  sr(MakeSizeTag(vec.size()));
  for (const auto &val : vec) {
    ar(val);
  }
}

template <typename Sr, typename T, typename A>
inline std::enable_if_t<!CONDITIONAL_OR_V<IsBinaryDeserialize<Sr>, std::is_same<bool, T> >, void>
DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr, std::vector<T, A> &vec) noexcept {
  size_t size = 0;
  sr(MakeSizeTag(size));
  if (size == 0) {
    return;
  }
  vec.resize(size);
  for (auto &val : vec) {
    ar(val);
  }
}

template <typename Sr>
inline void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr, const std::vector<bool> &vec) noexcept {
  sr(MakeSizeTag(vec.size()));
  for (bool it : vec) {
    sr(static_cast<bool>(it));
  }
}

template <typename Sr>
inline void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr, std::vector<bool> &vec) noexcept {
  size_t size = 0;
  sr(MakeSizeTag(size));
  if (size == 0) {
    return;
  }
  vec.resize(size);
  bool temp;
  for (size_t index = 0; index < size; ++index) {
    sr(temp);
    vec[index] = temp;
  }
}

}  // namespace clsn

#endif  // DEFTRPC_VECTOR_H
