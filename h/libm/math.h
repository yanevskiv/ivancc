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

// The bits of a sign byte but its sign, and how far up the sign is.
#define _MATH_SIGN_CLEAR 0x7F
#define _MATH_SIGN_SHIFT 7

// Each format's biased exponent, its field's place and its largest value,
// that of the infinities and NaNs, and its fraction below it.
#define _MATH_FLOAT_EXP_SHIFT   23
#define _MATH_FLOAT_EXP_MAX     0xFFU
#define _MATH_FLOAT_FRAC_MASK   0x7FFFFFU
#define _MATH_DOUBLE_EXP_SHIFT  52
#define _MATH_DOUBLE_EXP_MAX    0x7FFU
#define _MATH_DOUBLE_FRAC_MASK  0xFFFFFFFFFFFFFULL
#define _MATH_LDOUBLE_EXP_MAX   0x7FFFU
#define _MATH_LDOUBLE_FRAC_MASK 0x7FFFFFFFFFFFFFFFULL

// The bytes of a long double that hold its significand, its integer bit
// explicit at the top, and the two above them, its sign and exponent.
#define _MATH_LDOUBLE_SIGNIFICAND 0
#define _MATH_LDOUBLE_TOP         8
#define _MATH_LDOUBLE_INT_SHIFT   63

// Mathematics
// [C99:7.12p3]: HUGE_VAL, HUGE_VALF and HUGE_VALL
// [C99:7.12p4]: INFINITY, a constant expression of float
// [C99:7.12p5]: NAN, a quiet NaN of float
// [C99:7.12p6]: the number classification macros, distinct
// [C99:7.12p7]: FP_FAST_FMA, FP_FAST_FMAF and FP_FAST_FMAL, left out
// [C99:7.12p8]: FP_ILOGB0 and FP_ILOGBNAN, each INT_MIN
// [C99:7.12p9]: MATH_ERRNO, MATH_ERREXCEPT and math_errhandling
// [C99:F.9p2]:  HUGE_VAL and its analogs, positive infinities
// [C99:F.9p4]:  math_errhandling & MATH_ERREXCEPT, nonzero
#define HUGE_VAL         (__builtin_inf())
#define HUGE_VALF        (__builtin_inff())
#define HUGE_VALL        (__builtin_infl())
#define INFINITY         (__builtin_inff())
#define NAN              (__builtin_nanf(""))
#define FP_INFINITE      1
#define FP_NAN           0
#define FP_NORMAL        4
#define FP_SUBNORMAL     3
#define FP_ZERO          2
#define FP_ILOGB0        (-2147483647 - 1)
#define FP_ILOGBNAN      (-2147483647 - 1)
#define MATH_ERRNO       1
#define MATH_ERREXCEPT   2
#define math_errhandling (MATH_ERRNO | MATH_ERREXCEPT)

// Classification macros
// [C99:7.12.3.1]: fpclassify, by its argument's type, as its EXAMPLE does
// [C99:7.12.3.2]: isfinite, FP_ZERO and up being the finite classes
// [C99:7.12.3.3]: isinf
// [C99:7.12.3.4]: isnan
// [C99:7.12.3.5]: isnormal
// [C99:7.12.3.6]: signbit, of a zero, an infinity and a NaN too
#define fpclassify(x)                                          \
    (sizeof (x) == sizeof (float)  ? _Math_ClassifyFloat(x)  : \
     sizeof (x) == sizeof (double) ? _Math_ClassifyDouble(x) : \
                                     _Math_ClassifyLDouble(x))
#define isfinite(x) (fpclassify(x) >= FP_ZERO)
#define isinf(x)    (fpclassify(x) == FP_INFINITE)
#define isnan(x)    (fpclassify(x) == FP_NAN)
#define isnormal(x) (fpclassify(x) == FP_NORMAL)
#define signbit(x)                                             \
    (sizeof (x) == sizeof (float)  ? _Math_ReadSignFloat(x)  : \
     sizeof (x) == sizeof (double) ? _Math_ReadSignDouble(x) : \
                                     _Math_ReadSignLDouble(x))

// Comparison macros
// [C99:7.12.14p1]: quiet versions of the relational operators
// [C99:7.12.14.1]: isgreater, isless reversed
// [C99:7.12.14.2]: isgreaterequal, islessequal reversed
// [C99:7.12.14.3]: isless
// [C99:7.12.14.4]: islessequal
// [C99:7.12.14.5]: islessgreater, each argument evaluated once
// [C99:7.12.14.6]: isunordered
// Each compares in long double, which holds every float and double exactly.
#define isgreater(x, y)      _Math_CompareLess((y), (x))
#define isgreaterequal(x, y) _Math_CompareLessEqual((y), (x))
#define isless(x, y)         _Math_CompareLess((x), (y))
#define islessequal(x, y)    _Math_CompareLessEqual((x), (y))
#define islessgreater(x, y)  _Math_CompareLessGreater((x), (y))
#define isunordered(x, y)    _Math_CompareUnordered((x), (y))

// Mathematics
// [C99:7.12p2]: float_t and double_t, float and double, FLT_EVAL_METHOD 0
typedef float  float_t;
typedef double double_t;

// The classification and sign of a value of each type.
int _Math_ClassifyFields(unsigned int exp, unsigned int exp_max, unsigned long long frac);
int _Math_ClassifyFloat(float x);
int _Math_ClassifyDouble(double x);
int _Math_ClassifyLDouble(long double x);
int _Math_ReadSignFloat(float x);
int _Math_ReadSignDouble(double x);
int _Math_ReadSignLDouble(long double x);

// The quiet comparisons, raising no invalid on a quiet NaN.
int _Math_CompareLess(long double x, long double y);
int _Math_CompareLessEqual(long double x, long double y);
int _Math_CompareLessGreater(long double x, long double y);
int _Math_CompareUnordered(long double x, long double y);

// Absolute value functions
// [C99:7.12.7.2]: fabs, fabsf and fabsl
double      fabs(double x);
float       fabsf(float x);
long double fabsl(long double x);

#endif // _MATH_H
