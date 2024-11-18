//
// Created by lqf on 23-4-24.
//

#ifndef DEFTRPC_HELPER_H
#define DEFTRPC_HELPER_H

#include <unistd.h>
#include <type_traits>
#include <utility>
#include "serialize/detail/access.h"
#include "serialize/detail/config.h"

namespace clsn {

template <class Header, class... Args>
struct ConditionalAnd : public std::conditional_t<Header::value, ConditionalAnd<Args...>, std::false_type> {};

template <class Header>
struct ConditionalAnd<Header> : public std::conditional_t<Header::value, std::true_type, std::false_type> {};

template <typename... Args>
inline constexpr bool CONDITIONAL_AND_V = ConditionalAnd<Args...>::value;

template <class Header, class... Args>
struct ConditionalOr : public std::conditional_t<Header::value, std::true_type, ConditionalAnd<Args...>> {};

template <class Header>
struct ConditionalOr<Header> : public std::conditional_t<Header::value, std::true_type, std::false_type> {};

template <typename... Args>
inline constexpr bool CONDITIONAL_OR_V = ConditionalOr<Args...>::value;

class BinarySerialize;

class BinaryDeSerialize;

template <class T>
struct IsBinarySerialize : ConditionalOr<std::is_base_of<T, BinarySerialize>, std::is_same<T, BinarySerialize>> {};

template <class T>
struct IsBinaryDeserialize : ConditionalOr<std::is_base_of<T, BinaryDeSerialize>, std::is_same<T, BinaryDeSerialize>> {
};

class StringSerialize;

class StringDeSerialize;

template <class T>
struct IsStringSerialize : ConditionalOr<std::is_base_of<T, StringSerialize>, std::is_same<T, StringSerialize>> {};

template <class T>
struct IsStringDeserialize : ConditionalOr<std::is_base_of<T, StringDeSerialize>, std::is_same<T, StringDeSerialize>> {
};

template <class T>
class Serializer;

template <class T>
struct IsSerializer : std::is_base_of<Serializer<T>, T> {};

template <class T>
class DeSerializer;

template <class T>
struct IsDeSerializer : std::is_base_of<DeSerializer<T>, T> {};

template <typename T>
class Construct;

namespace detail {
#define DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME_HELPER SerializeFuncHelper
#define DEFTRPC_DESERIALIZE_INPUT_FUNCNAME_HELPER DeSerializeFuncHelper

#define NOMEMBERFUNCHELPER(Type, FuncName, Parameter)                                                       \
  template <class sr, typename T,                                                                           \
            typename v = std::enable_if<                                                                    \
                std::is_same_v<void, decltype(FuncName(std::declval<sr &>(), std::declval<Parameter>()))>>> \
  std::true_type has_no_member_##Type##_func_helper(int);                                                   \
                                                                                                            \
  template <class sr, typename T>                                                                           \
  std::false_type has_no_member_##Type##_func_helper(...);

NOMEMBERFUNCHELPER(serialize, DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME, T &&)

NOMEMBERFUNCHELPER(deserialize, DEFTRPC_DESERIALIZE_INPUT_FUNCNAME, T &&)

NOMEMBERFUNCHELPER(load_and_construct, DEFTRPC_DESERIALIZE_LOAD_AND_CONSTRUCT_FUNCNANE,
                   Construct<std::remove_reference_t<T>> &)

#undef NOMEMBERFUNCHELPER

#define MEMBERFUNCHELPER(Type, FuncName, Parameter)                                                                 \
  template <class sr, typename T,                                                                                   \
            typename v = std::enable_if<                                                                            \
                std::is_same_v<void, decltype(access::FuncName(std::declval<sr &>(), std::declval<Parameter>()))>>> \
  std::true_type has_member_##Type##_func_helper(int);                                                              \
                                                                                                                    \
  template <class sr, typename T>                                                                                   \
  std::false_type has_member_##Type##_func_helper(...);

MEMBERFUNCHELPER(serialize, DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME, T &&)

MEMBERFUNCHELPER(deserialize, DEFTRPC_DESERIALIZE_INPUT_FUNCNAME, T &&)

MEMBERFUNCHELPER(load_and_construct, DEFTRPC_DESERIALIZE_LOAD_AND_CONSTRUCT_FUNCNANE,
                 Construct<std::remove_reference_t<T>> &)

#undef MEMBERFUNCHELPER

}  // namespace detail

#define NOMEMBERFUNC(FuncType)                                                                      \
  template <class Sr, typename T>                                                                   \
  struct has_no_member_##FuncType##_func                                                            \
      : public decltype(detail::has_no_member_##FuncType##_func_helper<std::decay_t<Sr>, T>(0)) {}; \
  template <class Sr, typename T>                                                                   \
  inline constexpr bool has_no_member_##FuncType##_func_v = has_no_member_##FuncType##_func<Sr, T>::value;

NOMEMBERFUNC(serialize)

NOMEMBERFUNC(deserialize)

NOMEMBERFUNC(load_and_construct)
#undef NOMEMBERFUNC

#define MEMBERFUNC(FuncType)                                                                     \
  template <class Sr, typename T>                                                                \
  struct has_member_##FuncType##_func                                                            \
      : public decltype(detail::has_member_##FuncType##_func_helper<std::decay_t<Sr>, T>(0)) {}; \
  template <class Sr, typename T>                                                                \
  inline constexpr bool has_member_##FuncType##_func_v = has_member_##FuncType##_func<Sr, T>::value;

MEMBERFUNC(serialize)

MEMBERFUNC(deserialize)

MEMBERFUNC(load_and_construct)
#undef MEMBERFUNC

#define HASFUNC(FuncType)                                                                                     \
  template <typename Sr, typename T>                                                                          \
  struct has_##FuncType                                                                                       \
      : public ConditionalOr<has_member_##FuncType##_func<Sr, T>, has_no_member_##FuncType##_func<Sr, T>> {}; \
  template <class Sr, typename T>                                                                             \
  inline constexpr bool has_##FuncType##_v = has_##FuncType<Sr, T>::value;

HASFUNC(serialize)

HASFUNC(deserialize)

HASFUNC(load_and_construct)

#undef HASFUNC

template <typename T>
class SizeTag {
  using type = std::conditional_t<std::is_lvalue_reference_v<T>, T, std::decay_t<T>>;

 public:
  explicit SizeTag(T &&s) noexcept : m_size_(std::forward<T>(s)) {}

  ~SizeTag() = default;

  SizeTag(const SizeTag &) = delete;

  SizeTag &operator()(const SizeTag &) = delete;

  type m_size_;
};

template <typename T>
SizeTag<T> MakeSizeTag(T &&size) noexcept {
  return SizeTag<T>(std::forward<T>(size));
}

/**
 * sequence idx
 */
template <size_t... Idx>
struct SequenceIndex {
  using next = SequenceIndex<Idx..., sizeof...(Idx)>;
};

template <size_t size>
struct SequenceIndexMaker {
  static_assert(0 < size);
  using type = typename SequenceIndexMaker<size - 1>::type::next;
};

template <>
struct SequenceIndexMaker<0> {
  using type = SequenceIndex<>;
};

template <size_t size>
auto MakeSequenceIndex() noexcept {
  return typename SequenceIndexMaker<size>::type{};
}

}  // namespace clsn

#endif  // DEFTRPC_HELPER_H
