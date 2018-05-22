
// Copyright (c) 2010-2018 niXman (i dot nixman dog gmail dot com). All
// rights reserved.
//
// This file is part of YAS(https://github.com/niXman/yas) project.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef __yas__object_hpp
#define __yas__object_hpp

#include <yas/detail/preprocessor/preprocessor.hpp>
#include <yas/detail/type_traits/type_traits.hpp>
#include <yas/detail/tools/fnv1a.hpp>

#include <tuple>
#include <cstring>

namespace yas {
namespace detail {

/***************************************************************************/

template<typename, typename>
struct concat;

template<typename... Xs, typename... Ys>
struct concat<std::tuple<Xs...>, std::tuple<Ys...>> {
    using type = std::tuple<Xs..., Ys...>;
};

template<typename X, typename TPL>
struct split_accum_fst {
    using type = std::tuple<
        typename concat<
             std::tuple<X>
            ,typename std::tuple_element<0,TPL>::type
        >::type
        ,typename std::tuple_element<1,TPL>::type
    >;
};

template<typename, typename>
struct split_gather {};

template<typename X, typename... Xs, typename ignore1, typename ignore2, typename... Ys>
struct split_gather<std::tuple<X, Xs...>, std::tuple<ignore1, ignore2, Ys...>>  {
    using type = typename split_accum_fst<
         X
        ,typename split_gather<
             std::tuple<Xs...>
            ,std::tuple<Ys...>
        >::type
    >::type;
};

template<typename ... Xs, typename ignore>
struct split_gather<std::tuple<Xs...>, ignore>  {
    using type = std::tuple<std::tuple<>, std::tuple<Xs...>>;
};

template<typename SEQ>
struct split {
    using type = typename split_gather<SEQ, SEQ>::type;
};

template<template<typename, typename> class PRED, typename SEQ1, typename SEQ2>
struct merge {};

template<template<typename, typename> class PRED, typename SEQ>
struct merge<PRED, SEQ, std::tuple<>> {
    using type = SEQ;
};

template<template<typename, typename> class PRED, typename SEQ>
struct merge<PRED, std::tuple<>, SEQ> {
    using type = SEQ;
};

template<template<typename, typename> class PRED, typename X, typename... Xs, typename Y, typename... Ys>
struct merge<PRED, std::tuple<X, Xs...>, std::tuple<Y, Ys...>>  {
    template<bool cond, typename T1, typename T2>
    struct merge_case { //case true
        using type = T1;
    };

    template<typename T1, typename T2>
    struct merge_case<0, T1, T2> { //case false
        using type = T2;
    };

    using type = typename merge_case<
         PRED<X, Y>::value
        ,typename concat<
             std::tuple<X>
            ,typename merge<
                 PRED
                ,std::tuple<Xs...>
                ,std::tuple<Y, Ys...>
            >::type
         >::type
        ,typename concat<
             std::tuple<Y>
            ,typename merge<
                 PRED
                ,std::tuple<X, Xs...>
                ,std::tuple<Ys...>
            >::type
        >::type
    >::type;
};

template<template<typename, typename> class PRED, typename SEQ>
struct mergesort {
    using type = typename merge<PRED,
        typename mergesort<
             PRED
            ,typename std::tuple_element<
                 0
                ,typename split<SEQ>::type
            >::type
         >::type
        ,typename mergesort<
             PRED
            ,typename std::tuple_element<
                 1
                ,typename split<SEQ>::type
            >::type
        >::type
    >::type;
};

template <template<typename, typename> class PRED, typename X>
struct mergesort<PRED, std::tuple<X>> {
    using type = std::tuple<X>;
};

template <template<typename, typename> class PRED>
struct mergesort<PRED, std::tuple<>> {
    using type = std::tuple<>;
};

template<typename lhs, typename rhs>
struct predic_less {
    enum { value = lhs::first_type::value < rhs::first_type::value };
};

/***********************************************************************************/

template<typename K, typename V>
struct pair {
    K key;
    V val;
};

using optional_t = pair<bool, std::uint8_t>;

template<typename>
struct ctmap;

template<typename... KVI>
struct ctmap<std::tuple<KVI...>> {
    optional_t find(std::uint32_t k) const {
        auto beg = &kvis[0];
        auto end = &kvis[sizeof...(KVI)];
        std::size_t count = sizeof...(KVI);

        while ( count > 0 ) {
            if ( (beg+count/2)->key < k ) {
                beg = beg+count/2+1;
                count -= count/2+1;
            } else {
                count = count/2;
            }
        }

        return {(beg != end && beg->key == k), beg->val};
    }

