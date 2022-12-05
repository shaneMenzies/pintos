// Move, forward and identity for C++11 + swap -*- C++ -*-

// Copyright (C) 2007-2022 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file bits/move.h
 *  This is an internal header file, included by other library headers.
 *  Do not attempt to use it directly. @headername{utility}
 */

#ifndef _MOVE_H
#define _MOVE_H 1

#if __cplusplus < 201103L
    #include <bits/concept_check.h>
#endif

namespace std_k {

// Used, in C++03 mode too, by allocators, etc.
/**
 *  @brief Same as C++11 std_k::addressof
 *  @ingroup utilities
 */
template<typename _Tp> inline constexpr _Tp* __addressof(_Tp& __r) noexcept {
    return __builtin_addressof(__r);
}

#if __cplusplus >= 201103L

} // namespace

    #include "../type_traits" // Brings in std_k::declval too.

namespace std_k {

/**
 *  @addtogroup utilities
 *  @{
 */

/**
 *  @brief  Forward an lvalue.
 *  @return The parameter cast to the specified type.
 *
 *  This function is used to implement "perfect forwarding".
 */
template<typename _Tp>
[[nodiscard]] constexpr _Tp&&
    forward(typename std_k::remove_reference<_Tp>::type& __t) noexcept {
    return static_cast<_Tp&&>(__t);
}

/**
 *  @brief  Forward an rvalue.
 *  @return The parameter cast to the specified type.
 *
 *  This function is used to implement "perfect forwarding".
 */
template<typename _Tp>
[[nodiscard]] constexpr _Tp&&
    forward(typename std_k::remove_reference<_Tp>::type&& __t) noexcept {
    static_assert(
        !std_k::is_lvalue_reference<_Tp>::value,
        "std_k::forward must not be used to convert an rvalue to an lvalue");
    return static_cast<_Tp&&>(__t);
}

/**
 *  @brief  Convert a value to an rvalue.
 *  @param  __t  A thing of arbitrary type.
 *  @return The parameter cast to an rvalue-reference to allow moving it.
 */
template<typename _Tp>
[[nodiscard]] constexpr typename std_k::remove_reference<_Tp>::type&&
    move(_Tp&& __t) noexcept {
    return static_cast<typename std_k::remove_reference<_Tp>::type&&>(__t);
}

template<typename _Tp>
struct __move_if_noexcept_cond
    : public __and_<__not_<is_nothrow_move_constructible<_Tp>>,
                    is_copy_constructible<_Tp>>::type {};

/**
 *  @brief  Conditionally convert a value to an rvalue.
 *  @param  __x  A thing of arbitrary type.
 *  @return The parameter, possibly cast to an rvalue-reference.
 *
 *  Same as std_k::move unless the type's move constructor could throw and the
 *  type is copyable, in which case an lvalue-reference is returned instead.
 */
template<typename _Tp>
[[nodiscard]] constexpr __conditional_t<__move_if_noexcept_cond<_Tp>::value,
                                        const _Tp&, _Tp&&>
    move_if_noexcept(_Tp& __x) noexcept {
    return std_k::move(__x);
}

    // declval, from type_traits.

    #if __cplusplus > 201402L
        // _GLIBCXX_RESOLVE_LIB_DEFECTS
        // 2296. std_k::addressof should be constexpr
        #define __cpp_lib_addressof_constexpr 201603L
    #endif
/**
 *  @brief Returns the actual address of the object or function
 *         referenced by r, even in the presence of an overloaded
 *         operator&.
 *  @param  __r  Reference to an object or function.
 *  @return   The actual address.
 */
template<typename _Tp>
[[nodiscard]] inline constexpr _Tp* addressof(_Tp& __r) noexcept {
    return std_k::__addressof(__r);
}

// _GLIBCXX_RESOLVE_LIB_DEFECTS
// 2598. addressof works on temporaries
template<typename _Tp> const _Tp* addressof(const _Tp&&) = delete;

// C++11 version of std_k::exchange for internal use.
template<typename _Tp, typename _Up = _Tp>
constexpr inline _Tp __exchange(_Tp& __obj, _Up&& __new_val) {
    _Tp __old_val = std_k::move(__obj);
    __obj         = std_k::forward<_Up>(__new_val);
    return __old_val;
}

    /// @} group utilities

    #define _GLIBCXX_FWDREF(_Tp)         _Tp&&
    #define _GLIBCXX_MOVE(__val)         std_k::move(__val)
    #define _GLIBCXX_FORWARD(_Tp, __val) std_k::forward<_Tp>(__val)
#else
    #define _GLIBCXX_FWDREF(_Tp)         const _Tp&
    #define _GLIBCXX_MOVE(__val)         (__val)
    #define _GLIBCXX_FORWARD(_Tp, __val) (__val)
#endif

/**
 *  @addtogroup utilities
 *  @{
 */

/**
 *  @brief Swaps two values.
 *  @param  __a  A thing of arbitrary type.
 *  @param  __b  Another thing of arbitrary type.
 *  @return   Nothing.
 */
template<typename _Tp>
constexpr inline
#if __cplusplus >= 201103L
    typename enable_if<
        __and_<__not_<__is_tuple_like<_Tp>>, is_move_constructible<_Tp>,
               is_move_assignable<_Tp>>::value>::type
#else
    void
#endif
    swap(_Tp& __a,
         _Tp& __b) noexcept(__and_<is_nothrow_move_constructible<_Tp>,
                                   is_nothrow_move_assignable<_Tp>>::value) {
#if __cplusplus < 201103L
    // concept requirements
    __glibcxx_function_requires(_SGIAssignableConcept<_Tp>)
#endif
        _Tp __tmp
        = _GLIBCXX_MOVE(__a);
    __a = _GLIBCXX_MOVE(__b);
    __b = _GLIBCXX_MOVE(__tmp);
}

// _GLIBCXX_RESOLVE_LIB_DEFECTS
// DR 809. std_k::swap should be overloaded for array types.
/// Swap the contents of two arrays.
template<typename _Tp, size_t _Nm>
constexpr inline
#if __cplusplus >= 201103L
    typename enable_if<__is_swappable<_Tp>::value>::type
#else
    void
#endif
    swap(_Tp (&__a)[_Nm],
         _Tp (&__b)[_Nm]) noexcept(__is_nothrow_swappable<_Tp>::value) {
    for (size_t __n = 0; __n < _Nm; ++__n) swap(__a[__n], __b[__n]);
}

/// @} group utilities
} // namespace std_k

#endif /* _MOVE_H */
