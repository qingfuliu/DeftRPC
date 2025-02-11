//
// Created by lqf on 23-4-24.
//

#ifndef DEFTRPC_SIZE_TAG_H
#define DEFTRPC_SIZE_TAG_H

#include "../detail/helper.h"

namespace clsn {

template <typename Sr, typename T>
void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr, SizeTag<T> &&tag) noexcept {
  sr(tag.m_size_);
}

template <typename Sr, typename T>
void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr, SizeTag<T> &&tag) noexcept {
  sr(tag.m_size_);
}

}  // namespace clsn

#endif  // DEFTRPC_SIZE_TAG_H
