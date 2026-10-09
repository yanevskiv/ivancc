/*
 * C source file for the integer types.
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
#include <stdint.h>

// Types the checks compare against.
#include <stddef.h>

// Check the header against the compiler.
typedef char _Stdint_CheckExact[sizeof(int8_t) == 1 && sizeof(int16_t) == 2 && sizeof(int32_t) == 4 && sizeof(int64_t) == 8 ? 1 : -1];
typedef char _Stdint_CheckUnsigned[(uint8_t) -1 > 0 && (uint16_t) -1 > 0 && (uint32_t) -1 > 0 && (uint64_t) -1 > 0 ? 1 : -1];
typedef char _Stdint_CheckSigned[(int8_t) -1 < 0 && (int16_t) -1 < 0 && (int32_t) -1 < 0 && (int64_t) -1 < 0 ? 1 : -1];
typedef char _Stdint_CheckLeast[sizeof(int_least8_t) >= 1 && sizeof(int_least16_t) >= 2 && sizeof(int_least32_t) >= 4 && sizeof(int_least64_t) >= 8 ? 1 : -1];
typedef char _Stdint_CheckFast[sizeof(int_fast8_t) >= 1 && sizeof(int_fast16_t) >= 2 && sizeof(int_fast32_t) >= 4 && sizeof(int_fast64_t) >= 8 ? 1 : -1];
typedef char _Stdint_CheckPointer[sizeof(intptr_t) == sizeof(void *) && sizeof(uintptr_t) == sizeof(void *) ? 1 : -1];
typedef char _Stdint_CheckMax[sizeof(intmax_t) >= sizeof(long long) && sizeof(uintmax_t) >= sizeof(long long) ? 1 : -1];
typedef char _Stdint_CheckLimits[INT32_MAX == (int32_t) (~0U >> 1) && UINT64_MAX == (uint64_t) -1 && INT64_MIN == -INT64_MAX - 1 ? 1 : -1];
typedef char _Stdint_CheckOther[PTRDIFF_MAX == (ptrdiff_t) (~0UL >> 1) && SIZE_MAX == (size_t) -1 && INTPTR_MAX == (intptr_t) (~0UL >> 1) ? 1 : -1];
typedef char _Stdint_CheckConstants[sizeof(INT64_C(1)) >= 8 && sizeof(UINT64_C(1)) >= 8 && sizeof(INTMAX_C(1)) >= 8 && UINT32_C(1) - 2 > 0 ? 1 : -1];