    static constexpr pair<std::uint32_t, std::uint8_t> kvis[] = {
        {KVI::first_type::value, KVI::second_type::value}...
    };
};
template<typename... KVI>
constexpr pair<std::uint32_t, std::uint8_t> ctmap<std::tuple<KVI...>>::kvis[];

template<typename KVI>
struct ctmap<std::tuple<KVI>> {
    optional_t find(std::uint32_t k) const {
        return {KVI::first_type::value == k, KVI::second_type::value};
    }
};

template<>
struct ctmap<std::tuple<>> {
    optional_t find(std::uint32_t /*k*/) const {
        return {false, 0};
    }
};

/***********************************************************************************/

} // ns detail

/***************************************************************************/

template<typename K, typename V>
struct value {
    template<typename KVT>
    struct real_kvt {
        using type = typename std::conditional<
             std::is_array<typename std::remove_reference<KVT>::type>::value
            ,typename std::remove_cv<KVT>::type
            ,typename std::conditional<
                 std::is_lvalue_reference<KVT>::value
                ,KVT
                ,typename std::decay<KVT>::type
            >::type
        >::type;
    };
    using key_type   = typename real_kvt<K>::type;
    using value_type = typename real_kvt<V>::type;

    value(const value &) = delete;
    value& operator=(const value &) = delete;

    constexpr value(const char *k, std::size_t klen, V &&v) noexcept
        :key(k)
        ,klen(klen)
        ,val(std::forward<V>(v))
    {}
    constexpr value(K &&k, V &&v) noexcept
        :key(std::forward<K>(k))
        ,klen(0)
        ,val(std::forward<V>(v))
    {}
    constexpr value(value &&r) noexcept
        :key(r.key)
        ,klen(r.klen)
        ,val(std::forward<value_type>(r.val))
    {}

    key_type key;
    const std::size_t klen;
    value_type val;
};

template<typename K, typename V>
constexpr typename std::enable_if<
    !std::is_same<
         typename std::remove_reference<K>::type
        ,const char*
    >::value
    ,value<K, V>
>::type
make_val(K &&key, V &&val) {
    return {std::forward<K>(key), std::forward<V>(val)};
}

template<std::size_t N, typename V>
constexpr value<const char*, V>
make_val(const char (&key)[N], V &&val) {
    return {key, N-1, std::forward<V>(val)};
}

template<typename ConstCharPtr, typename V>
constexpr typename std::enable_if<
     std::is_same<ConstCharPtr, const char*>::value
    ,value<const char*, V>
>::type
make_val(ConstCharPtr key, V &&val) {
    return {key, std::strlen(key), std::forward<V>(val)};
}

/***************************************************************************/

template<typename KVI, typename... Pairs>
struct object {
    using tuple = std::tuple<Pairs...>;

    object(const object &) = delete;
    object& operator=(const object &) = delete;

    constexpr object(const char *k, std::size_t klen, Pairs&&... pairs) noexcept
        :key(k)
        ,klen(klen)
        ,pairs(std::forward<Pairs>(pairs)...)
    {}
    constexpr object(object &&r) noexcept
        :key(r.key)
        ,klen(r.klen)
        ,pairs(std::move(r.pairs))
    {}

    const char *key;
    const std::size_t klen;
    tuple pairs;

    static constexpr detail::ctmap<KVI> map{};
};
template<typename KVI, typename... Pairs>
constexpr detail::ctmap<KVI> object<KVI, Pairs...>::map;

template<typename KVI, std::size_t N, typename... Pairs>
constexpr object<KVI, Pairs...>
make_object(const char (&key)[N], Pairs &&... pairs) {
    return {key, N-1, std::forward<Pairs>(pairs)...};
}

template<typename KVI, typename... Pairs>
constexpr object<KVI, Pairs...>
make_object(std::nullptr_t, Pairs &&... pairs) {
    return {nullptr, 0, std::forward<Pairs>(pairs)...};
}

/**************************************************************************/

#define __YAS_ARG16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define __YAS_HAS_COMMA(...) __YAS_ARG16(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
#define __YAS__TRIGGER_PARENTHESIS_(...) ,
#define __YAS_PASTE5(_0, _1, _2, _3, _4) _0 ## _1 ## _2 ## _3 ## _4
#define __YAS_IS_EMPTY_CASE_0001 ,
#define __YAS_ISEMPTY(_0, _1, _2, _3) __YAS_HAS_COMMA(__YAS_PASTE5(__YAS_IS_EMPTY_CASE_, _0, _1, _2, _3))
#define __YAS_TUPLE_IS_EMPTY_IMPL(...) \
    __YAS_ISEMPTY( \
        __YAS_HAS_COMMA(__VA_ARGS__), \
        __YAS_HAS_COMMA(__YAS__TRIGGER_PARENTHESIS_ __VA_ARGS__),                 \
        __YAS_HAS_COMMA(__VA_ARGS__ (/*empty*/)), \
        __YAS_HAS_COMMA(__YAS__TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/)) \
    )

