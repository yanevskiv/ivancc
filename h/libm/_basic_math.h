/*
 * C header file for the basic mathematical functions.
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


#ifndef _BASIC_MATH_H
#define _BASIC_MATH_H

// Exact values and long double fields.
#include <_exact.h>

// The byte of each floating type that holds its sign, in its top bit.
#define _BASIC_MATH_FLOAT_SIGN_BYTE   3
#define _BASIC_MATH_DOUBLE_SIGN_BYTE  7
#define _BASIC_MATH_LDOUBLE_SIGN_BYTE 9

// The bits of a sign byte but its sign, its sign bit, and its sign's place.
#define _BASIC_MATH_SIGN_CLEAR 0x7F
#define _BASIC_MATH_SIGN_BIT   0x80
#define _BASIC_MATH_SIGN_SHIFT 7

// A float's and a double's biased exponent, its field's place and its
// largest value, that of the infinities and NaNs, and its fraction below it.
#define _BASIC_MATH_FLOAT_EXP_SHIFT  23
#define _BASIC_MATH_FLOAT_EXP_MAX    0xFFU
#define _BASIC_MATH_FLOAT_FRAC_MASK  0x7FFFFFU
#define _BASIC_MATH_DOUBLE_EXP_SHIFT 52
#define _BASIC_MATH_DOUBLE_EXP_MAX   0x7FFU
#define _BASIC_MATH_DOUBLE_FRAC_MASK 0xFFFFFFFFFFFFFULL

// The bits of each format's quiet NaN without a payload.
#define _BASIC_MATH_FLOAT_QUIET   0x7FC00000U
#define _BASIC_MATH_DOUBLE_QUIET  0x7FF8000000000000ULL
#define _BASIC_MATH_LDOUBLE_QUIET 0xC000000000000000ULL

// The least subnormal of each format.
#define _BASIC_MATH_FLOAT_TINY   0x1p-149f
#define _BASIC_MATH_DOUBLE_TINY  0x1p-1074
#define _BASIC_MATH_LDOUBLE_TINY 0x1p-16445L

// The least magnitude of a long double that is always an integer.
#define _BASIC_MATH_LDOUBLE_INTEGRAL 0x1p63L

// The distance from an integer at which round goes away from zero.
#define _BASIC_MATH_HALF 0.5L

// A long double's normal exponents and the steps scalbn scales by.
#define _BASIC_MATH_LDOUBLE_EXP_LOW    (-16382)
#define _BASIC_MATH_LDOUBLE_EXP_HIGH   16383
#define _BASIC_MATH_LDOUBLE_STEP_DOWN  16318
#define _BASIC_MATH_LDOUBLE_SCALE_UP   0x1p16383L
#define _BASIC_MATH_LDOUBLE_SCALE_DOWN 0x1p-16318L
#define _BASIC_MATH_SCALE_STEPS        2

// The low bits of the quotient remquo gives, and its odd bit.
#define _BASIC_MATH_QUOTIENT_MASK 7U
#define _BASIC_MATH_QUOTIENT_ODD  1U

// The exponent of a fraction in [1/2, 1).
#define _BASIC_MATH_FRACTION_EXP (-1)

// The terms of the exponential's and the logarithm's series, and the
// magnitude past which e or 2 to x is an infinity or a zero in each type.
#define _BASIC_MATH_EXP_TERMS 15
#define _BASIC_MATH_LOG_TERMS 13
#define _BASIC_MATH_EXP_CLAMP 0x1p15L

// The magnitude past which expm1 of a negative x is -1, the power of 2
// past which its 1 is lost, and the magnitude below which log1p of x is x,
// in long double.
#define _BASIC_MATH_EXPM1_FLOOR 0x1p6L
#define _BASIC_MATH_EXPM1_WIDE  64
#define _BASIC_MATH_LOG1P_TINY  0x1p-64L

// ln 2, its top 48 bits and the rest, log2 e, log10 2, log10 e and the
// square root of 2.
#define _BASIC_MATH_LN2      0xb.17217f7d1cf79acp-4L
#define _BASIC_MATH_LN2_HIGH 0xb.17217f7d1cfp-4L
#define _BASIC_MATH_LN2_LOW  0xf.35793c7673007e6p-53L
#define _BASIC_MATH_LOG2E    0xb.8aa3b295c17f0bcp-3L
#define _BASIC_MATH_LOG10_2  0x9.a209a84fbcff799p-5L
#define _BASIC_MATH_LOG10E   0xd.e5bd8a937287195p-5L
#define _BASIC_MATH_SQRT_TWO 0xb.504f333f9de6484p-3L

// The value of a long double significand's lowest bit, its integer bit 1.
#define _BASIC_MATH_LDOUBLE_UNIT 0x1p-63L

// 2 to 32 plus 1, which splits a long double into two halves of 32 bits.
#define _BASIC_MATH_SPLIT 0x1.00000001p32L

// The steps of the logarithm's table, j/32 for j from 23 to 45, the terms
// of its series pow takes, and the magnitude past which x to y is an
// infinity or a zero for every x but 1 and -1.
#define _BASIC_MATH_LOG_STEPS      32
#define _BASIC_MATH_LOG_STEP_FIRST 23
#define _BASIC_MATH_LOG_STEP_COUNT 23
#define _BASIC_MATH_POW_TERMS      6
#define _BASIC_MATH_POW_CLAMP      0x1p80L

// The magnitude below which the trigonometric functions reduce x by four
// parts of pi/2, those parts, 2/pi and pi/4.
#define _BASIC_MATH_TRIG_NEAR    0x1p30L
#define _BASIC_MATH_HALF_PI_1    0xc.90fdaa2p-3L
#define _BASIC_MATH_HALF_PI_2    0x8.5a308d3p-37L
#define _BASIC_MATH_HALF_PI_3    0x9.8cc51701b839a25p-72L
#define _BASIC_MATH_HALF_PI_4    0x8.127044533e63a01p-138L
#define _BASIC_MATH_TWO_OVER_PI  0xa.2f9836e4e44152ap-4L
#define _BASIC_MATH_QUARTER_PI   0xc.90fdaa22168c235p-4L

// pi/2 and pi, each its nearest long double and the rest.
#define _BASIC_MATH_HALF_PI_HIGH 0xc.90fdaa22168c235p-3L
#define _BASIC_MATH_HALF_PI_LOW  (-0xe.ce675d1fc8f8cbbp-69L)
#define _BASIC_MATH_PI_HIGH      0xc.90fdaa22168c235p-2L
#define _BASIC_MATH_PI_LOW       (-0xe.ce675d1fc8f8cbbp-68L)

// A reduced argument's quadrant's mask, its bit of an odd quarter turn and
// its bit of a half turn.
#define _BASIC_MATH_QUADRANT_MASK 3U
#define _BASIC_MATH_QUADRANT_ODD  1U
#define _BASIC_MATH_QUADRANT_HALF 2U

// The terms of the sine's and the cosine's series, of the arc tangent's, the
// steps of its table, j/16 for j from 0 to 16, and the ratio below which the
// arc tangent of y over a positive x is y over x.
#define _BASIC_MATH_SIN_TERMS       9
#define _BASIC_MATH_COS_TERMS       10
#define _BASIC_MATH_ATAN_TERMS      6
#define _BASIC_MATH_ATAN_STEPS      16
#define _BASIC_MATH_ATAN_STEP_COUNT 17
#define _BASIC_MATH_ATAN_TINY       0x1p-33L

// The magnitude past which a hyperbolic function's smaller exponential is
// lost, and past which an inverse's 1 is lost.
#define _BASIC_MATH_HYPERBOLIC_WIDE     0x1p5L
#define _BASIC_MATH_ARC_HYPERBOLIC_WIDE 0x1p32L

// The first guess of 1/sqrt(m) for m in [1, 4), a quadratic's coefficients,
// and Newton's steps from it.
#define _BASIC_MATH_ROOT_GUESS_0 0x1.5078p+0L
#define _BASIC_MATH_ROOT_GUESS_1 (-0x1.9126p-2L)
#define _BASIC_MATH_ROOT_GUESS_2 0x1.85efp-5L
#define _BASIC_MATH_ROOT_STEPS   4

// The classes fpclassify gives, and ilogb's results of a zero and a NaN.
#define _BASIC_MATH_FP_INFINITE  1
#define _BASIC_MATH_FP_NAN       0
#define _BASIC_MATH_FP_NORMAL    4
#define _BASIC_MATH_FP_SUBNORMAL 3
#define _BASIC_MATH_FP_ZERO      2
#define _BASIC_MATH_FP_ILOGB0    (-2147483647 - 1)
#define _BASIC_MATH_FP_ILOGBNAN  (-2147483647 - 1)

// An int's largest and least values.
#define _BASIC_MATH_INT_MAX 2147483647
#define _BASIC_MATH_INT_MIN (-2147483647 - 1)

// The least normal value of each type.
#define _BASIC_MATH_FLT_MIN  0x1p-126f
#define _BASIC_MATH_DBL_MIN  0x1p-1022
#define _BASIC_MATH_LDBL_MIN 0x1p-16382L

// A value's class and sign by its type, and the tests of its class.
#define _BASIC_MATH_CLASSIFY(x)                                      \
    (sizeof (x) == sizeof (float)  ? _Basic_Math_ClassifyFloat(x)  : \
     sizeof (x) == sizeof (double) ? _Basic_Math_ClassifyDouble(x) : \
                                     _Basic_Math_ClassifyLDouble(x))
#define _BASIC_MATH_SIGNBIT(x)                                       \
    (sizeof (x) == sizeof (float)  ? _Basic_Math_ReadSignFloat(x)  : \
     sizeof (x) == sizeof (double) ? _Basic_Math_ReadSignDouble(x) : \
                                     _Basic_Math_ReadSignLDouble(x))
#define _BASIC_MATH_IS_FINITE(x) (_BASIC_MATH_CLASSIFY(x) >= _BASIC_MATH_FP_ZERO)
#define _BASIC_MATH_IS_INF(x)    (_BASIC_MATH_CLASSIFY(x) == _BASIC_MATH_FP_INFINITE)
#define _BASIC_MATH_IS_NAN(x)    (_BASIC_MATH_CLASSIFY(x) == _BASIC_MATH_FP_NAN)
#define _BASIC_MATH_IS_NORMAL(x) (_BASIC_MATH_CLASSIFY(x) == _BASIC_MATH_FP_NORMAL)

// Which integer quotient a remainder is taken by.
typedef enum _Basic_Math_Quotient _Basic_Math_Quotient;
enum _Basic_Math_Quotient {
    _BASIC_MATH_QUOTIENT_TRUNCATED, // toward zero, fmod's
    _BASIC_MATH_QUOTIENT_NEAREST    // the nearest, ties to even, remainder's
};

// The base of an exponential or a logarithm.
typedef enum _Basic_Math_Base _Basic_Math_Base;
enum _Basic_Math_Base {
    _BASIC_MATH_BASE_E, // e, exp's and log's
    _BASIC_MATH_BASE_2, // 2, exp2's and log2's
    _BASIC_MATH_BASE_10 // 10, log10's
};

// Whether a finite value is an integer, and which.
typedef enum _Basic_Math_Parity _Basic_Math_Parity;
enum _Basic_Math_Parity {
    _BASIC_MATH_PARITY_NONE, // not an integer
    _BASIC_MATH_PARITY_EVEN, // an even integer
    _BASIC_MATH_PARITY_ODD   // an odd integer
};

// Which function of an angle, or its inverse, is taken.
typedef enum _Basic_Math_Trig _Basic_Math_Trig;
enum _Basic_Math_Trig {
    _BASIC_MATH_TRIG_SIN, // the sine's, or its inverse's
    _BASIC_MATH_TRIG_COS, // the cosine's, or its inverse's
    _BASIC_MATH_TRIG_TAN  // the tangent's, or its inverse's
};

// The terms of e to r less 1 over r, 1/k! from k = 1, and of the
// logarithm's series, 1/(2k + 1) from k = 1.
extern const long double _Basic_Math_ExpTerms[_BASIC_MATH_EXP_TERMS];
extern const long double _Basic_Math_LogTerms[_BASIC_MATH_LOG_TERMS];

// The natural logarithm of each step of the logarithm's table, its top 64
// bits and the rest.
extern const long double _Basic_Math_LogStepHigh[_BASIC_MATH_LOG_STEP_COUNT];
extern const long double _Basic_Math_LogStepLow[_BASIC_MATH_LOG_STEP_COUNT];

// The terms of the sine's series, (-1)^k/(2k + 1)! from k = 1, and of the
// cosine's, (-1)^k/(2k)! from k = 1.
extern const long double _Basic_Math_SinTerms[_BASIC_MATH_SIN_TERMS];
extern const long double _Basic_Math_CosTerms[_BASIC_MATH_COS_TERMS];

// The arc tangent of each step of the arc tangent's table, its top 64 bits
// and the rest.
extern const long double _Basic_Math_AtanStepHigh[_BASIC_MATH_ATAN_STEP_COUNT];
extern const long double _Basic_Math_AtanStepLow[_BASIC_MATH_ATAN_STEP_COUNT];

// The classification and sign of a value of each type.
int _Basic_Math_ClassifyFields(unsigned int exp, unsigned int exp_max, unsigned long long frac);
int _Basic_Math_ClassifyFloat(float x);
int _Basic_Math_ClassifyDouble(double x);
int _Basic_Math_ClassifyLDouble(long double x);
int _Basic_Math_ReadSignFloat(float x);
int _Basic_Math_ReadSignDouble(double x);
int _Basic_Math_ReadSignLDouble(long double x);

// The quiet comparisons, raising no invalid on a quiet NaN.
int _Basic_Math_CompareLess(long double x, long double y);
int _Basic_Math_CompareLessEqual(long double x, long double y);
int _Basic_Math_CompareLessGreater(long double x, long double y);
int _Basic_Math_CompareUnordered(long double x, long double y);

// The functions' shared work and errors.
long double        _Basic_Math_ScaleLDouble(long double x, int n);
int                _Basic_Math_ClampExponent(long n);
long double        _Basic_Math_TakeRemainder(long double x, long double y, _Basic_Math_Quotient kind, unsigned int *quo);
unsigned long long _Basic_Math_ReadPayload(const char *tagp);
long double        _Basic_Math_RaiseDomain(long double x, long double y);
long double        _Basic_Math_RaisePole(void);
void               _Basic_Math_CheckScaled(long double x, long double y);
long double        _Basic_Math_CheckRange(long double x, long double y, long double least);
long double        _Basic_Math_CheckPower(long double x, long double y, long double z, long double least);
void               _Basic_Math_CheckNext(long double x, int class);

// The correctly rounded functions, each by one rounding of an exact value.
long double _Basic_Math_TakeRoot(long double x, _Exact_Root degree, _Exact_Type type);
long double _Basic_Math_TakeHypot(long double x, long double y, _Exact_Type type);
long double _Basic_Math_TakeFma(long double x, long double y, long double z, _Exact_Type type);

// The exponentials, logarithms and powers, each by a series in long double.
int                _Basic_Math_ReduceExp(long double x, _Basic_Math_Base base, long double *r);
int                _Basic_Math_SplitFraction(long double x, long double *m);
long double        _Basic_Math_SumExp(long double r);
long double        _Basic_Math_SumLog(long double f);
long double        _Basic_Math_TakeExp(long double x, _Basic_Math_Base base);
long double        _Basic_Math_TakeExpm1(long double x);
long double        _Basic_Math_TakeLog(long double x, _Basic_Math_Base base);
long double        _Basic_Math_TakeLog1p(long double x);
long double        _Basic_Math_SplitHalves(long double x, long double *low);
long double        _Basic_Math_AddParts(long double x, long double y, long double *error);
long double        _Basic_Math_MultiplyParts(long double x, long double y, long double *error);
long double        _Basic_Math_TakeLogParts(long double x, long double *low);
_Basic_Math_Parity _Basic_Math_ReadParity(long double y);
long double        _Basic_Math_TakePow(long double x, long double y);

// The trigonometric and hyperbolic functions and their inverses, each by a
// series in long double.
long double  _Basic_Math_IterateRoot(long double x);
unsigned int _Basic_Math_ReduceTrig(long double x, long double *r, long double *low);
long double  _Basic_Math_SumSin(long double r, long double low);
long double  _Basic_Math_SumCos(long double r, long double low);
long double  _Basic_Math_SumAtan(long double t);
long double  _Basic_Math_TakeTrig(long double x, _Basic_Math_Trig which);
long double  _Basic_Math_TakeAtan2(long double y, long double x);
long double  _Basic_Math_TakeArc(long double x, _Basic_Math_Trig which);
long double  _Basic_Math_TakeHyperbolic(long double x, _Basic_Math_Trig which);
long double  _Basic_Math_TakeArcHyperbolic(long double x, _Basic_Math_Trig which);

// Trigonometric functions
double      __basic_math_acos(double x);
float       __basic_math_acosf(float x);
long double __basic_math_acosl(long double x);
double      __basic_math_asin(double x);
float       __basic_math_asinf(float x);
long double __basic_math_asinl(long double x);
double      __basic_math_atan(double x);
float       __basic_math_atanf(float x);
long double __basic_math_atanl(long double x);
double      __basic_math_atan2(double y, double x);
float       __basic_math_atan2f(float y, float x);
long double __basic_math_atan2l(long double y, long double x);
double      __basic_math_cos(double x);
float       __basic_math_cosf(float x);
long double __basic_math_cosl(long double x);
double      __basic_math_sin(double x);
float       __basic_math_sinf(float x);
long double __basic_math_sinl(long double x);
double      __basic_math_tan(double x);
float       __basic_math_tanf(float x);
long double __basic_math_tanl(long double x);

// Hyperbolic functions
double      __basic_math_acosh(double x);
float       __basic_math_acoshf(float x);
long double __basic_math_acoshl(long double x);
double      __basic_math_asinh(double x);
float       __basic_math_asinhf(float x);
long double __basic_math_asinhl(long double x);
double      __basic_math_atanh(double x);
float       __basic_math_atanhf(float x);
long double __basic_math_atanhl(long double x);
double      __basic_math_cosh(double x);
float       __basic_math_coshf(float x);
long double __basic_math_coshl(long double x);
double      __basic_math_sinh(double x);
float       __basic_math_sinhf(float x);
long double __basic_math_sinhl(long double x);
double      __basic_math_tanh(double x);
float       __basic_math_tanhf(float x);
long double __basic_math_tanhl(long double x);

// Exponential and logarithmic functions
double      __basic_math_exp(double x);
float       __basic_math_expf(float x);
long double __basic_math_expl(long double x);
double      __basic_math_exp2(double x);
float       __basic_math_exp2f(float x);
long double __basic_math_exp2l(long double x);
double      __basic_math_expm1(double x);
float       __basic_math_expm1f(float x);
long double __basic_math_expm1l(long double x);
double      __basic_math_frexp(double value, int *exp);
float       __basic_math_frexpf(float value, int *exp);
long double __basic_math_frexpl(long double value, int *exp);
int         __basic_math_ilogb(double x);
int         __basic_math_ilogbf(float x);
int         __basic_math_ilogbl(long double x);
double      __basic_math_ldexp(double x, int exp);
float       __basic_math_ldexpf(float x, int exp);
long double __basic_math_ldexpl(long double x, int exp);
double      __basic_math_log(double x);
float       __basic_math_logf(float x);
long double __basic_math_logl(long double x);
double      __basic_math_log10(double x);
float       __basic_math_log10f(float x);
long double __basic_math_log10l(long double x);
double      __basic_math_log1p(double x);
float       __basic_math_log1pf(float x);
long double __basic_math_log1pl(long double x);
double      __basic_math_log2(double x);
float       __basic_math_log2f(float x);
long double __basic_math_log2l(long double x);
double      __basic_math_logb(double x);
float       __basic_math_logbf(float x);
long double __basic_math_logbl(long double x);
double      __basic_math_modf(double value, double *iptr);
float       __basic_math_modff(float value, float *iptr);
long double __basic_math_modfl(long double value, long double *iptr);
double      __basic_math_scalbn(double x, int n);
float       __basic_math_scalbnf(float x, int n);
long double __basic_math_scalbnl(long double x, int n);
double      __basic_math_scalbln(double x, long int n);
float       __basic_math_scalblnf(float x, long int n);
long double __basic_math_scalblnl(long double x, long int n);

// Power and absolute-value functions
double      __basic_math_cbrt(double x);
float       __basic_math_cbrtf(float x);
long double __basic_math_cbrtl(long double x);
double      __basic_math_fabs(double x);
float       __basic_math_fabsf(float x);
long double __basic_math_fabsl(long double x);
double      __basic_math_hypot(double x, double y);
float       __basic_math_hypotf(float x, float y);
long double __basic_math_hypotl(long double x, long double y);
double      __basic_math_pow(double x, double y);
float       __basic_math_powf(float x, float y);
long double __basic_math_powl(long double x, long double y);
double      __basic_math_sqrt(double x);
float       __basic_math_sqrtf(float x);
long double __basic_math_sqrtl(long double x);

// Error and gamma functions
double      __basic_math_erf(double x);
float       __basic_math_erff(float x);
long double __basic_math_erfl(long double x);
double      __basic_math_erfc(double x);
float       __basic_math_erfcf(float x);
long double __basic_math_erfcl(long double x);
double      __basic_math_lgamma(double x);
float       __basic_math_lgammaf(float x);
long double __basic_math_lgammal(long double x);
double      __basic_math_tgamma(double x);
float       __basic_math_tgammaf(float x);
long double __basic_math_tgammal(long double x);

// Nearest integer functions
double        __basic_math_ceil(double x);
float         __basic_math_ceilf(float x);
long double   __basic_math_ceill(long double x);
double        __basic_math_floor(double x);
float         __basic_math_floorf(float x);
long double   __basic_math_floorl(long double x);
double        __basic_math_nearbyint(double x);
float         __basic_math_nearbyintf(float x);
long double   __basic_math_nearbyintl(long double x);
double        __basic_math_rint(double x);
float         __basic_math_rintf(float x);
long double   __basic_math_rintl(long double x);
long int      __basic_math_lrint(double x);
long int      __basic_math_lrintf(float x);
long int      __basic_math_lrintl(long double x);
long long int __basic_math_llrint(double x);
long long int __basic_math_llrintf(float x);
long long int __basic_math_llrintl(long double x);
double        __basic_math_round(double x);
float         __basic_math_roundf(float x);
long double   __basic_math_roundl(long double x);
long int      __basic_math_lround(double x);
long int      __basic_math_lroundf(float x);
long int      __basic_math_lroundl(long double x);
long long int __basic_math_llround(double x);
long long int __basic_math_llroundf(float x);
long long int __basic_math_llroundl(long double x);
double        __basic_math_trunc(double x);
float         __basic_math_truncf(float x);
long double   __basic_math_truncl(long double x);

// Remainder functions
double      __basic_math_fmod(double x, double y);
float       __basic_math_fmodf(float x, float y);
long double __basic_math_fmodl(long double x, long double y);
double      __basic_math_remainder(double x, double y);
float       __basic_math_remainderf(float x, float y);
long double __basic_math_remainderl(long double x, long double y);
double      __basic_math_remquo(double x, double y, int *quo);
float       __basic_math_remquof(float x, float y, int *quo);
long double __basic_math_remquol(long double x, long double y, int *quo);

// Manipulation functions
double      __basic_math_copysign(double x, double y);
float       __basic_math_copysignf(float x, float y);
long double __basic_math_copysignl(long double x, long double y);
double      __basic_math_nan(const char *tagp);
float       __basic_math_nanf(const char *tagp);
long double __basic_math_nanl(const char *tagp);
double      __basic_math_nextafter(double x, double y);
float       __basic_math_nextafterf(float x, float y);
long double __basic_math_nextafterl(long double x, long double y);
double      __basic_math_nexttoward(double x, long double y);
float       __basic_math_nexttowardf(float x, long double y);
long double __basic_math_nexttowardl(long double x, long double y);

// Maximum, minimum, and positive difference functions
double      __basic_math_fdim(double x, double y);
float       __basic_math_fdimf(float x, float y);
long double __basic_math_fdiml(long double x, long double y);
double      __basic_math_fmax(double x, double y);
float       __basic_math_fmaxf(float x, float y);
long double __basic_math_fmaxl(long double x, long double y);
double      __basic_math_fmin(double x, double y);
float       __basic_math_fminf(float x, float y);
long double __basic_math_fminl(long double x, long double y);

// Floating multiply-add
double      __basic_math_fma(double x, double y, double z);
float       __basic_math_fmaf(float x, float y, float z);
long double __basic_math_fmal(long double x, long double y, long double z);

#endif // _BASIC_MATH_H
