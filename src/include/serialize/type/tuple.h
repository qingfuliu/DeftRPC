//
// Created by lqf on 23-4-25.
//

#ifndef DEFTRPC_TUPLE_H
#define DEFTRPC_TUPLE_H

#include <tuple>
#include "../detail/helper.h"
#include "serialize/SerializeException.h"

namespace clsn {

    namespace detail {
        template<typename Sr, typename... Args, size_t... Idx>
        inline void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME_HELPER(Sr &sr, const std::tuple<Args...> &tp,
                                                             SequenceIndex<Idx...> &&index) noexcept {
            (void) index;
            sr(std::get<Idx>(tp)...);
        }

        template<typename Sr, typename... Args, size_t... Idx>
        inline void
        DEFTRPC_DESERIALIZE_INPUT_FUNCNAME_HELPER(Sr &sr, std::tuple<Args...> &tp, SequenceIndex<Idx...> &&index) {
            (void) index;
            sr(std::get<Idx>(tp)...);
        }
    }  // namespace detail

    template<typename Sr, typename... Args>
    inline void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr, const std::tuple<Args...> &tp) noexcept {
        sr(MakeSizeTag(sizeof...(Args)));
        DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME_HELPER(sr, tp, MakeSequenceIndex<sizeof...(Args)>());
    }

    template<typename Sr, typename... Args>
    inline void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr, std::tuple<Args...> &tp) {
        size_t size = 0;
        sr(MakeSizeTag(size));
        if (size != sizeof...(Args)) {
            throw std::logic_error(args_size_error);
        }
        if (size != 0) {
            DEFTRPC_DESERIALIZE_INPUT_FUNCNAME_HELPER(sr, tp, MakeSequenceIndex<sizeof...(Args)>());
        }
    }

}  // namespace clsn

#endif  // DEFTRPC_TUPLE_H
