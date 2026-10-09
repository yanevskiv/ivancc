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

#ifndef _LIBC_IMPL_STDINT_H
#define _LIBC_IMPL_STDINT_H

// Exact-width integer types
typedef signed char _Libc_Impl_Stdint_int8_t;
typedef short _Libc_Impl_Stdint_int16_t;
typedef int _Libc_Impl_Stdint_int32_t;
typedef long _Libc_Impl_Stdint_int64_t;
typedef unsigned char _Libc_Impl_Stdint_uint8_t;
typedef unsigned short _Libc_Impl_Stdint_uint16_t;
typedef unsigned int _Libc_Impl_Stdint_uint32_t;
typedef unsigned long _Libc_Impl_Stdint_uint64_t;

// Minimum-width integer types
typedef signed char _Libc_Impl_Stdint_int_least8_t;
typedef short _Libc_Impl_Stdint_int_least16_t;
typedef int _Libc_Impl_Stdint_int_least32_t;
typedef long _Libc_Impl_Stdint_int_least64_t;
typedef unsigned char _Libc_Impl_Stdint_uint_least8_t;
typedef unsigned short _Libc_Impl_Stdint_uint_least16_t;
typedef unsigned int _Libc_Impl_Stdint_uint_least32_t;
typedef unsigned long _Libc_Impl_Stdint_uint_least64_t;

// Fastest minimum-width integer types
typedef signed char _Libc_Impl_Stdint_int_fast8_t;
typedef long _Libc_Impl_Stdint_int_fast16_t;
typedef long _Libc_Impl_Stdint_int_fast32_t;
typedef long _Libc_Impl_Stdint_int_fast64_t;
typedef unsigned char _Libc_Impl_Stdint_uint_fast8_t;
typedef unsigned long _Libc_Impl_Stdint_uint_fast16_t;
typedef unsigned long _Libc_Impl_Stdint_uint_fast32_t;
typedef unsigned long _Libc_Impl_Stdint_uint_fast64_t;

// Integer types capable of holding object pointers
typedef long _Libc_Impl_Stdint_intptr_t;
typedef unsigned long _Libc_Impl_Stdint_uintptr_t;

// Greatest-width integer types
typedef long _Libc_Impl_Stdint_intmax_t;
typedef unsigned long _Libc_Impl_Stdint_uintmax_t;

// Limits of exact-width integer types
#define _LIBC_IMPL_STDINT_INT8_MIN   (-128)
#define _LIBC_IMPL_STDINT_INT16_MIN  (-32767 - 1)
#define _LIBC_IMPL_STDINT_INT32_MIN  (-2147483647 - 1)
#define _LIBC_IMPL_STDINT_INT64_MIN  (-9223372036854775807L - 1)
#define _LIBC_IMPL_STDINT_INT8_MAX   127
#define _LIBC_IMPL_STDINT_INT16_MAX  32767
#define _LIBC_IMPL_STDINT_INT32_MAX  2147483647
#define _LIBC_IMPL_STDINT_INT64_MAX  9223372036854775807L
#define _LIBC_IMPL_STDINT_UINT8_MAX  255
#define _LIBC_IMPL_STDINT_UINT16_MAX 65535
#define _LIBC_IMPL_STDINT_UINT32_MAX 4294967295U
#define _LIBC_IMPL_STDINT_UINT64_MAX 18446744073709551615UL

// Limits of minimum-width integer types
#define _LIBC_IMPL_STDINT_INT_LEAST8_MIN   (-128)
#define _LIBC_IMPL_STDINT_INT_LEAST16_MIN  (-32767 - 1)
#define _LIBC_IMPL_STDINT_INT_LEAST32_MIN  (-2147483647 - 1)
#define _LIBC_IMPL_STDINT_INT_LEAST64_MIN  (-9223372036854775807L - 1)
#define _LIBC_IMPL_STDINT_INT_LEAST8_MAX   127
#define _LIBC_IMPL_STDINT_INT_LEAST16_MAX  32767
#define _LIBC_IMPL_STDINT_INT_LEAST32_MAX  2147483647
#define _LIBC_IMPL_STDINT_INT_LEAST64_MAX  9223372036854775807L
#define _LIBC_IMPL_STDINT_UINT_LEAST8_MAX  255
#define _LIBC_IMPL_STDINT_UINT_LEAST16_MAX 65535
#define _LIBC_IMPL_STDINT_UINT_LEAST32_MAX 4294967295U
#define _LIBC_IMPL_STDINT_UINT_LEAST64_MAX 18446744073709551615UL

