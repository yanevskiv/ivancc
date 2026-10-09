/*
 * C source file for the sizes of integer types.
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

// Module header.
#include <limits.h>

// Check the header against the compiler.
typedef char _Limits_CheckByte[CHAR_BIT == 8 && sizeof(char) * CHAR_BIT == 8 ? 1 : -1];
typedef char _Limits_CheckChar[((char) -1 < 0) == (CHAR_MIN < 0) && CHAR_MAX == (char) CHAR_MAX ? 1 : -1];
typedef char _Limits_CheckSchar[SCHAR_MAX == (signed char) SCHAR_MAX && SCHAR_MIN == -SCHAR_MAX - 1 ? 1 : -1];
typedef char _Limits_CheckUchar[UCHAR_MAX == (unsigned char) -1 ? 1 : -1];
typedef char _Limits_CheckShort[SHRT_MAX == (short) SHRT_MAX && SHRT_MIN == -SHRT_MAX - 1 && USHRT_MAX == (unsigned short) -1 ? 1 : -1];
typedef char _Limits_CheckInt[INT_MAX == (int) (~0U >> 1) && INT_MIN == -INT_MAX - 1 && UINT_MAX == ~0U ? 1 : -1];
typedef char _Limits_CheckLong[LONG_MAX == (long) (~0UL >> 1) && LONG_MIN == -LONG_MAX - 1 && ULONG_MAX == ~0UL ? 1 : -1];
typedef char _Limits_CheckLongLong[LLONG_MAX == (long long) (~0ULL >> 1) && LLONG_MIN == -LLONG_MAX - 1 && ULLONG_MAX == ~0ULL ? 1 : -1];
