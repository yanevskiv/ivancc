/*
 * C header file for the integer types.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivancc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivancc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivancc; see
 * the files LICENSE and COPYING.RUNTIME respectively.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef __STDINT_H__
#define __STDINT_H__

// The implementation.
#include <libc/impl/libc_stdint.h>

// (S7.18.1.1) Exact-width integer types
typedef __libc_impl_stdint_int8_t int8_t;
typedef __libc_impl_stdint_int16_t int16_t;
typedef __libc_impl_stdint_int32_t int32_t;
typedef __libc_impl_stdint_int64_t int64_t;
typedef __libc_impl_stdint_uint8_t uint8_t;
typedef __libc_impl_stdint_uint16_t uint16_t;
typedef __libc_impl_stdint_uint32_t uint32_t;
typedef __libc_impl_stdint_uint64_t uint64_t;

// (S7.18.1.2) Minimum-width integer types
typedef __libc_impl_stdint_int_least8_t int_least8_t;
typedef __libc_impl_stdint_int_least16_t int_least16_t;
typedef __libc_impl_stdint_int_least32_t int_least32_t;
typedef __libc_impl_stdint_int_least64_t int_least64_t;
typedef __libc_impl_stdint_uint_least8_t uint_least8_t;
typedef __libc_impl_stdint_uint_least16_t uint_least16_t;
typedef __libc_impl_stdint_uint_least32_t uint_least32_t;
typedef __libc_impl_stdint_uint_least64_t uint_least64_t;

// (S7.18.1.3) Fastest minimum-width integer types
typedef __libc_impl_stdint_int_fast8_t int_fast8_t;
typedef __libc_impl_stdint_int_fast16_t int_fast16_t;
typedef __libc_impl_stdint_int_fast32_t int_fast32_t;
typedef __libc_impl_stdint_int_fast64_t int_fast64_t;
typedef __libc_impl_stdint_uint_fast8_t uint_fast8_t;
typedef __libc_impl_stdint_uint_fast16_t uint_fast16_t;
typedef __libc_impl_stdint_uint_fast32_t uint_fast32_t;
typedef __libc_impl_stdint_uint_fast64_t uint_fast64_t;

// (S7.18.1.4) Integer types capable of holding object pointers
typedef __libc_impl_stdint_intptr_t intptr_t;
typedef __libc_impl_stdint_uintptr_t uintptr_t;

// (S7.18.1.5) Greatest-width integer types
typedef __libc_impl_stdint_intmax_t intmax_t;
typedef __libc_impl_stdint_uintmax_t uintmax_t;

// (S7.18.2.1) Limits of exact-width integer types
#define INT8_MIN __LIBC_IMPL_STDINT_INT8_MIN
#define INT16_MIN __LIBC_IMPL_STDINT_INT16_MIN
#define INT32_MIN __LIBC_IMPL_STDINT_INT32_MIN
#define INT64_MIN __LIBC_IMPL_STDINT_INT64_MIN
#define INT8_MAX __LIBC_IMPL_STDINT_INT8_MAX
#define INT16_MAX __LIBC_IMPL_STDINT_INT16_MAX
#define INT32_MAX __LIBC_IMPL_STDINT_INT32_MAX
#define INT64_MAX __LIBC_IMPL_STDINT_INT64_MAX
#define UINT8_MAX __LIBC_IMPL_STDINT_UINT8_MAX
#define UINT16_MAX __LIBC_IMPL_STDINT_UINT16_MAX
#define UINT32_MAX __LIBC_IMPL_STDINT_UINT32_MAX
#define UINT64_MAX __LIBC_IMPL_STDINT_UINT64_MAX

// (S7.18.2.2) Limits of minimum-width integer types
#define INT_LEAST8_MIN __LIBC_IMPL_STDINT_INT_LEAST8_MIN
#define INT_LEAST16_MIN __LIBC_IMPL_STDINT_INT_LEAST16_MIN
#define INT_LEAST32_MIN __LIBC_IMPL_STDINT_INT_LEAST32_MIN
#define INT_LEAST64_MIN __LIBC_IMPL_STDINT_INT_LEAST64_MIN
#define INT_LEAST8_MAX __LIBC_IMPL_STDINT_INT_LEAST8_MAX
#define INT_LEAST16_MAX __LIBC_IMPL_STDINT_INT_LEAST16_MAX
#define INT_LEAST32_MAX __LIBC_IMPL_STDINT_INT_LEAST32_MAX
#define INT_LEAST64_MAX __LIBC_IMPL_STDINT_INT_LEAST64_MAX
#define UINT_LEAST8_MAX __LIBC_IMPL_STDINT_UINT_LEAST8_MAX
#define UINT_LEAST16_MAX __LIBC_IMPL_STDINT_UINT_LEAST16_MAX
#define UINT_LEAST32_MAX __LIBC_IMPL_STDINT_UINT_LEAST32_MAX
#define UINT_LEAST64_MAX __LIBC_IMPL_STDINT_UINT_LEAST64_MAX

// (S7.18.2.3) Limits of fastest minimum-width integer types
#define INT_FAST8_MIN __LIBC_IMPL_STDINT_INT_FAST8_MIN
#define INT_FAST16_MIN __LIBC_IMPL_STDINT_INT_FAST16_MIN
#define INT_FAST32_MIN __LIBC_IMPL_STDINT_INT_FAST32_MIN
#define INT_FAST64_MIN __LIBC_IMPL_STDINT_INT_FAST64_MIN
#define INT_FAST8_MAX __LIBC_IMPL_STDINT_INT_FAST8_MAX
#define INT_FAST16_MAX __LIBC_IMPL_STDINT_INT_FAST16_MAX
#define INT_FAST32_MAX __LIBC_IMPL_STDINT_INT_FAST32_MAX
#define INT_FAST64_MAX __LIBC_IMPL_STDINT_INT_FAST64_MAX
#define UINT_FAST8_MAX __LIBC_IMPL_STDINT_UINT_FAST8_MAX
#define UINT_FAST16_MAX __LIBC_IMPL_STDINT_UINT_FAST16_MAX
#define UINT_FAST32_MAX __LIBC_IMPL_STDINT_UINT_FAST32_MAX
#define UINT_FAST64_MAX __LIBC_IMPL_STDINT_UINT_FAST64_MAX

// (S7.18.2.4) Limits of integer types capable of holding object pointers
#define INTPTR_MIN __LIBC_IMPL_STDINT_INTPTR_MIN
#define INTPTR_MAX __LIBC_IMPL_STDINT_INTPTR_MAX
#define UINTPTR_MAX __LIBC_IMPL_STDINT_UINTPTR_MAX

// (S7.18.2.5) Limits of greatest-width integer types
#define INTMAX_MIN __LIBC_IMPL_STDINT_INTMAX_MIN
#define INTMAX_MAX __LIBC_IMPL_STDINT_INTMAX_MAX
#define UINTMAX_MAX __LIBC_IMPL_STDINT_UINTMAX_MAX

// (S7.18.3) Limits of other integer types
#define PTRDIFF_MIN __LIBC_IMPL_STDINT_PTRDIFF_MIN
#define PTRDIFF_MAX __LIBC_IMPL_STDINT_PTRDIFF_MAX
#define SIG_ATOMIC_MIN __LIBC_IMPL_STDINT_SIG_ATOMIC_MIN
#define SIG_ATOMIC_MAX __LIBC_IMPL_STDINT_SIG_ATOMIC_MAX
#define SIZE_MAX __LIBC_IMPL_STDINT_SIZE_MAX
#define WCHAR_MIN __LIBC_IMPL_STDINT_WCHAR_MIN
#define WCHAR_MAX __LIBC_IMPL_STDINT_WCHAR_MAX
#define WINT_MIN __LIBC_IMPL_STDINT_WINT_MIN
#define WINT_MAX __LIBC_IMPL_STDINT_WINT_MAX

// (S7.18.4.1) Macros for minimum-width integer constants
#define INT8_C(value) __LIBC_IMPL_STDINT_INT8_C(value)
#define INT16_C(value) __LIBC_IMPL_STDINT_INT16_C(value)
#define INT32_C(value) __LIBC_IMPL_STDINT_INT32_C(value)
#define INT64_C(value) __LIBC_IMPL_STDINT_INT64_C(value)
#define UINT8_C(value) __LIBC_IMPL_STDINT_UINT8_C(value)
#define UINT16_C(value) __LIBC_IMPL_STDINT_UINT16_C(value)
#define UINT32_C(value) __LIBC_IMPL_STDINT_UINT32_C(value)
#define UINT64_C(value) __LIBC_IMPL_STDINT_UINT64_C(value)

// (S7.18.4.2) Macros for greatest-width integer constants
#define INTMAX_C(value) __LIBC_IMPL_STDINT_INTMAX_C(value)
#define UINTMAX_C(value) __LIBC_IMPL_STDINT_UINTMAX_C(value)

#endif // __STDINT_H__
