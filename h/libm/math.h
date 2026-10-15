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

// Exact values and long double fields.
#include <_exact.h>

// The byte of each floating type that holds its sign, in its top bit.
#define _MATH_FLOAT_SIGN_BYTE   3
#define _MATH_DOUBLE_SIGN_BYTE  7
#define _MATH_LDOUBLE_SIGN_BYTE 9

// The bits of a sign byte but its sign, its sign bit, and its sign's place.
#define _MATH_SIGN_CLEAR 0x7F
#define _MATH_SIGN_BIT   0x80
#define _MATH_SIGN_SHIFT 7

// A float's and a double's biased exponent, its field's place and its
// largest value, that of the infinities and NaNs, and its fraction below it.
#define _MATH_FLOAT_EXP_SHIFT  23
#define _MATH_FLOAT_EXP_MAX    0xFFU
#define _MATH_FLOAT_FRAC_MASK  0x7FFFFFU
#define _MATH_DOUBLE_EXP_SHIFT 52
#define _MATH_DOUBLE_EXP_MAX   0x7FFU
#define _MATH_DOUBLE_FRAC_MASK 0xFFFFFFFFFFFFFULL

// The bits of each format's quiet NaN without a payload.
#define _MATH_FLOAT_QUIET   0x7FC00000U
#define _MATH_DOUBLE_QUIET  0x7FF8000000000000ULL
#define _MATH_LDOUBLE_QUIET 0xC000000000000000ULL

// The least subnormal of each format.
#define _MATH_FLOAT_TINY   0x1p-149f
#define _MATH_DOUBLE_TINY  0x1p-1074
#define _MATH_LDOUBLE_TINY 0x1p-16445L

// The least magnitude of a long double that is always an integer.
#define _MATH_LDOUBLE_INTEGRAL 0x1p63L

// The distance from an integer at which round goes away from zero.
#define _MATH_HALF 0.5L

// A long double's normal exponents and the steps scalbn scales by.
#define _MATH_LDOUBLE_EXP_LOW    (-16382)
#define _MATH_LDOUBLE_EXP_HIGH   16383
#define _MATH_LDOUBLE_STEP_DOWN  16318
#define _MATH_LDOUBLE_SCALE_UP   0x1p16383L
#define _MATH_LDOUBLE_SCALE_DOWN 0x1p-16318L
#define _MATH_SCALE_STEPS        2

// The low bits of the quotient remquo gives, and its odd bit.
#define _MATH_QUOTIENT_MASK 7U
#define _MATH_QUOTIENT_ODD  1U

// The exponent of a fraction in [1/2, 1).
#define _MATH_FRACTION_EXP (-1)

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

// Which integer quotient a remainder is taken by.
typedef enum _Math_Quotient _Math_Quotient;
enum _Math_Quotient {
    _MATH_QUOTIENT_TRUNCATED, // toward zero, fmod's
    _MATH_QUOTIENT_NEAREST    // the nearest, ties to even, remainder's
};

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

// The exact functions' shared work and errors.
long double        _Math_ScaleLDouble(long double x, int n);
int                _Math_ClampExponent(long n);
long double        _Math_TakeRemainder(long double x, long double y, _Math_Quotient kind, unsigned int *quo);
unsigned long long _Math_ReadPayload(const char *tagp);
long double        _Math_RaiseDomain(long double x, long double y);
void               _Math_CheckScaled(long double x, long double y);
void               _Math_CheckNext(long double x, int class);

// The correctly rounded functions, each by one rounding of an exact value.
long double _Math_TakeRoot(long double x, _Exact_Root degree, _Exact_Type type);
long double _Math_TakeHypot(long double x, long double y, _Exact_Type type);
long double _Math_TakeFma(long double x, long double y, long double z, _Exact_Type type);

