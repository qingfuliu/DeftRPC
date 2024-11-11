//
// Created by lqf on 23-4-21.
//

#ifndef DEFTRPC_TYPE_H
#define DEFTRPC_TYPE_H

#include <unistd.h>
#include <type_traits>

namespace CLSN {
namespace detail {
template <size_t Idx, class Header, class... InnerArgs>
struct get_index_type_Impl {
  static_assert(Idx > 0 && Idx <= sizeof...(InnerArgs));
  using type = typename get_index_type_Impl<Idx - 1, InnerArgs...>::type;
};

template <class Header, class... Resume>
struct get_index_type_Impl<0, Header, Resume...> {
  using type = Header;
};
}  // namespace detail

template <class... Args>
struct get_index_type {
  template <size_t Idx>
  using type = typename detail::get_index_type_Impl<Idx, Args...>::type;
};

}  // namespace CLSN

#endif  // DEFTRPC_TYPE_H
