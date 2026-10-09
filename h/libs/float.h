/*
 * C header file for the characteristics of floating types.
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

#ifndef __FLOAT_H__
#define __FLOAT_H__

// The implementation.
#include <libc/impl/libc_float.h>

// (S7.7) Characteristics of floating types
#define FLT_ROUNDS __LIBC_IMPL_FLOAT_FLT_ROUNDS
#define FLT_EVAL_METHOD __LIBC_IMPL_FLOAT_FLT_EVAL_METHOD
#define FLT_RADIX __LIBC_IMPL_FLOAT_FLT_RADIX

#define FLT_MANT_DIG __LIBC_IMPL_FLOAT_FLT_MANT_DIG
#define DBL_MANT_DIG __LIBC_IMPL_FLOAT_DBL_MANT_DIG
#define LDBL_MANT_DIG __LIBC_IMPL_FLOAT_LDBL_MANT_DIG

#define DECIMAL_DIG __LIBC_IMPL_FLOAT_DECIMAL_DIG

#define FLT_DIG __LIBC_IMPL_FLOAT_FLT_DIG
#define DBL_DIG __LIBC_IMPL_FLOAT_DBL_DIG
#define LDBL_DIG __LIBC_IMPL_FLOAT_LDBL_DIG

#define FLT_MIN_EXP __LIBC_IMPL_FLOAT_FLT_MIN_EXP
#define DBL_MIN_EXP __LIBC_IMPL_FLOAT_DBL_MIN_EXP
#define LDBL_MIN_EXP __LIBC_IMPL_FLOAT_LDBL_MIN_EXP

#define FLT_MIN_10_EXP __LIBC_IMPL_FLOAT_FLT_MIN_10_EXP
#define DBL_MIN_10_EXP __LIBC_IMPL_FLOAT_DBL_MIN_10_EXP
#define LDBL_MIN_10_EXP __LIBC_IMPL_FLOAT_LDBL_MIN_10_EXP

#define FLT_MAX_EXP __LIBC_IMPL_FLOAT_FLT_MAX_EXP
#define DBL_MAX_EXP __LIBC_IMPL_FLOAT_DBL_MAX_EXP
#define LDBL_MAX_EXP __LIBC_IMPL_FLOAT_LDBL_MAX_EXP

#define FLT_MAX_10_EXP __LIBC_IMPL_FLOAT_FLT_MAX_10_EXP
#define DBL_MAX_10_EXP __LIBC_IMPL_FLOAT_DBL_MAX_10_EXP
#define LDBL_MAX_10_EXP __LIBC_IMPL_FLOAT_LDBL_MAX_10_EXP

#define FLT_MAX __LIBC_IMPL_FLOAT_FLT_MAX
#define DBL_MAX __LIBC_IMPL_FLOAT_DBL_MAX
#define LDBL_MAX __LIBC_IMPL_FLOAT_LDBL_MAX

#define FLT_EPSILON __LIBC_IMPL_FLOAT_FLT_EPSILON
#define DBL_EPSILON __LIBC_IMPL_FLOAT_DBL_EPSILON
#define LDBL_EPSILON __LIBC_IMPL_FLOAT_LDBL_EPSILON

#define FLT_MIN __LIBC_IMPL_FLOAT_FLT_MIN
#define DBL_MIN __LIBC_IMPL_FLOAT_DBL_MIN
#define LDBL_MIN __LIBC_IMPL_FLOAT_LDBL_MIN

#endif // __FLOAT_H__
