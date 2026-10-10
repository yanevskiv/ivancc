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

#ifndef _FLOAT_H
#define _FLOAT_H

// (S7.7) Characteristics of floating types
#define FLT_ROUNDS      1
#define FLT_EVAL_METHOD 0
#define FLT_RADIX       2

#define FLT_MANT_DIG  24
#define DBL_MANT_DIG  53
#define LDBL_MANT_DIG 64

#define DECIMAL_DIG 21

#define FLT_DIG  6
#define DBL_DIG  15
#define LDBL_DIG 18

#define FLT_MIN_EXP  (-125)
#define DBL_MIN_EXP  (-1021)
#define LDBL_MIN_EXP (-16381)

#define FLT_MIN_10_EXP  (-37)
#define DBL_MIN_10_EXP  (-307)
#define LDBL_MIN_10_EXP (-4931)

#define FLT_MAX_EXP  128
#define DBL_MAX_EXP  1024
#define LDBL_MAX_EXP 16384

#define FLT_MAX_10_EXP  38
#define DBL_MAX_10_EXP  308
#define LDBL_MAX_10_EXP 4932

#define FLT_MAX  3.40282346638528859811704183484516925e+38F
#define DBL_MAX  ((double) 1.79769313486231570814527423731704357e+308L)
#define LDBL_MAX 1.18973149535723176502126385303097021e+4932L

#define FLT_EPSILON  1.19209289550781250000000000000000000e-7F
#define DBL_EPSILON  ((double) 2.22044604925031308084726333618164062e-16L)
#define LDBL_EPSILON 1.08420217248550443400745280086994171e-19L

#define FLT_MIN  1.17549435082228750796873653722224568e-38F
#define DBL_MIN  ((double) 2.22507385850720138309023271733240406e-308L)
#define LDBL_MIN 3.36210314311209350626267781732175260e-4932L

#endif // _FLOAT_H
