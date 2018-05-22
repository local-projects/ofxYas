
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

#ifndef __yas__binary_oarchive_hpp
#define __yas__binary_oarchive_hpp

#include <yas/detail/type_traits/type_traits.hpp>
#include <yas/detail/type_traits/serializer.hpp>
#include <yas/detail/io/header.hpp>
#include <yas/detail/io/binary_streams.hpp>
#include <yas/detail/tools/base_object.hpp>
#include <yas/detail/tools/noncopyable.hpp>
#include <yas/detail/tools/limit.hpp>

#include <yas/types/utility/fundamental.hpp>
#include <yas/types/utility/enum.hpp>
#include <yas/types/utility/usertype.hpp>
#include <yas/types/utility/autoarray.hpp>
#include <yas/types/utility/buffer.hpp>
#include <yas/types/utility/value.hpp>
#include <yas/types/utility/object.hpp>

#include <yas/buffers.hpp>
#include <yas/object.hpp>
#include <yas/version.hpp>

namespace yas {

/***************************************************************************/

template<typename OS, std::size_t F = binary|ehost>
struct binary_oarchive
    :detail::binary_ostream<OS, F>
    ,detail::oarchive_header<F>
{
    YAS_NONCOPYABLE(binary_oarchive)
    YAS_MOVABLE(binary_oarchive)

    using stream_type = OS;
    using this_type = binary_oarchive<OS, F>;

    binary_oarchive(OS &os)
        :detail::binary_ostream<OS, F>(os)
        ,detail::oarchive_header<F>(os)
    {}

    template<typename T>
    this_type& operator& (const T &v) {
        using namespace detail;
        return serializer<
             type_properties<T>::value
            ,serialization_method<T, this_type>::value
            ,F
            ,T
        >::save(*this, v);
    }

    this_type& serialize() { return *this; }

    template<typename Head, typename... Tail>
    this_type& serialize(const Head &head, const Tail&... tail) {
        return operator& (head).serialize(tail...);
    }

    template<typename... Args>
    this_type& operator()(const Args&... args) {
        return serialize(args...);
    }
};

/***************************************************************************/

} // namespace yas

#endif // __yas__binary_oarchive_hpp