// Limits of fastest minimum-width integer types
#define _LIBC_IMPL_STDINT_INT_FAST8_MIN   (-128)
#define _LIBC_IMPL_STDINT_INT_FAST16_MIN  (-9223372036854775807L - 1)
#define _LIBC_IMPL_STDINT_INT_FAST32_MIN  (-9223372036854775807L - 1)
#define _LIBC_IMPL_STDINT_INT_FAST64_MIN  (-9223372036854775807L - 1)
#define _LIBC_IMPL_STDINT_INT_FAST8_MAX   127
#define _LIBC_IMPL_STDINT_INT_FAST16_MAX  9223372036854775807L
#define _LIBC_IMPL_STDINT_INT_FAST32_MAX  9223372036854775807L
#define _LIBC_IMPL_STDINT_INT_FAST64_MAX  9223372036854775807L
#define _LIBC_IMPL_STDINT_UINT_FAST8_MAX  255
#define _LIBC_IMPL_STDINT_UINT_FAST16_MAX 18446744073709551615UL
#define _LIBC_IMPL_STDINT_UINT_FAST32_MAX 18446744073709551615UL
#define _LIBC_IMPL_STDINT_UINT_FAST64_MAX 18446744073709551615UL

// Limits of integer types capable of holding object pointers
#define _LIBC_IMPL_STDINT_INTPTR_MIN  (-9223372036854775807L - 1)
#define _LIBC_IMPL_STDINT_INTPTR_MAX  9223372036854775807L
#define _LIBC_IMPL_STDINT_UINTPTR_MAX 18446744073709551615UL

// Limits of greatest-width integer types
#define _LIBC_IMPL_STDINT_INTMAX_MIN  (-9223372036854775807L - 1)
#define _LIBC_IMPL_STDINT_INTMAX_MAX  9223372036854775807L
#define _LIBC_IMPL_STDINT_UINTMAX_MAX 18446744073709551615UL

// Limits of other integer types
#define _LIBC_IMPL_STDINT_PTRDIFF_MIN    (-9223372036854775807L - 1)
#define _LIBC_IMPL_STDINT_PTRDIFF_MAX    9223372036854775807L
#define _LIBC_IMPL_STDINT_SIG_ATOMIC_MIN (-2147483647 - 1)
#define _LIBC_IMPL_STDINT_SIG_ATOMIC_MAX 2147483647
#define _LIBC_IMPL_STDINT_SIZE_MAX       18446744073709551615UL
#define _LIBC_IMPL_STDINT_WCHAR_MIN      (-2147483647 - 1)
#define _LIBC_IMPL_STDINT_WCHAR_MAX      2147483647
#define _LIBC_IMPL_STDINT_WINT_MIN       0U
#define _LIBC_IMPL_STDINT_WINT_MAX       4294967295U

// Macros for minimum-width integer constants
#define _LIBC_IMPL_STDINT_INT8_C(value)   value
#define _LIBC_IMPL_STDINT_INT16_C(value)  value
#define _LIBC_IMPL_STDINT_INT32_C(value)  value
#define _LIBC_IMPL_STDINT_INT64_C(value)  value ## L
#define _LIBC_IMPL_STDINT_UINT8_C(value)  value
#define _LIBC_IMPL_STDINT_UINT16_C(value) value
#define _LIBC_IMPL_STDINT_UINT32_C(value) value ## U
#define _LIBC_IMPL_STDINT_UINT64_C(value) value ## UL

// Macros for greatest-width integer constants
#define _LIBC_IMPL_STDINT_INTMAX_C(value)  value ## L
#define _LIBC_IMPL_STDINT_UINTMAX_C(value) value ## UL

#endif // _LIBC_IMPL_STDINT_H
