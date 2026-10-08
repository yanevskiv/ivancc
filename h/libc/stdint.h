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

// (S7.18.1.1) Exact-width integer types
typedef signed char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int64_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

// (S7.18.1.2) Minimum-width integer types
typedef signed char int_least8_t;
typedef short int_least16_t;
typedef int int_least32_t;
typedef long int_least64_t;
typedef unsigned char uint_least8_t;
typedef unsigned short uint_least16_t;
typedef unsigned int uint_least32_t;
typedef unsigned long uint_least64_t;

// (S7.18.1.3) Fastest minimum-width integer types
typedef signed char int_fast8_t;
typedef long int_fast16_t;
typedef long int_fast32_t;
typedef long int_fast64_t;
typedef unsigned char uint_fast8_t;
typedef unsigned long uint_fast16_t;
typedef unsigned long uint_fast32_t;
typedef unsigned long uint_fast64_t;

// (S7.18.1.4) Integer types capable of holding object pointers
typedef long intptr_t;
typedef unsigned long uintptr_t;

// (S7.18.1.5) Greatest-width integer types
typedef long intmax_t;
typedef unsigned long uintmax_t;

// (S7.18.2.1) Limits of exact-width integer types
#define INT8_MIN (-128)
#define INT16_MIN (-32767 - 1)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-9223372036854775807L - 1)
#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807L
#define UINT8_MAX 255
#define UINT16_MAX 65535
#define UINT32_MAX 4294967295U
#define UINT64_MAX 18446744073709551615UL

// (S7.18.2.2) Limits of minimum-width integer types
#define INT_LEAST8_MIN (-128)
#define INT_LEAST16_MIN (-32767 - 1)
#define INT_LEAST32_MIN (-2147483647 - 1)
#define INT_LEAST64_MIN (-9223372036854775807L - 1)
#define INT_LEAST8_MAX 127
#define INT_LEAST16_MAX 32767
#define INT_LEAST32_MAX 2147483647
#define INT_LEAST64_MAX 9223372036854775807L
#define UINT_LEAST8_MAX 255
#define UINT_LEAST16_MAX 65535
#define UINT_LEAST32_MAX 4294967295U
#define UINT_LEAST64_MAX 18446744073709551615UL

// (S7.18.2.3) Limits of fastest minimum-width integer types
#define INT_FAST8_MIN (-128)
#define INT_FAST16_MIN (-9223372036854775807L - 1)
#define INT_FAST32_MIN (-9223372036854775807L - 1)
#define INT_FAST64_MIN (-9223372036854775807L - 1)
#define INT_FAST8_MAX 127
#define INT_FAST16_MAX 9223372036854775807L
#define INT_FAST32_MAX 9223372036854775807L
#define INT_FAST64_MAX 9223372036854775807L
#define UINT_FAST8_MAX 255
#define UINT_FAST16_MAX 18446744073709551615UL
#define UINT_FAST32_MAX 18446744073709551615UL
#define UINT_FAST64_MAX 18446744073709551615UL

// (S7.18.2.4) Limits of integer types capable of holding object pointers
#define INTPTR_MIN (-9223372036854775807L - 1)
#define INTPTR_MAX 9223372036854775807L
#define UINTPTR_MAX 18446744073709551615UL

// (S7.18.2.5) Limits of greatest-width integer types
#define INTMAX_MIN (-9223372036854775807L - 1)
#define INTMAX_MAX 9223372036854775807L
#define UINTMAX_MAX 18446744073709551615UL

// (S7.18.3) Limits of other integer types
#define PTRDIFF_MIN (-9223372036854775807L - 1)
#define PTRDIFF_MAX 9223372036854775807L
#define SIG_ATOMIC_MIN (-2147483647 - 1)
#define SIG_ATOMIC_MAX 2147483647
#define SIZE_MAX 18446744073709551615UL
#define WCHAR_MIN (-2147483647 - 1)
#define WCHAR_MAX 2147483647
#define WINT_MIN 0U
#define WINT_MAX 4294967295U

// (S7.18.4.1) Macros for minimum-width integer constants
#define INT8_C(value) value
#define INT16_C(value) value
#define INT32_C(value) value
#define INT64_C(value) value ## L
#define UINT8_C(value) value
#define UINT16_C(value) value
#define UINT32_C(value) value ## U
#define UINT64_C(value) value ## UL

// (S7.18.4.2) Macros for greatest-width integer constants
#define INTMAX_C(value) value ## L
#define UINTMAX_C(value) value ## UL

#endif // __STDINT_H__
