/*
 * C header file for mathematics.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivcc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivcc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivcc; see
 * the files LICENSE and COPYING.RUNTIME respectively.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef _MATH_H
#define _MATH_H

// The byte of each floating type that holds its sign, in its top bit.
#define _MATH_FLOAT_SIGN_BYTE   3
#define _MATH_DOUBLE_SIGN_BYTE  7
#define _MATH_LDOUBLE_SIGN_BYTE 9

// The bits of a sign byte but its sign.
#define _MATH_SIGN_CLEAR 0x7F

// Absolute value functions
// [C99:7.12.7.2]: fabs, fabsf and fabsl
double      fabs(double x);
float       fabsf(float x);
long double fabsl(long double x);

#endif // _MATH_H
