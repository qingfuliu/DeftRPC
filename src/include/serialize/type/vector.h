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
inline std::enable_if_t<ConditionalAnd_v<std::is_arithmetic<T>, is_binary_serialize<Sr> >, void>
DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr, const std::vector<T, A> &vec) noexcept {
  sr(make_size_tag(static_cast<size_t>(vec.size())));
  if (!vec.empty()) {
    sr.OutPut(static_cast<const void *>(vec.data()), vec.size() * sizeof(T));
  }
}

template <typename Sr, typename T, typename A>
inline std::enable_if_t<ConditionalAnd_v<std::is_arithmetic<T>, is_binary_deserialize<Sr> >, void>
DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr, std::vector<T, A> &vec) noexcept {
  size_t size = 0;
  sr(make_size_tag(size));
  if (size == 0) {
    return;
  }
  vec.resize(size);
  sr.Input(static_cast<void *>(vec.data()), size * sizeof(T));
}

template <typename Sr, typename T, typename A>
inline std::enable_if_t<!ConditionalOr_v<is_binary_serialize<Sr>, std::is_same<bool, T> >, void>
DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr, const std::vector<T, A> &vec) noexcept {
  sr(make_size_tag(vec.size()));
  for (const auto &val : vec) {
    ar(val);
  }
}

template <typename Sr, typename T, typename A>
inline std::enable_if_t<!ConditionalOr_v<is_binary_deserialize<Sr>, std::is_same<bool, T> >, void>
DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr, std::vector<T, A> &vec) noexcept {
  size_t size = 0;
  sr(make_size_tag(size));
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
  sr(make_size_tag(vec.size()));
  for (bool it : vec) {
    sr(static_cast<bool>(it));
  }
}

template <typename Sr>
inline void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr, std::vector<bool> &vec) noexcept {
  size_t size = 0;
  sr(make_size_tag(size));
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
