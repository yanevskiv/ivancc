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

#ifndef __LIMITS_H__
#define __LIMITS_H__

// The implementation.
#include <libc/impl/libc_limits.h>

// (S7.10) Sizes of integer types
#define CHAR_BIT _LIBC_IMPL_LIMITS_CHAR_BIT
#define SCHAR_MIN _LIBC_IMPL_LIMITS_SCHAR_MIN
#define SCHAR_MAX _LIBC_IMPL_LIMITS_SCHAR_MAX
#define UCHAR_MAX _LIBC_IMPL_LIMITS_UCHAR_MAX
#define CHAR_MIN _LIBC_IMPL_LIMITS_CHAR_MIN
#define CHAR_MAX _LIBC_IMPL_LIMITS_CHAR_MAX
#define MB_LEN_MAX _LIBC_IMPL_LIMITS_MB_LEN_MAX
#define SHRT_MIN _LIBC_IMPL_LIMITS_SHRT_MIN
#define SHRT_MAX _LIBC_IMPL_LIMITS_SHRT_MAX
#define USHRT_MAX _LIBC_IMPL_LIMITS_USHRT_MAX
#define INT_MIN _LIBC_IMPL_LIMITS_INT_MIN
#define INT_MAX _LIBC_IMPL_LIMITS_INT_MAX
#define UINT_MAX _LIBC_IMPL_LIMITS_UINT_MAX
#define LONG_MIN _LIBC_IMPL_LIMITS_LONG_MIN
#define LONG_MAX _LIBC_IMPL_LIMITS_LONG_MAX
#define ULONG_MAX _LIBC_IMPL_LIMITS_ULONG_MAX
#define LLONG_MIN _LIBC_IMPL_LIMITS_LLONG_MIN
#define LLONG_MAX _LIBC_IMPL_LIMITS_LLONG_MAX
#define ULLONG_MAX _LIBC_IMPL_LIMITS_ULLONG_MAX

#endif // __LIMITS_H__
