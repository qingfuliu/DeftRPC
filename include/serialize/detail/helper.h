//
// Created by lqf on 23-4-24.
//

#ifndef DEFTRPC_HELPER_H
#define DEFTRPC_HELPER_H

#include "config.h"
#include "access.h"
#include <type_traits>
#include <unistd.h>


namespace CLSN {





    template<class Header, class ...Args>
    struct ConditionalAnd
            : public std::conditional_t<Header::value, ConditionalAnd<Args...>, std::false_type> {
    };

    template<class Header>
    struct ConditionalAnd<Header> :
            public std::conditional_t<Header::value, std::true_type, std::false_type> {
    };

    template<typename ...Args>
    inline constexpr bool ConditionalAnd_v = ConditionalAnd<Args...>::value;

    template<class Header, class ...Args>
    struct ConditionalOr
            : public std::conditional_t<Header::value, std::true_type, ConditionalAnd<Args...>> {
    };

    template<class Header>
    struct ConditionalOr<Header> :
            public std::conditional_t<Header::value, std::true_type, std::false_type> {
    };

    template<typename ...Args>
    inline constexpr bool ConditionalOr_v = ConditionalOr<Args...>::value;


    class BinarySerialize;

    class BinaryDeSerialize;

    template<class T>
    struct is_binary_serialize : ConditionalOr<std::is_base_of<T, BinarySerialize>,
            std::is_same<T, BinarySerialize>> {

    };


    template<class T>
    struct is_binary_deserialize : ConditionalOr<std::is_base_of<T, BinaryDeSerialize>,
            std::is_same<T, BinaryDeSerialize>> {

    };

    template<class T>
    class Serializer;

    template<class T>
    struct is_Serializer : std::is_base_of<Serializer<T>, T> {

    };

    template<class T>
    class DeSerializer;

    template<class T>
    struct is_DeSerializer : std::is_base_of<DeSerializer<T>, T> {

    };

    template<typename T>
    class Construct;

    namespace detail {
#define DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME_HELPER SerializeFuncHelper
#define DEFTRPC_DESERIALIZE_INPUT_FUNCNAME_HELPER DeSerializeFuncHelper

#define NOMEMBERFUNCHELPER(Type, FuncName, Parameter)            \
        template<class sr, typename T,\
        typename v= std::enable_if<\
                std::is_same_v<\
                        void,\
                        decltype(FuncName(std::declval<sr &>(), std::declval<Parameter>()))>\
        >\
                    >\
                    std::true_type has_no_member_##Type##_func_helper(int);\
\
        template<class sr, typename T>\
        std::false_type has_no_member_##Type##_func_helper(...);\


        NOMEMBERFUNCHELPER(serialize, DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME, T &&)

        NOMEMBERFUNCHELPER(deserialize, DEFTRPC_DESERIALIZE_INPUT_FUNCNAME, T &&)


        NOMEMBERFUNCHELPER(load_and_construct, DEFTRPC_DESERIALIZE_LOAD_AND_CONSTRUCT_FUNCNANE,
                           Construct<std::remove_reference_t<T>> &)

#undef NOMEMBERFUNCHELPER


#define MEMBERFUNCHELPER(Type, FuncName, Parameter)            \
        template<class sr, typename T,\
        typename v= std::enable_if<\
                std::is_same_v<\
                        void,\
                        decltype(access::FuncName(std::declval<sr &>(), std::declval<Parameter>()))>\
        >\
                    >\
                    std::true_type has_member_##Type##_func_helper(int);\
\
        template<class sr, typename T>\
        std::false_type has_member_##Type##_func_helper(...);\



        MEMBERFUNCHELPER(serialize, DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME, T &&)

        MEMBERFUNCHELPER(deserialize, DEFTRPC_DESERIALIZE_INPUT_FUNCNAME, T &&)

        MEMBERFUNCHELPER(load_and_construct, DEFTRPC_DESERIALIZE_LOAD_AND_CONSTRUCT_FUNCNANE,
                         Construct<std::remove_reference_t<T>> &)

#undef MEMBERFUNCHELPER

    }


#define NOMEMBERFUNC(FuncType)          \
    template<class Sr, typename T>  \
    struct has_no_member_##FuncType##_func \
            : public decltype(detail::has_no_member_##FuncType##_func_helper<std::decay_t<Sr>, T>(0)) {    \
    };      \
    template<class Sr, typename T>      \
    inline constexpr bool has_no_member_##FuncType##_func_v = has_no_member_##FuncType##_func<Sr, T>::value;

    NOMEMBERFUNC(serialize)

    NOMEMBERFUNC(deserialize)

    NOMEMBERFUNC(load_and_construct)
#undef  NOMEMBERFUNC

#define MEMBERFUNC(FuncType)          \
    template<class Sr, typename T>  \
    struct has_member_##FuncType##_func \
            : public decltype(detail::has_member_##FuncType##_func_helper<std::decay_t<Sr>, T>(0)) {    \
    };      \
    template<class Sr, typename T>      \
    inline constexpr bool has_member_##FuncType##_func_v = has_member_##FuncType##_func<Sr, T>::value;

    MEMBERFUNC(serialize)

    MEMBERFUNC(deserialize)

    MEMBERFUNC(load_and_construct)
#undef  MEMBERFUNC

#define HASFUNC(FuncType)               \
    template<typename Sr, typename T>\
    struct has_##FuncType : public ConditionalOr<has_member_##FuncType##_func<Sr, T>, has_no_member_##FuncType##_func<Sr, T>> {\
    };\
    template<class Sr, typename T>\
    inline constexpr bool has_##FuncType##_v = has_##FuncType<Sr, T>::value;

    HASFUNC(serialize)

    HASFUNC(deserialize)

    HASFUNC(load_and_construct)

#undef HASFUNC


    template<typename T>
    class size_tag {
        using type =
                std::conditional_t<std::is_lvalue_reference_v<T>, T, std::decay_t<T>>;

    public:
        size_tag(T &&s) noexcept: size(std::forward<T>(s)) {}

        ~size_tag() = default;

        size_tag(const size_tag &) = delete;

        size_tag &operator()(const size_tag &) = delete;

        type size;
    };

    template<typename T>
    size_tag<T> make_size_tag(T &&size) noexcept {
        return {std::forward<T>(size)};
    }

/**
 * sequence idx
 */
    template<size_t...Idx>
    struct sequence_index {
        using next = sequence_index<Idx..., sizeof...(Idx)>;
    };

    template<size_t size>
    struct sequence_index_maker {
        static_assert(0 < size);
        using type = typename sequence_index_maker<size - 1>::type::next;
    };

    template<>
    struct sequence_index_maker<0> {
        using type = sequence_index<>;
    };

    template<size_t size>
    auto make_sequence_index() noexcept {
        return typename sequence_index_maker<size>::type{};
    }


}

#endif //DEFTRPC_HELPER_H