#if defined(YAS_SERIALIZE_BOOST_TYPES)
#   include <boost/version.hpp>
#   if BOOST_VERSION >= 106000
#       include <boost/vmd/is_empty.hpp>
#       define __YAS_TUPLE_IS_EMPTY(...) BOOST_VMD_IS_EMPTY(__VA_ARGS__)
#   else
#       define __YAS_TUPLE_IS_EMPTY(...) __YAS_TUPLE_IS_EMPTY_IMPL(__VA_ARGS__)
#   endif // BOOST_VERSION >= 106000
#else // !defined(YAS_SERIALIZE_BOOST_TYPES)
#   define __YAS_TUPLE_IS_EMPTY(...) __YAS_TUPLE_IS_EMPTY_IMPL(__VA_ARGS__)
#endif // defined(YAS_SERIALIZE_BOOST_TYPES)

/**************************************************************************/

#define __YAS_OBJECT_GEN_PAIRS(unused0, unised1, idx, elem) \
    YAS_PP_COMMA_IF(idx) \
        ::yas::make_val(YAS_PP_STRINGIZE(elem), elem)

#define __YAS_OBJECT_IMPL(seq) \
	YAS_PP_SEQ_FOR_EACH_I( \
		 __YAS_OBJECT_GEN_PAIRS \
		,~ \
		,seq \
	)

#define __YAS_OBJECT_NONEMPTY_GEN_KVI_CB(unised0, unised1, idx, elem) \
    YAS_PP_COMMA_IF(idx) \
        std::pair< \
             std::integral_constant< \
                  std::uint32_t \
                 ,::yas::detail::fnv1a(YAS_PP_STRINGIZE(elem)) \
             > \
            ,std::integral_constant<std::uint8_t, idx> \
        >

#define __YAS_OBJECT_NONEMPTY_GEN_KVI(seq) \
    typename ::yas::detail::mergesort< \
        ::yas::detail::predic_less \
        ,std::tuple< \
            YAS_PP_SEQ_FOR_EACH_I( \
                 __YAS_OBJECT_NONEMPTY_GEN_KVI_CB \
                ,~ \
                ,seq \
            ) \
        > \
    >::type

#define __YAS_OBJECT_EMPTY(name, ...) \
	::yas::make_object<std::tuple<>>(name)

#define __YAS_OBJECT_NONEMPTY(name, seq) \
	::yas::make_object< \
        __YAS_OBJECT_NONEMPTY_GEN_KVI(seq) \
    >( \
		 name \
		,__YAS_OBJECT_IMPL(seq) \
	)

#define YAS_OBJECT(name, ...) \
	YAS_PP_IF( \
		 __YAS_TUPLE_IS_EMPTY(__VA_ARGS__) \
		,__YAS_OBJECT_EMPTY \
		,__YAS_OBJECT_NONEMPTY \
	)(name, YAS_PP_TUPLE_TO_SEQ((__VA_ARGS__)))

/***************************************************************************/

#define __YAS_OBJECT_NVP_GEN_PAIRS(unused0, unised1, idx, elem) \
    YAS_PP_COMMA_IF(idx) \
        ::yas::make_val( \
             YAS_PP_TUPLE_ELEM(0, elem) \
            ,YAS_PP_TUPLE_ELEM(1, elem) \
        )

#define __YAS_OBJECT_NVP_IMPL(seq) \
	YAS_PP_SEQ_FOR_EACH_I( \
		 __YAS_OBJECT_NVP_GEN_PAIRS \
        ,~ \
		,seq \
	)

#define __YAS_OBJECT_NVP_NONEMPTY_GEN_KVI_CB(unised0, unised1, idx, elem) \
    YAS_PP_COMMA_IF(idx) \
        std::pair< \
             std::integral_constant< \
                  std::uint32_t \
                 ,::yas::detail::fnv1a(YAS_PP_TUPLE_ELEM(0, elem)) \
             > \
            ,std::integral_constant<std::uint8_t, idx> \
        >

#define __YAS_OBJECT_NVP_NONEMPTY_GEN_KVI(seq) \
    typename ::yas::detail::mergesort< \
        ::yas::detail::predic_less \
        ,std::tuple< \
            YAS_PP_SEQ_FOR_EACH_I( \
                 __YAS_OBJECT_NVP_NONEMPTY_GEN_KVI_CB \
                ,~ \
                ,seq \
            ) \
        > \
    >::type

#define __YAS_OBJECT_NVP_EMPTY(name, ...) \
	::yas::make_object<std::tuple<>>(name)

#define __YAS_OBJECT_NVP_NONEMPTY(name, seq) \
	::yas::make_object< \
        __YAS_OBJECT_NVP_NONEMPTY_GEN_KVI(seq) \
    >( \
		 name \
		,__YAS_OBJECT_NVP_IMPL(seq) \
	)

#define YAS_OBJECT_NVP(name, ...) \
	YAS_PP_IF( \
		 __YAS_TUPLE_IS_EMPTY(__VA_ARGS__) \
		,__YAS_OBJECT_NVP_EMPTY \
		,__YAS_OBJECT_NVP_NONEMPTY \
	)(name, YAS_PP_TUPLE_TO_SEQ((__VA_ARGS__)))

/***************************************************************************/

} // ns yas

#endif // __yas__object_hpp
