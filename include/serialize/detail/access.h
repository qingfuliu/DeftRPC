//
// Created by lqf on 23-4-25.
//

#ifndef DEFTRPC_ACCESS_H
#define DEFTRPC_ACCESS_H

#include "config.h"
#include <type_traits>
#include <bits/move.h>

namespace CLSN {
    namespace access {

        template<class T, class... Args>
        std::enable_if_t<std::is_constructible_v<T, Args...>, T *> construct(T *ptr, Args &&...arg)
        noexcept(std::is_nothrow_constructible_v<T, Args...>) {
            new(ptr) T(std::forward<Args>(arg)...);
        }

        template<class T>
        std::enable_if_t<std::is_constructible_v<T>, T *> construct()
        noexcept(std::is_nothrow_constructible_v<T>) {
            return new T{};
        }

        template<typename Sr, typename T, typename v=
        decltype(std::declval<T>().DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(std::declval<Sr &>()))>
        void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr, T &t)
        noexcept {
            t.DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(sr);
        }

        template<typename Sr, typename T, typename v=
        decltype(std::declval<T>().DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(std::declval<Sr &>()))>
        void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr, T &t)
        noexcept {
            t.DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(sr);
        }

        template<typename Sr, typename T, typename v=
        decltype(std::declval<T>().get()->DEFTRPC_DESERIALIZE_LOAD_AND_CONSTRUCT_FUNCNANE(std::declval<Sr &>(),
                                                                                   std::declval<T &>()))>
        void DEFTRPC_DESERIALIZE_LOAD_AND_CONSTRUCT_FUNCNANE(Sr &sr, T &t)
        noexcept {
            t.get()->DEFTRPC_DESERIALIZE_LOAD_AND_CONSTRUCT_FUNCNANE(sr, t);
        }


    }
}

#endif //DEFTRPC_ACCESS_H