// Trigonometric functions
// [C99:7.12.1p2]:  a domain error, of an infinity too by its footnote 203
// [C99:7.12.4.1]:  acos, in [0, pi], a domain error outside [-1, +1]
// [C99:7.12.4.2]:  asin, in [-pi/2, +pi/2], the same
// [C99:7.12.4.3]:  atan, in [-pi/2, +pi/2]
// [C99:7.12.4.4]:  atan2, of y/x in [-pi, +pi] by both signs
// [C99:7.12.4.5]:  cos, of x in radians
// [C99:7.12.4.6]:  sin, the same
// [C99:7.12.4.7]:  tan, the same
// [C99:F.9.1.1]:   acos(1) +0, invalid for |x| > 1
// [C99:F.9.1.2]:   asin of a zero, invalid for |x| > 1
// [C99:F.9.1.3]:   atan of a zero and an infinity
// [C99:F.9.1.4]:   atan2 of zeros and infinities, raising nothing of zeros
// [C99:F.9.1.5]:   cos of a zero, invalid of an infinity
// [C99:F.9.1.6]:   sin, the same
// [C99:F.9.1.7]:   tan, the same
double      acos(double x);
float       acosf(float x);
long double acosl(long double x);
double      asin(double x);
float       asinf(float x);
long double asinl(long double x);
double      atan(double x);
float       atanf(float x);
long double atanl(long double x);
double      atan2(double y, double x);
float       atan2f(float y, float x);
long double atan2l(long double y, long double x);
double      cos(double x);
float       cosf(float x);
long double cosl(long double x);
double      sin(double x);
float       sinf(float x);
long double sinl(long double x);
double      tan(double x);
float       tanf(float x);
long double tanl(long double x);

// Hyperbolic functions
// [C99:7.12.1p4]:  an exact infinity from finite arguments, divide-by-zero
// [C99:7.12.5.1]:  acosh, in [0, +inf], a domain error below 1
// [C99:7.12.5.2]:  asinh
// [C99:7.12.5.3]:  atanh, a domain error outside [-1, +1], a range error of 1
// [C99:7.12.5.4]:  cosh, a range error if x's magnitude is too large
// [C99:7.12.5.5]:  sinh, the same
// [C99:7.12.5.6]:  tanh
// [C99:F.9.2.1]:   acosh of 1 and +inf, invalid below 1
// [C99:F.9.2.2]:   asinh of a zero and an infinity
// [C99:F.9.2.3]:   atanh of a zero, and of 1 divide-by-zero, invalid past 1
// [C99:F.9.2.4]:   cosh of a zero and an infinity
// [C99:F.9.2.5]:   sinh, the same
// [C99:F.9.2.6]:   tanh of a zero, and of an infinity 1
double      acosh(double x);
float       acoshf(float x);
long double acoshl(long double x);
double      asinh(double x);
float       asinhf(float x);
long double asinhl(long double x);
double      atanh(double x);
float       atanhf(float x);
long double atanhl(long double x);
double      cosh(double x);
float       coshf(float x);
long double coshl(long double x);
double      sinh(double x);
float       sinhf(float x);
long double sinhl(long double x);
double      tanh(double x);
float       tanhf(float x);
long double tanhl(long double x);

// Exponential and logarithmic functions
// [C99:7.12.1p2]:  a domain error, EDOM and invalid
// [C99:7.12.1p4]:  an overflow, HUGE_VAL, ERANGE and overflow
// [C99:7.12.1p5]:  an underflow, ERANGE implementation-defined
// [C99:7.12.6.1]:  exp, a range error if x's magnitude is too large
// [C99:7.12.6.2]:  exp2, the same
// [C99:7.12.6.3]:  expm1, e to x minus 1, a range error if x is too large
// [C99:7.12.6.4]:  frexp, a fraction in [1/2, 1) or zero, and a power of 2
// [C99:7.12.6.5]:  ilogb, logb's exponent as an int
// [C99:7.12.6.6]:  ldexp, x times 2 to exp
// [C99:7.12.6.7]:  log, a domain error below zero, a range error at zero
// [C99:7.12.6.8]:  log10, the same
// [C99:7.12.6.9]:  log1p, of 1 plus x, a domain error below -1
// [C99:7.12.6.10]: log2, as log
// [C99:7.12.6.11]: logb, the exponent of x, a subnormal as if normalized
// [C99:7.12.6.12]: modf, the integral and fractional parts, x's sign each
// [C99:7.12.6.13]: scalbn and scalbln, x times FLT_RADIX to n
// [C99:F.9.3.1]:   exp of a zero and an infinity
// [C99:F.9.3.2]:   exp2, the same
// [C99:F.9.3.3]:   expm1 of a zero and an infinity
// [C99:F.9.3.4]:   frexp of a zero, an infinity or a NaN, raising nothing
// [C99:F.9.3.5]:   ilogb, invalid when the result is outside int
// [C99:F.9.3.6]:   ldexp, scalbn on a binary system
// [C99:F.9.3.7]:   log of a zero, 1, a negative and +inf
// [C99:F.9.3.8]:   log10, the same
// [C99:F.9.3.9]:   log1p of a zero, -1, below -1 and +inf
// [C99:F.9.3.10]:  log2, as log
// [C99:F.9.3.11]:  logb of a zero, -inf and divide-by-zero
// [C99:F.9.3.12]:  modf of an infinity and a NaN
// [C99:F.9.3.13]:  scalbn of a zero, an infinity, and by 0
double      exp(double x);
float       expf(float x);
long double expl(long double x);
double      exp2(double x);
float       exp2f(float x);
long double exp2l(long double x);
double      expm1(double x);
float       expm1f(float x);
long double expm1l(long double x);
double      frexp(double value, int *exp);
float       frexpf(float value, int *exp);
long double frexpl(long double value, int *exp);
int         ilogb(double x);
int         ilogbf(float x);
int         ilogbl(long double x);
double      ldexp(double x, int exp);
float       ldexpf(float x, int exp);
long double ldexpl(long double x, int exp);
double      log(double x);
float       logf(float x);
long double logl(long double x);
double      log10(double x);
float       log10f(float x);
long double log10l(long double x);
double      log1p(double x);
float       log1pf(float x);
long double log1pl(long double x);
double      log2(double x);
float       log2f(float x);
long double log2l(long double x);
double      logb(double x);
float       logbf(float x);
long double logbl(long double x);
double      modf(double value, double *iptr);
float       modff(float value, float *iptr);
long double modfl(long double value, long double *iptr);
double      scalbn(double x, int n);
float       scalbnf(float x, int n);
long double scalbnl(long double x, int n);
double      scalbln(double x, long int n);
float       scalblnf(float x, long int n);
long double scalblnl(long double x, long int n);

