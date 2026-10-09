/*
 * C header file for the sizes of integer types.
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

#ifndef _LIBC_IMPL_LIMITS_H
#define _LIBC_IMPL_LIMITS_H

// Sizes of integer types
#define _LIBC_IMPL_LIMITS_CHAR_BIT   8
#define _LIBC_IMPL_LIMITS_SCHAR_MIN  (-128)
#define _LIBC_IMPL_LIMITS_SCHAR_MAX  127
#define _LIBC_IMPL_LIMITS_UCHAR_MAX  255
#define _LIBC_IMPL_LIMITS_CHAR_MIN   _LIBC_IMPL_LIMITS_SCHAR_MIN
#define _LIBC_IMPL_LIMITS_CHAR_MAX   _LIBC_IMPL_LIMITS_SCHAR_MAX
#define _LIBC_IMPL_LIMITS_MB_LEN_MAX 16
#define _LIBC_IMPL_LIMITS_SHRT_MIN   (-32768)
#define _LIBC_IMPL_LIMITS_SHRT_MAX   32767
#define _LIBC_IMPL_LIMITS_USHRT_MAX  65535
#define _LIBC_IMPL_LIMITS_INT_MIN    (-_LIBC_IMPL_LIMITS_INT_MAX - 1)
#define _LIBC_IMPL_LIMITS_INT_MAX    2147483647
#define _LIBC_IMPL_LIMITS_UINT_MAX   4294967295U
#define _LIBC_IMPL_LIMITS_LONG_MIN   (-_LIBC_IMPL_LIMITS_LONG_MAX - 1L)
#define _LIBC_IMPL_LIMITS_LONG_MAX   9223372036854775807L
#define _LIBC_IMPL_LIMITS_ULONG_MAX  18446744073709551615UL
#define _LIBC_IMPL_LIMITS_LLONG_MIN  (-_LIBC_IMPL_LIMITS_LLONG_MAX - 1LL)
#define _LIBC_IMPL_LIMITS_LLONG_MAX  9223372036854775807LL
#define _LIBC_IMPL_LIMITS_ULLONG_MAX 18446744073709551615ULL

#endif // _LIBC_IMPL_LIMITS_H
