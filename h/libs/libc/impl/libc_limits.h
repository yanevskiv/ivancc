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

#ifndef __LIBC_IMPL_LIMITS_H__
#define __LIBC_IMPL_LIMITS_H__

// Sizes of integer types
#define __LIBC_IMPL_LIMITS_CHAR_BIT   8
#define __LIBC_IMPL_LIMITS_SCHAR_MIN  (-128)
#define __LIBC_IMPL_LIMITS_SCHAR_MAX  127
#define __LIBC_IMPL_LIMITS_UCHAR_MAX  255
#define __LIBC_IMPL_LIMITS_CHAR_MIN   __LIBC_IMPL_LIMITS_SCHAR_MIN
#define __LIBC_IMPL_LIMITS_CHAR_MAX   __LIBC_IMPL_LIMITS_SCHAR_MAX
#define __LIBC_IMPL_LIMITS_MB_LEN_MAX 16
#define __LIBC_IMPL_LIMITS_SHRT_MIN   (-32768)
#define __LIBC_IMPL_LIMITS_SHRT_MAX   32767
#define __LIBC_IMPL_LIMITS_USHRT_MAX  65535
#define __LIBC_IMPL_LIMITS_INT_MIN    (-__LIBC_IMPL_LIMITS_INT_MAX - 1)
#define __LIBC_IMPL_LIMITS_INT_MAX    2147483647
#define __LIBC_IMPL_LIMITS_UINT_MAX   4294967295U
#define __LIBC_IMPL_LIMITS_LONG_MIN   (-__LIBC_IMPL_LIMITS_LONG_MAX - 1L)
#define __LIBC_IMPL_LIMITS_LONG_MAX   9223372036854775807L
#define __LIBC_IMPL_LIMITS_ULONG_MAX  18446744073709551615UL
#define __LIBC_IMPL_LIMITS_LLONG_MIN  (-__LIBC_IMPL_LIMITS_LLONG_MAX - 1LL)
#define __LIBC_IMPL_LIMITS_LLONG_MAX  9223372036854775807LL
#define __LIBC_IMPL_LIMITS_ULLONG_MAX 18446744073709551615ULL

#endif // __LIBC_IMPL_LIMITS_H__
