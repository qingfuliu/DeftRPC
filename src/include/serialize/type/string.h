//
// Created by lqf on 23-4-26.
//

#ifndef DEFTRPC_STRING_H
#define DEFTRPC_STRING_H

#include <string>
#include <type_traits>
#include "../detail/helper.h"

namespace clsn {
template <typename Sr>
inline std::enable_if_t<is_binary_serialize<Sr>::value || is_string_serialize<Sr>::value, void>
DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr, const std::string &vec) noexcept {
  sr(MakeSizeTag(static_cast<size_t>(vec.size())));
  if (!vec.empty()) {
    sr.OutPut(static_cast<const void *>(vec.data()), vec.size());
  }
}

template <typename Sr>
inline std::enable_if_t<is_binary_deserialize<Sr>::value || is_string_deserialize<Sr>::value, void>
DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr, std::string &vec) {
  size_t size = 0;
  sr(MakeSizeTag(size));
  if (size == 0) {
    return;
  }
  vec.resize(size);
  sr.Input(static_cast<void *>(vec.data()), size);
}

template <typename Sr>
inline std::enable_if_t<is_binary_serialize<Sr>::value || is_string_serialize<Sr>::value, void>
DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr, const std::string_view vec) noexcept {
  sr(MakeSizeTag(static_cast<size_t>(vec.size())));
  if (!vec.empty()) {
    sr.OutPut(static_cast<const void *>(vec.data()), vec.size());
  }
}

//    template<typename Sr>
//    inline std::enable_if_t<is_binary_deserialize<Sr>::value,
//            void>
//    DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr, std::string_view vec) noexcept {
////        size_t size = 0;
////        sr(make_size_tag(size));
////        if (size == 0) {
////            return;
////        }
////        vec.resize(size);
////        sr.Input(static_cast<void *>(vec.data()), size);
//    }
}  // namespace clsn

#endif  // DEFTRPC_STRING_H
