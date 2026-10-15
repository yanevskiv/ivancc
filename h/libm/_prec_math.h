/*
 * C header file for the precise mathematical functions.
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

#ifndef _PREC_MATH_H
#define _PREC_MATH_H

// Exact values and long double fields.
#include <_exact.h>

// The bits a working value keeps, the rest truncated.
#define _PREC_MATH_WORK_BITS 160

// A constant's limbs, 192 bits, the least significant first, each
// constant's limbs, and the exponent of its lowest bit.
#define _PREC_MATH_CONSTANTS         6
#define _PREC_MATH_CONST_LIMBS       6
#define _PREC_MATH_LN2_LIMBS         0x7298B62DU, 0x40F34326U, 0x03F2F6AFU, 0xC9E3B398U, 0xD1CF79ABU, 0xB17217F7U
#define _PREC_MATH_LN2_UNIT          (-192)
#define _PREC_MATH_LOG2E_LIMBS       0xDD695A58U, 0xEB577AA8U, 0x691D3E88U, 0xBE87FED0U, 0x5C17F0BBU, 0xB8AA3B29U
#define _PREC_MATH_LOG2E_UNIT        (-191)
#define _PREC_MATH_LOG10E_LIMBS      0x45C9A202U, 0x3EE34602U, 0xAD33DC32U, 0x355BAAAFU, 0x37287195U, 0xDE5BD8A9U
#define _PREC_MATH_LOG10E_UNIT       (-193)
#define _PREC_MATH_PI_LIMBS          0x8A67CC74U, 0x29024E08U, 0x80DC1CD1U, 0xC4C6628BU, 0x2168C234U, 0xC90FDAA2U
#define _PREC_MATH_PI_UNIT           (-190)
#define _PREC_MATH_INV_ROOT_PI_LIMBS 0xAC3E7517U, 0x1409A0EBU, 0x6BFEC344U, 0x71D48A7FU, 0x14DB688DU, 0x906EBA82U
#define _PREC_MATH_INV_ROOT_PI_UNIT  (-192)
#define _PREC_MATH_LN_ROOT_2PI_LIMBS 0x13458B4DU, 0xCFB08F8DU, 0x44192023U, 0x94BC9001U, 0x25F5A534U, 0xEB3F8E43U
#define _PREC_MATH_LN_ROOT_2PI_UNIT  (-192)

// The magnitude past which every exponential overflows or underflows, and
// its exponent.
#define _PREC_MATH_EXP_CLAMP     0x1p15L
#define _PREC_MATH_EXP_CLAMP_EXP 15

// The power pow takes as a square root.
#define _PREC_MATH_POW_ROOT 0.5L

// The halvings of an exponential's reduced argument, and the greatest
// divisor of its series.
#define _PREC_MATH_EXP_HALVINGS 8
#define _PREC_MATH_EXP_TERMS    15

// The terms of a logarithm's series.
#define _PREC_MATH_LOG_TERMS 34

// The greatest exponents of a negative and a positive argument log1p takes
// without forming 1 plus it.
#define _PREC_MATH_LOG1P_LOW_EXP  (-3)
#define _PREC_MATH_LOG1P_HIGH_EXP (-2)

// Ten, and its greatest power a long double holds exactly.
#define _PREC_MATH_TEN          10U
#define _PREC_MATH_LDOUBLE_TENS 27

// The long double Heron steps of a working square root's guess, and the
// Newton steps of its reciprocal past them.
#define _PREC_MATH_ROOT_GUESSES 6
#define _PREC_MATH_ROOT_STEPS   2

// The magnitude below which a function is its argument or 1, nudged, and its
// exponent.
#define _PREC_MATH_TINY     0x1p-60L
#define _PREC_MATH_TINY_EXP (-60)

// The limbs of 2/pi past its point, the most significant first, as many as
// the largest long double's reduction reads, and the limbs one reads.
#define _PREC_MATH_TWO_OVER_PI_LIMBS 519
#define _PREC_MATH_TRIG_WINDOW       10

// The bits of a reduced argument's quadrant, their mask, its bit of an odd
// quarter turn and its bit of a half turn.
#define _PREC_MATH_QUADRANT_BITS 2
#define _PREC_MATH_QUADRANT_MASK 3U
#define _PREC_MATH_QUADRANT_ODD  1U
#define _PREC_MATH_QUADRANT_HALF 2U

// The greatest n of the sine's and the cosine's series' n!.
#define _PREC_MATH_TRIG_TERMS 44

// The halvings of an arc tangent's argument, and the terms of its series.
#define _PREC_MATH_ATAN_HALVINGS 2
#define _PREC_MATH_ATAN_TERMS    36

// The quarters of pi in pi/4, pi/2, 3pi/4 and pi, and a quarter's exponent.
#define _PREC_MATH_QUARTER_PI       1U
#define _PREC_MATH_HALF_PI          2U
#define _PREC_MATH_THREE_QUARTER_PI 3U
#define _PREC_MATH_WHOLE_PI         4U
#define _PREC_MATH_QUARTER_EXP      (-2)

// The magnitude below which erf takes its series, and the series' terms, a
// base and more for each unit of the magnitude.
#define _PREC_MATH_ERF_SPLIT      6.0L
#define _PREC_MATH_ERF_TERMS_BASE 20U
#define _PREC_MATH_ERF_TERMS_STEP 25U

// The terms of erfc's continued fraction from the split on.
#define _PREC_MATH_ERFC_TERMS 80

// The fraction of an x past which sin(pi x) is taken as a cosine, and the
// half its distance to the nearest integer reaches.
#define _PREC_MATH_SINE_TURN 0.25L
#define _PREC_MATH_HALF      0.5L

// The least argument Stirling's series takes, and its terms.
#define _PREC_MATH_STIRLING_FROM  24
#define _PREC_MATH_STIRLING_TERMS 30

// The greatest integer whose gamma is taken as an exact factorial.
#define _PREC_MATH_FACTORIAL_MAX 64

// Which base an exponential or a logarithm takes.
typedef enum _Prec_Math_Base _Prec_Math_Base;
enum _Prec_Math_Base {
    _PREC_MATH_BASE_E,  // exp's and log's
    _PREC_MATH_BASE_2,  // exp2's and log2's
    _PREC_MATH_BASE_10  // log10's
};

// Which constant a working value is made of.
typedef enum _Prec_Math_Constant _Prec_Math_Constant;
enum _Prec_Math_Constant {
    _PREC_MATH_CONST_LN2,         // the natural logarithm of 2
    _PREC_MATH_CONST_LOG2E,       // the base-2 logarithm of e
    _PREC_MATH_CONST_LOG10E,      // the base-10 logarithm of e
    _PREC_MATH_CONST_PI,          // pi
    _PREC_MATH_CONST_INV_ROOT_PI, // 1 over the square root of pi
    _PREC_MATH_CONST_LN_ROOT_2PI  // the natural logarithm of the root of 2 pi
};

// Which trigonometric or hyperbolic function is taken, by its circular name.
typedef enum _Prec_Math_Trig _Prec_Math_Trig;
enum _Prec_Math_Trig {
    _PREC_MATH_TRIG_SIN, // the sine's, or its inverse's
    _PREC_MATH_TRIG_COS, // the cosine's, or its inverse's
    _PREC_MATH_TRIG_TAN  // the tangent's, or its inverse's
};

// Whether a power's exponent is an integer, and which.
typedef enum _Prec_Math_Parity _Prec_Math_Parity;
enum _Prec_Math_Parity {
    _PREC_MATH_PARITY_NONE, // not an integer
    _PREC_MATH_PARITY_EVEN, // an even integer
    _PREC_MATH_PARITY_ODD   // an odd integer
};

// The bits of 2/pi past its point, 32 to a limb, the most significant first.
extern const unsigned int _Prec_Math_TwoOverPi[_PREC_MATH_TWO_OVER_PI_LIMBS];

// The magnitudes of Stirling's coefficients, B2k over 2k(2k - 1), as limbs
// of constants, and their units; the first is positive, and signs alternate.
extern const unsigned int _Prec_Math_Stirling[_PREC_MATH_STIRLING_TERMS][_PREC_MATH_CONST_LIMBS];
extern const int          _Prec_Math_StirlingUnits[_PREC_MATH_STIRLING_TERMS];

// The approximated functions' working values, their cores and their errors.
void              _Prec_Math_TrimExact(_Exact_Value *e);
void              _Prec_Math_WorkMultiply(_Exact_Value *product, const _Exact_Value *a, const _Exact_Value *b);
void              _Prec_Math_WorkAdd(_Exact_Value *sum, const _Exact_Value *a, const _Exact_Value *b);
void              _Prec_Math_WorkDivide(_Exact_Value *quotient, const _Exact_Value *a, const _Exact_Value *b);
void              _Prec_Math_WorkDivideSmall(_Exact_Value *e, unsigned int d);
void              _Prec_Math_MakeReciprocal(_Exact_Value *e, unsigned int d);
void              _Prec_Math_MakeConstant(_Exact_Value *e, _Prec_Math_Constant which);
long              _Prec_Math_ReadNearest(const _Exact_Value *e);
long              _Prec_Math_ReduceExp(_Exact_Value *r, const _Exact_Value *t);
void              _Prec_Math_ExpM1Exact(_Exact_Value *em1, const _Exact_Value *r);
void              _Prec_Math_ExpExact(_Exact_Value *value, const _Exact_Value *t);
void              _Prec_Math_ExpM1Clamped(_Exact_Value *em1, const _Exact_Value *t);
int               _Prec_Math_SplitLog(_Exact_Value *num, _Exact_Value *den, const _Exact_Value *u);
void              _Prec_Math_LogExact(_Exact_Value *log, const _Exact_Value *num, const _Exact_Value *den, int e);
void              _Prec_Math_Log1pExact(_Exact_Value *log, const _Exact_Value *u);
_Bool             _Prec_Math_IsPowerTen(long double x, long k);
_Prec_Math_Parity _Prec_Math_ReadParity(long double y);
_Bool             _Prec_Math_PowerExact(_Exact_Value *value, long double x, long double y);
long double       _Prec_Math_RaiseDomain(long double x, long double y);
long double       _Prec_Math_RaisePole(void);
long double       _Prec_Math_RoundRange(const _Exact_Value *value, _Exact_Type type);
long double       _Prec_Math_TakeExp(long double x, _Prec_Math_Base base, _Exact_Type type);
long double       _Prec_Math_TakeExpm1(long double x, _Exact_Type type);
long double       _Prec_Math_TakeLog(long double x, _Prec_Math_Base base, _Exact_Type type);
long double       _Prec_Math_TakeLog1p(long double x, _Exact_Type type);
long double       _Prec_Math_TakePow(long double x, long double y, _Exact_Type type);

// The trigonometric and hyperbolic functions' cores.
void         _Prec_Math_NudgeExact(_Exact_Value *e, _Exact_Step step);
void         _Prec_Math_WorkRoot(_Exact_Value *root, const _Exact_Value *a);
void         _Prec_Math_MakeQuarters(_Exact_Value *e, unsigned int quarters);
unsigned int _Prec_Math_ReduceTrig(_Exact_Value *r, long double x);
void         _Prec_Math_TrigExact(_Exact_Value *value, const _Exact_Value *r, _Prec_Math_Trig which);
void         _Prec_Math_AtanExact(_Exact_Value *value, const _Exact_Value *num, const _Exact_Value *den);
long double  _Prec_Math_TakeTiny(long double x, _Exact_Step step, _Exact_Type type);
long double  _Prec_Math_TakeQuarters(unsigned int quarters, long double sign, _Exact_Type type);
long double  _Prec_Math_TakeTrig(long double x, _Prec_Math_Trig which, _Exact_Type type);
long double  _Prec_Math_TakeArc(long double x, _Prec_Math_Trig which, _Exact_Type type);
long double  _Prec_Math_TakeAtan2(long double y, long double x, _Exact_Type type);
long double  _Prec_Math_TakeHyperbolic(long double x, _Prec_Math_Trig which, _Exact_Type type);
long double  _Prec_Math_TakeArcHyperbolic(long double x, _Prec_Math_Trig which, _Exact_Type type);

// The error and gamma functions' cores.
void        _Prec_Math_ErfExact(_Exact_Value *value, long double a);
void        _Prec_Math_ErfcExact(_Exact_Value *value, long double a);
void        _Prec_Math_MakeStirling(_Exact_Value *e, int k);
void        _Prec_Math_StirlingExact(_Exact_Value *log, const _Exact_Value *y);
_Bool       _Prec_Math_LogGammaExact(_Exact_Value *log, long double x);
long double _Prec_Math_TakeErf(long double x, _Exact_Type type);
long double _Prec_Math_TakeErfc(long double x, _Exact_Type type);
long double _Prec_Math_TakeLgamma(long double x, _Exact_Type type);
long double _Prec_Math_TakeTgamma(long double x, _Exact_Type type);

// Trigonometric functions
double      _prec_acos(double x);
float       _prec_acosf(float x);
long double _prec_acosl(long double x);
double      _prec_asin(double x);
float       _prec_asinf(float x);
long double _prec_asinl(long double x);
double      _prec_atan(double x);
float       _prec_atanf(float x);
long double _prec_atanl(long double x);
double      _prec_atan2(double y, double x);
float       _prec_atan2f(float y, float x);
long double _prec_atan2l(long double y, long double x);
double      _prec_cos(double x);
float       _prec_cosf(float x);
long double _prec_cosl(long double x);
double      _prec_sin(double x);
float       _prec_sinf(float x);
long double _prec_sinl(long double x);
double      _prec_tan(double x);
float       _prec_tanf(float x);
long double _prec_tanl(long double x);

// Hyperbolic functions
double      _prec_acosh(double x);
float       _prec_acoshf(float x);
long double _prec_acoshl(long double x);
double      _prec_asinh(double x);
float       _prec_asinhf(float x);
long double _prec_asinhl(long double x);
double      _prec_atanh(double x);
float       _prec_atanhf(float x);
long double _prec_atanhl(long double x);
double      _prec_cosh(double x);
float       _prec_coshf(float x);
long double _prec_coshl(long double x);
double      _prec_sinh(double x);
float       _prec_sinhf(float x);
long double _prec_sinhl(long double x);
double      _prec_tanh(double x);
float       _prec_tanhf(float x);
long double _prec_tanhl(long double x);

// Exponential and logarithmic functions
double      _prec_exp(double x);
float       _prec_expf(float x);
long double _prec_expl(long double x);
double      _prec_exp2(double x);
float       _prec_exp2f(float x);
long double _prec_exp2l(long double x);
double      _prec_expm1(double x);
float       _prec_expm1f(float x);
long double _prec_expm1l(long double x);
double      _prec_log(double x);
float       _prec_logf(float x);
long double _prec_logl(long double x);
double      _prec_log10(double x);
float       _prec_log10f(float x);
long double _prec_log10l(long double x);
double      _prec_log1p(double x);
float       _prec_log1pf(float x);
long double _prec_log1pl(long double x);
double      _prec_log2(double x);
float       _prec_log2f(float x);
long double _prec_log2l(long double x);

// Power functions
double      _prec_pow(double x, double y);
float       _prec_powf(float x, float y);
long double _prec_powl(long double x, long double y);

// Error and gamma functions
double      _prec_erf(double x);
float       _prec_erff(float x);
long double _prec_erfl(long double x);
double      _prec_erfc(double x);
float       _prec_erfcf(float x);
long double _prec_erfcl(long double x);
double      _prec_lgamma(double x);
float       _prec_lgammaf(float x);
long double _prec_lgammal(long double x);
double      _prec_tgamma(double x);
float       _prec_tgammaf(float x);
long double _prec_tgammal(long double x);

#endif // _PREC_MATH_H