// Power and absolute-value functions
// [C99:7.12.7.1]: cbrt, the real cube root
// [C99:7.12.7.2]: fabs, fabsf and fabsl
// [C99:7.12.7.3]: hypot, without undue overflow or underflow
// [C99:7.12.7.4]: pow, a domain error of a negative x and a non-integer y
// [C99:7.12.7.5]: sqrt, a domain error below zero
// [C99:F.3p1]:    sqrt, IEC 60559's square root
// [C99:F.9.4.1]:  cbrt of a zero and an infinity
// [C99:F.9.4.3]:  hypot of a zero, and of an infinity even with a NaN
// [C99:F.9.4.4]:  pow of a zero, 1, -1, an infinity and a NaN, and by them
// [C99:F.9.4.5]:  sqrt, fully specified by IEC 60559
double      cbrt(double x);
float       cbrtf(float x);
long double cbrtl(long double x);
double      fabs(double x);
float       fabsf(float x);
long double fabsl(long double x);
double      hypot(double x, double y);
float       hypotf(float x, float y);
long double hypotl(long double x, long double y);
double      pow(double x, double y);
float       powf(float x, float y);
long double powl(long double x, long double y);
double      sqrt(double x);
float       sqrtf(float x);
long double sqrtl(long double x);

// Error and gamma functions
// [C99:7.12.8.1]: erf, the error function
// [C99:7.12.8.2]: erfc, 1 - erf x, a range error if x is too large
// [C99:7.12.8.3]: lgamma, ln |gamma x|, a range error if x is too large
// [C99:7.12.8.4]: tgamma, a domain or range error of a negative integer or 0
// [C99:F.9.5.1]:  erf of a zero and an infinity
// [C99:F.9.5.2]:  erfc of an infinity
// [C99:F.9.5.3]:  lgamma of 1, 2 and the infinities, of a pole divide-by-zero
// [C99:F.9.5.4]:  tgamma, a zero divide-by-zero, a negative integer invalid
double      erf(double x);
float       erff(float x);
long double erfl(long double x);
double      erfc(double x);
float       erfcf(float x);
long double erfcl(long double x);
double      lgamma(double x);
float       lgammaf(float x);
long double lgammal(long double x);
double      tgamma(double x);
float       tgammaf(float x);
long double tgammal(long double x);

// Nearest integer functions
// [C99:7.12.9.1]: ceil, the least integer not below x
// [C99:7.12.9.2]: floor, the greatest integer not above x
// [C99:7.12.9.3]: nearbyint, in the current direction, raising no inexact
// [C99:7.12.9.4]: rint, nearbyint that may raise inexact
// [C99:7.12.9.5]: lrint and llrint, rint to an integer type
// [C99:7.12.9.6]: round, halfway cases away from zero
// [C99:7.12.9.7]: lround and llround, round to an integer type
// [C99:7.12.9.8]: trunc, toward zero
// [C99:F.9.6.1]:  ceil of a zero and an infinity
// [C99:F.9.6.2]:  floor of a zero and an infinity
// [C99:F.9.6.3]:  nearbyint of a zero and an infinity, in every direction
// [C99:F.9.6.4]:  rint, inexact if the result differs
// [C99:F.9.6.5]:  lrint and llrint, invalid out of range, else inexact
// [C99:F.9.6.6]:  round of a zero and an infinity
// [C99:F.9.6.7]:  lround and llround, invalid out of range
// [C99:F.9.6.8]:  trunc of a zero and an infinity
double        ceil(double x);
float         ceilf(float x);
long double   ceill(long double x);
double        floor(double x);
float         floorf(float x);
long double   floorl(long double x);
double        nearbyint(double x);
float         nearbyintf(float x);
long double   nearbyintl(long double x);
double        rint(double x);
float         rintf(float x);
long double   rintl(long double x);
long int      lrint(double x);
long int      lrintf(float x);
long int      lrintl(long double x);
long long int llrint(double x);
long long int llrintf(float x);
long long int llrintl(long double x);
double        round(double x);
float         roundf(float x);
long double   roundl(long double x);
long int      lround(double x);
long int      lroundf(float x);
long int      lroundl(long double x);
long long int llround(double x);
long long int llroundf(float x);
long long int llroundl(long double x);
double        trunc(double x);
float         truncf(float x);
long double   truncl(long double x);

// Remainder functions
// [C99:7.12.10.1]: fmod, x - ny, x's sign, less than y in magnitude
// [C99:7.12.10.2]: remainder, IEC 60559's x REM y, exact, ties to even
// [C99:7.12.10.3]: remquo, remainder and the quotient's low bits
// [C99:F.9.7.1]:   fmod of a zero, by an infinity, and invalid
// [C99:F.9.7.2]:   remainder, as IEC 60559's
// [C99:F.9.7.3]:   remquo, as remainder
double      fmod(double x, double y);
float       fmodf(float x, float y);
long double fmodl(long double x, long double y);
double      remainder(double x, double y);
float       remainderf(float x, float y);
long double remainderl(long double x, long double y);
double      remquo(double x, double y, int *quo);
float       remquof(float x, float y, int *quo);
long double remquol(long double x, long double y, int *quo);

// Manipulation functions
// [C99:7.12.11.1]: copysign, x's magnitude and y's sign, of a NaN too
// [C99:7.12.11.2]: nan, strtod of "NAN(n-char-sequence)", else of "NAN"
// [C99:7.12.11.3]: nextafter, the next value after x toward y, y if equal
// [C99:7.12.11.4]: nexttoward, nextafter toward a long double y
// [C99:F.9.8.3]:   nextafter, overflow or underflow, and inexact
double      copysign(double x, double y);
float       copysignf(float x, float y);
long double copysignl(long double x, long double y);
double      nan(const char *tagp);
float       nanf(const char *tagp);
long double nanl(const char *tagp);
double      nextafter(double x, double y);
float       nextafterf(float x, float y);
long double nextafterl(long double x, long double y);
double      nexttoward(double x, long double y);
float       nexttowardf(float x, long double y);
long double nexttowardl(long double x, long double y);

// Maximum, minimum, and positive difference functions
// [C99:7.12.12.1]: fdim, x - y if x > y, else +0
// [C99:7.12.12.2]: fmax, a NaN taken for missing data
// [C99:7.12.12.3]: fmin, the same
// [C99:F.9.9.2]:   fmax, the other argument of one NaN, ideally +0 of zeros
// [C99:F.9.9.3]:   fmin, as fmax
double      fdim(double x, double y);
float       fdimf(float x, float y);
long double fdiml(long double x, long double y);
double      fmax(double x, double y);
float       fmaxf(float x, float y);
long double fmaxl(long double x, long double y);
double      fmin(double x, double y);
float       fminf(float x, float y);
long double fminl(long double x, long double y);

// Floating multiply-add
// [C99:7.12.13.1]: fma, x times y plus z, rounded once in the current mode
// [C99:F.9.10.1]:  fma, invalid of an infinity times a zero, optional by a NaN
double      fma(double x, double y, double z);
float       fmaf(float x, float y, float z);
long double fmal(long double x, long double y, long double z);

#endif // _MATH_H
