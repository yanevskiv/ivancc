/*
 * C source file for mathematics.
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

// Module header.
#include <math.h>

// Includes.
#include <_exact.h>
#include <_fp.h>
#include <_prec_math.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>

// Classify a value by its biased exponent and its fraction.
int _Math_ClassifyFields(unsigned int exp, unsigned int exp_max, unsigned long long frac)
{
    if (exp == exp_max) {
        return frac ? FP_NAN : FP_INFINITE;
    }
    if (exp == 0) {
        return frac ? FP_SUBNORMAL : FP_ZERO;
    }
    return FP_NORMAL;
}

// Classify x as a float.
int _Math_ClassifyFloat(float x)
{
    uint32_t bits;
    memcpy(&bits, &x, sizeof(bits));
    return _Math_ClassifyFields((bits >> _MATH_FLOAT_EXP_SHIFT) & _MATH_FLOAT_EXP_MAX, _MATH_FLOAT_EXP_MAX, bits & _MATH_FLOAT_FRAC_MASK);
}

// Classify x as a double.
int _Math_ClassifyDouble(double x)
{
    uint64_t bits;
    memcpy(&bits, &x, sizeof(bits));
    return _Math_ClassifyFields((uint32_t) (bits >> _MATH_DOUBLE_EXP_SHIFT) & _MATH_DOUBLE_EXP_MAX, _MATH_DOUBLE_EXP_MAX, bits & _MATH_DOUBLE_FRAC_MASK);
}

// Classify x as a long double, the x87's refused encodings as glibc does.
int _Math_ClassifyLDouble(long double x)
{
    uint64_t sig;
    uint16_t top;
    memcpy(&sig, (unsigned char *) &x + _EXACT_LDOUBLE_SIGNIFICAND, sizeof(sig));
    memcpy(&top, (unsigned char *) &x + _EXACT_LDOUBLE_TOP, sizeof(top));
    uint32_t exp = top & _EXACT_LDOUBLE_EXP_MAX;

    if (exp == 0) {
        return sig >> _EXACT_LDOUBLE_INT_SHIFT ? FP_NORMAL : _Math_ClassifyFields(exp, _EXACT_LDOUBLE_EXP_MAX, sig);
    }
    if (! (sig >> _EXACT_LDOUBLE_INT_SHIFT)) {
        return FP_NAN;
    }
    return _Math_ClassifyFields(exp, _EXACT_LDOUBLE_EXP_MAX, sig & _EXACT_LDOUBLE_FRAC_MASK);
}

// Return 1 if x's sign is negative, else 0.
int _Math_ReadSignFloat(float x)
{
    return ((unsigned char *) &x)[_MATH_FLOAT_SIGN_BYTE] >> _MATH_SIGN_SHIFT;
}

// Return 1 if x's sign is negative, else 0.
int _Math_ReadSignDouble(double x)
{
    return ((unsigned char *) &x)[_MATH_DOUBLE_SIGN_BYTE] >> _MATH_SIGN_SHIFT;
}

// Return 1 if x's sign is negative, else 0.
int _Math_ReadSignLDouble(long double x)
{
    return ((unsigned char *) &x)[_MATH_LDOUBLE_SIGN_BYTE] >> _MATH_SIGN_SHIFT;
}

// Return x < y, compared only once known to be ordered.
int _Math_CompareLess(long double x, long double y)
{
    return ! _Math_CompareUnordered(x, y) && x < y;
}

// Return x <= y, compared only once known to be ordered.
int _Math_CompareLessEqual(long double x, long double y)
{
    return ! _Math_CompareUnordered(x, y) && x <= y;
}

// Return x < y || x > y, compared only once known to be ordered.
int _Math_CompareLessGreater(long double x, long double y)
{
    return ! _Math_CompareUnordered(x, y) && x != y;
}

// Return whether x and y are unordered, by != alone, quiet on a quiet NaN.
int _Math_CompareUnordered(long double x, long double y)
{
    return x != x || y != y;
}

// Scale x by 2 to n, rounding once.
long double _Math_ScaleLDouble(long double x, int n)
{
    for (int i = 0; i < _MATH_SCALE_STEPS && n > _MATH_LDOUBLE_EXP_HIGH; i++) {
        x *= _MATH_LDOUBLE_SCALE_UP;
        n -= _MATH_LDOUBLE_EXP_HIGH;
    }
    for (int i = 0; i < _MATH_SCALE_STEPS && n < _MATH_LDOUBLE_EXP_LOW; i++) {
        x *= _MATH_LDOUBLE_SCALE_DOWN;
        n += _MATH_LDOUBLE_STEP_DOWN;
    }
    n = n > _MATH_LDOUBLE_EXP_HIGH ? _MATH_LDOUBLE_EXP_HIGH : n < _MATH_LDOUBLE_EXP_LOW ? _MATH_LDOUBLE_EXP_LOW : n;
    return x * _Exact_MakeLDouble((unsigned int) (n + _EXACT_LDOUBLE_BIAS), _EXACT_LDOUBLE_INT_BIT);
}

// Clamp a long exponent to an int.
int _Math_ClampExponent(long n)
{
    return n > INT_MAX ? INT_MAX : n < INT_MIN ? INT_MIN : (int) n;
}

// Take the remainder of x by y by a quotient of kind.
long double _Math_TakeRemainder(long double x, long double y, _Math_Quotient kind, unsigned int *quo)
{
    int ex;
    int ey;
    uint32_t q = 0;
    uint64_t carry = 0;
    long double sign = x;
    uint64_t rest = _Exact_SplitLDouble(x, &ex);
    uint64_t div = _Exact_SplitLDouble(y, &ey);

    for (int shift = ex - ey; shift >= 0; shift--) {
        if (carry != 0 || rest >= div) {
            rest -= div;
            q++;
        }
        if (shift > 0) {
            carry = rest & _EXACT_LDOUBLE_INT_BIT;
            rest <<= 1;
            q <<= 1;
        }
    }
    if (kind == _MATH_QUOTIENT_NEAREST && ex >= ey && (rest > div - rest || (rest == div - rest && (q & _MATH_QUOTIENT_ODD) != 0))) {
        rest = div - rest;
        sign = -x;
        q++;
    } else if (kind == _MATH_QUOTIENT_NEAREST && ex == ey - 1 && rest > div) {
        rest = div - (rest - div);
        sign = -x;
        q = _MATH_QUOTIENT_ODD;
    }
    *quo = q;
    return copysignl(_Exact_JoinLDouble(ex < ey ? ex : ey, rest), sign);
}

// Read nan's payload from tagp as strtod reads one from "NAN(tagp)".
unsigned long long _Math_ReadPayload(const char *tagp)
{
    unsigned long len = 0;

    while (tagp[len] == '_' || _Fp_IsAlnum((unsigned char) tagp[len])) {
        len++;
    }
    return tagp[len] == '\0' ? _Fp_ReadPayload(tagp, _FP_TEXT_NARROW, 0, len) : 0;
}

// Raise the domain error of x and y.
long double _Math_RaiseDomain(long double x, long double y)
{
    errno = EDOM;
    return (x * y) / (x * y);
}

// Check y, x scaled, for a range error.
void _Math_CheckScaled(long double x, long double y)
{
    if (isfinite(x) && x != 0 && (isinf(y) || y == 0)) {
        errno = ERANGE;
    }
}

// Check a step from x to a value of class for a range error.
void _Math_CheckNext(long double x, int class)
{
    if (class == FP_INFINITE) {
        feraiseexcept(FE_OVERFLOW | FE_INEXACT);
        errno = ERANGE;
    } else if ((class == FP_SUBNORMAL || class == FP_ZERO) && x != 0) {
        feraiseexcept(FE_UNDERFLOW | FE_INEXACT);
        errno = ERANGE;
    } else if (class == FP_SUBNORMAL) {
        feraiseexcept(FE_UNDERFLOW | FE_INEXACT);
    }
}

// Take x's square or cube root, correctly rounded to type.
long double _Math_TakeRoot(long double x, _Exact_Root degree, _Exact_Type type)
{
    _Exact_Value root;
    _Exact_Value radicand;

    if (isnan(x)) {
        return x + x;
    }
    if (x == 0 || (isinf(x) && (degree == _EXACT_ROOT_CUBE || x > 0))) {
        return x;
    }
    if (x < 0 && degree == _EXACT_ROOT_SQUARE) {
        return _Math_RaiseDomain(x, 0);
    }
    _Exact_MakeValue(&radicand, x);
    _Exact_ExtractRoot(&root, &radicand, degree);
    return _Exact_RoundValue(&root, type);
}

// Take the hypotenuse of x and y, correctly rounded to type.
long double _Math_TakeHypot(long double x, long double y, _Exact_Type type)
{
    _Exact_Value sum;
    _Exact_Value root;
    _Exact_Value side_x;
    _Exact_Value side_y;
    long double result;

    if (isinf(x) || isinf(y)) {
        return HUGE_VALL;
    }
    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (x == 0 || y == 0) {
        return fabsl(x) + fabsl(y);
    }
    _Exact_MakeValue(&side_x, x);
    _Exact_MakeValue(&side_y, y);
    _Exact_MultiplyValues(&side_x, &side_x, &side_x);
    _Exact_MultiplyValues(&side_y, &side_y, &side_y);
    _Exact_AddValues(&sum, &side_x, &side_y);
    _Exact_ExtractRoot(&root, &sum, _EXACT_ROOT_SQUARE);
    result = _Exact_RoundValue(&root, type);
    if (isinf(result)) {
        errno = ERANGE;
    }
    return result;
}

// Take x times y plus z, correctly rounded once to type.
long double _Math_TakeFma(long double x, long double y, long double z, _Exact_Type type)
{
    _Exact_Value sum;
    _Exact_Value addend;
    _Exact_Value product;

    if (isnan(x) || isnan(y)) {
        return x * y + z;
    }
    if (isnan(z)) {
        return z + z;
    }
    if (isinf(x) || isinf(y)) {
        return x * y + z;
    }
    if (isinf(z)) {
        return z;
    }
    if (x == 0 || y == 0) {
        return x * y + z;
    }
    _Exact_MakeValue(&product, x);
    _Exact_MakeValue(&addend, y);
    _Exact_MultiplyValues(&product, &product, &addend);
    if (z == 0) {
        return _Exact_RoundValue(&product, type);
    }
    _Exact_MakeValue(&addend, z);
    _Exact_AddValues(&sum, &product, &addend);
    return _Exact_RoundValue(&sum, type);
}

// Take the arc cosine of x.
double acos(double x)
{
    return __prec_math_acos(x);
}

// Take the arc cosine of x.
float acosf(float x)
{
    return __prec_math_acosf(x);
}

// Take the arc cosine of x.
long double acosl(long double x)
{
    return __prec_math_acosl(x);
}

// Take the arc sine of x.
double asin(double x)
{
    return __prec_math_asin(x);
}

// Take the arc sine of x.
float asinf(float x)
{
    return __prec_math_asinf(x);
}

// Take the arc sine of x.
long double asinl(long double x)
{
    return __prec_math_asinl(x);
}

// Take the arc tangent of x.
double atan(double x)
{
    return __prec_math_atan(x);
}

// Take the arc tangent of x.
float atanf(float x)
{
    return __prec_math_atanf(x);
}

// Take the arc tangent of x.
long double atanl(long double x)
{
    return __prec_math_atanl(x);
}

// Take the arc tangent of y over x by both signs.
double atan2(double y, double x)
{
    return __prec_math_atan2(y, x);
}

// Take the arc tangent of y over x by both signs.
float atan2f(float y, float x)
{
    return __prec_math_atan2f(y, x);
}

// Take the arc tangent of y over x by both signs.
long double atan2l(long double y, long double x)
{
    return __prec_math_atan2l(y, x);
}

// Take the cosine of x.
double cos(double x)
{
    return __prec_math_cos(x);
}

// Take the cosine of x.
float cosf(float x)
{
    return __prec_math_cosf(x);
}

// Take the cosine of x.
long double cosl(long double x)
{
    return __prec_math_cosl(x);
}

// Take the sine of x.
double sin(double x)
{
    return __prec_math_sin(x);
}

// Take the sine of x.
float sinf(float x)
{
    return __prec_math_sinf(x);
}

// Take the sine of x.
long double sinl(long double x)
{
    return __prec_math_sinl(x);
}

// Take the tangent of x.
double tan(double x)
{
    return __prec_math_tan(x);
}

// Take the tangent of x.
float tanf(float x)
{
    return __prec_math_tanf(x);
}

// Take the tangent of x.
long double tanl(long double x)
{
    return __prec_math_tanl(x);
}

// Take the inverse hyperbolic cosine of x.
double acosh(double x)
{
    return __prec_math_acosh(x);
}

// Take the inverse hyperbolic cosine of x.
float acoshf(float x)
{
    return __prec_math_acoshf(x);
}

// Take the inverse hyperbolic cosine of x.
long double acoshl(long double x)
{
    return __prec_math_acoshl(x);
}

// Take the inverse hyperbolic sine of x.
double asinh(double x)
{
    return __prec_math_asinh(x);
}

// Take the inverse hyperbolic sine of x.
float asinhf(float x)
{
    return __prec_math_asinhf(x);
}

// Take the inverse hyperbolic sine of x.
long double asinhl(long double x)
{
    return __prec_math_asinhl(x);
}

// Take the inverse hyperbolic tangent of x.
double atanh(double x)
{
    return __prec_math_atanh(x);
}

// Take the inverse hyperbolic tangent of x.
float atanhf(float x)
{
    return __prec_math_atanhf(x);
}

// Take the inverse hyperbolic tangent of x.
long double atanhl(long double x)
{
    return __prec_math_atanhl(x);
}

// Take the hyperbolic cosine of x.
double cosh(double x)
{
    return __prec_math_cosh(x);
}

// Take the hyperbolic cosine of x.
float coshf(float x)
{
    return __prec_math_coshf(x);
}

// Take the hyperbolic cosine of x.
long double coshl(long double x)
{
    return __prec_math_coshl(x);
}

// Take the hyperbolic sine of x.
double sinh(double x)
{
    return __prec_math_sinh(x);
}

// Take the hyperbolic sine of x.
float sinhf(float x)
{
    return __prec_math_sinhf(x);
}

// Take the hyperbolic sine of x.
long double sinhl(long double x)
{
    return __prec_math_sinhl(x);
}

// Take the hyperbolic tangent of x.
double tanh(double x)
{
    return __prec_math_tanh(x);
}

// Take the hyperbolic tangent of x.
float tanhf(float x)
{
    return __prec_math_tanhf(x);
}

// Take the hyperbolic tangent of x.
long double tanhl(long double x)
{
    return __prec_math_tanhl(x);
}

// Take e to x.
double exp(double x)
{
    return __prec_math_exp(x);
}

// Take e to x.
float expf(float x)
{
    return __prec_math_expf(x);
}

// Take e to x.
long double expl(long double x)
{
    return __prec_math_expl(x);
}

// Take 2 to x.
double exp2(double x)
{
    return __prec_math_exp2(x);
}

// Take 2 to x.
float exp2f(float x)
{
    return __prec_math_exp2f(x);
}

// Take 2 to x.
long double exp2l(long double x)
{
    return __prec_math_exp2l(x);
}

// Take e to x, less 1.
double expm1(double x)
{
    return __prec_math_expm1(x);
}

// Take e to x, less 1.
float expm1f(float x)
{
    return __prec_math_expm1f(x);
}

// Take e to x, less 1.
long double expm1l(long double x)
{
    return __prec_math_expm1l(x);
}

// Break value into a fraction and a power of 2.
double frexp(double value, int *exp)
{
    return (double) frexpl(value, exp);
}

// Break value into a fraction and a power of 2.
float frexpf(float value, int *exp)
{
    return (float) frexpl(value, exp);
}

// Break value into a fraction and a power of 2.
long double frexpl(long double value, int *exp)
{
    int power;
    uint64_t sig;

    *exp = 0;
    if (! isfinite(value) || value == 0) {
        return value + value;
    }
    sig = _Exact_SplitLDouble(value, &power);
    *exp = power - _MATH_FRACTION_EXP;
    return copysignl(_Exact_JoinLDouble(_MATH_FRACTION_EXP, sig), value);
}

// Extract x's exponent as an int.
int ilogb(double x)
{
    return ilogbl(x);
}

// Extract x's exponent as an int.
int ilogbf(float x)
{
    return ilogbl(x);
}

// Extract x's exponent as an int.
int ilogbl(long double x)
{
    int exp;
    int class = fpclassify(x);

    if (class == FP_NAN || class == FP_INFINITE || class == FP_ZERO) {
        errno = EDOM;
        feraiseexcept(FE_INVALID);
        return class == FP_INFINITE ? INT_MAX : class == FP_ZERO ? FP_ILOGB0 : FP_ILOGBNAN;
    }
    _Exact_SplitLDouble(x, &exp);
    return exp;
}

// Multiply x by 2 to exp.
double ldexp(double x, int exp)
{
    return scalbn(x, exp);
}

// Multiply x by 2 to exp.
float ldexpf(float x, int exp)
{
    return scalbnf(x, exp);
}

// Multiply x by 2 to exp.
long double ldexpl(long double x, int exp)
{
    return scalbnl(x, exp);
}

// Take the natural logarithm of x.
double log(double x)
{
    return __prec_math_log(x);
}

// Take the natural logarithm of x.
float logf(float x)
{
    return __prec_math_logf(x);
}

// Take the natural logarithm of x.
long double logl(long double x)
{
    return __prec_math_logl(x);
}

// Take the base-10 logarithm of x.
double log10(double x)
{
    return __prec_math_log10(x);
}

// Take the base-10 logarithm of x.
float log10f(float x)
{
    return __prec_math_log10f(x);
}

// Take the base-10 logarithm of x.
long double log10l(long double x)
{
    return __prec_math_log10l(x);
}

// Take the natural logarithm of 1 plus x.
double log1p(double x)
{
    return __prec_math_log1p(x);
}

// Take the natural logarithm of 1 plus x.
float log1pf(float x)
{
    return __prec_math_log1pf(x);
}

// Take the natural logarithm of 1 plus x.
long double log1pl(long double x)
{
    return __prec_math_log1pl(x);
}

// Take the base-2 logarithm of x.
double log2(double x)
{
    return __prec_math_log2(x);
}

// Take the base-2 logarithm of x.
float log2f(float x)
{
    return __prec_math_log2f(x);
}

// Take the base-2 logarithm of x.
long double log2l(long double x)
{
    return __prec_math_log2l(x);
}

// Extract x's exponent as a floating value.
double logb(double x)
{
    return (double) logbl(x);
}

// Extract x's exponent as a floating value.
float logbf(float x)
{
    return (float) logbl(x);
}

// Extract x's exponent as a floating value.
long double logbl(long double x)
{
    int exp;

    if (isnan(x)) {
        return x + x;
    }
    if (isinf(x)) {
        return HUGE_VALL;
    }
    if (x == 0) {
        feraiseexcept(FE_DIVBYZERO);
        return -HUGE_VALL;
    }
    _Exact_SplitLDouble(x, &exp);
    return exp;
}

// Break value into its integral and fractional parts.
double modf(double value, double *iptr)
{
    long double whole;
    double fraction = (double) modfl(value, &whole);

    *iptr = (double) whole;
    return fraction;
}

// Break value into its integral and fractional parts.
float modff(float value, float *iptr)
{
    long double whole;
    float fraction = (float) modfl(value, &whole);

    *iptr = (float) whole;
    return fraction;
}

// Break value into its integral and fractional parts.
long double modfl(long double value, long double *iptr)
{
    *iptr = truncl(value);
    if (isinf(value)) {
        return copysignl(0, value);
    }
    return copysignl(value - *iptr, value);
}

// Scale x by FLT_RADIX to n.
double scalbn(double x, int n)
{
    double y = (double) _Math_ScaleLDouble(x, n);

    _Math_CheckScaled(x, y);
    return y;
}

// Scale x by FLT_RADIX to n.
float scalbnf(float x, int n)
{
    float y = (float) _Math_ScaleLDouble(x, n);

    _Math_CheckScaled(x, y);
    return y;
}

// Scale x by FLT_RADIX to n.
long double scalbnl(long double x, int n)
{
    long double y = _Math_ScaleLDouble(x, n);

    _Math_CheckScaled(x, y);
    return y;
}

// Scale x by FLT_RADIX to a long n.
double scalbln(double x, long int n)
{
    return scalbn(x, _Math_ClampExponent(n));
}

// Scale x by FLT_RADIX to a long n.
float scalblnf(float x, long int n)
{
    return scalbnf(x, _Math_ClampExponent(n));
}

// Scale x by FLT_RADIX to a long n.
long double scalblnl(long double x, long int n)
{
    return scalbnl(x, _Math_ClampExponent(n));
}

// Take the real cube root of x.
double cbrt(double x)
{
    return (double) _Math_TakeRoot(x, _EXACT_ROOT_CUBE, _EXACT_TYPE_DOUBLE);
}

// Take the real cube root of x.
float cbrtf(float x)
{
    return (float) _Math_TakeRoot(x, _EXACT_ROOT_CUBE, _EXACT_TYPE_FLOAT);
}

// Take the real cube root of x.
long double cbrtl(long double x)
{
    return _Math_TakeRoot(x, _EXACT_ROOT_CUBE, _EXACT_TYPE_LDOUBLE);
}

// Clear the sign of x.
double fabs(double x)
{
    ((unsigned char *) &x)[_MATH_DOUBLE_SIGN_BYTE] &= _MATH_SIGN_CLEAR;
    return x;
}

// Clear the sign of x.
float fabsf(float x)
{
    ((unsigned char *) &x)[_MATH_FLOAT_SIGN_BYTE] &= _MATH_SIGN_CLEAR;
    return x;
}

// Clear the sign of x.
long double fabsl(long double x)
{
    ((unsigned char *) &x)[_MATH_LDOUBLE_SIGN_BYTE] &= _MATH_SIGN_CLEAR;
    return x;
}

// Take the square root of the sum of the squares of x and y.
double hypot(double x, double y)
{
    return (double) _Math_TakeHypot(x, y, _EXACT_TYPE_DOUBLE);
}

// Take the square root of the sum of the squares of x and y.
float hypotf(float x, float y)
{
    return (float) _Math_TakeHypot(x, y, _EXACT_TYPE_FLOAT);
}

// Take the square root of the sum of the squares of x and y.
long double hypotl(long double x, long double y)
{
    return _Math_TakeHypot(x, y, _EXACT_TYPE_LDOUBLE);
}

// Raise x to the power y.
double pow(double x, double y)
{
    return __prec_math_pow(x, y);
}

// Raise x to the power y.
float powf(float x, float y)
{
    return __prec_math_powf(x, y);
}

// Raise x to the power y.
long double powl(long double x, long double y)
{
    return __prec_math_powl(x, y);
}

// Take the nonnegative square root of x.
double sqrt(double x)
{
    return (double) _Math_TakeRoot(x, _EXACT_ROOT_SQUARE, _EXACT_TYPE_DOUBLE);
}

// Take the nonnegative square root of x.
float sqrtf(float x)
{
    return (float) _Math_TakeRoot(x, _EXACT_ROOT_SQUARE, _EXACT_TYPE_FLOAT);
}

// Take the nonnegative square root of x.
long double sqrtl(long double x)
{
    return _Math_TakeRoot(x, _EXACT_ROOT_SQUARE, _EXACT_TYPE_LDOUBLE);
}

// Take the error function of x.
double erf(double x)
{
    return __prec_math_erf(x);
}

// Take the error function of x.
float erff(float x)
{
    return __prec_math_erff(x);
}

// Take the error function of x.
long double erfl(long double x)
{
    return __prec_math_erfl(x);
}

// Take the complementary error function of x.
double erfc(double x)
{
    return __prec_math_erfc(x);
}

// Take the complementary error function of x.
float erfcf(float x)
{
    return __prec_math_erfcf(x);
}

// Take the complementary error function of x.
long double erfcl(long double x)
{
    return __prec_math_erfcl(x);
}

// Take the natural logarithm of the magnitude of gamma of x.
double lgamma(double x)
{
    return __prec_math_lgamma(x);
}

// Take the natural logarithm of the magnitude of gamma of x.
float lgammaf(float x)
{
    return __prec_math_lgammaf(x);
}

// Take the natural logarithm of the magnitude of gamma of x.
long double lgammal(long double x)
{
    return __prec_math_lgammal(x);
}

// Take gamma of x.
double tgamma(double x)
{
    return __prec_math_tgamma(x);
}

// Take gamma of x.
float tgammaf(float x)
{
    return __prec_math_tgammaf(x);
}

// Take gamma of x.
long double tgammal(long double x)
{
    return __prec_math_tgammal(x);
}

// Round x up to an integer.
double ceil(double x)
{
    return (double) ceill(x);
}

// Round x up to an integer.
float ceilf(float x)
{
    return (float) ceill(x);
}

// Round x up to an integer.
long double ceill(long double x)
{
    long double whole = truncl(x);

    return isfinite(x) && whole < x ? whole + 1 : whole;
}

// Round x down to an integer.
double floor(double x)
{
    return (double) floorl(x);
}

// Round x down to an integer.
float floorf(float x)
{
    return (float) floorl(x);
}

// Round x down to an integer.
long double floorl(long double x)
{
    long double whole = truncl(x);

    return isfinite(x) && whole > x ? whole - 1 : whole;
}

// Round x to an integer in the current direction without inexact.
double nearbyint(double x)
{
    return (double) nearbyintl(x);
}

// Round x to an integer in the current direction without inexact.
float nearbyintf(float x)
{
    return (float) nearbyintl(x);
}

// Round x to an integer in the current direction without inexact.
long double nearbyintl(long double x)
{
    fenv_t env;
    long double rounded;

    if (isnan(x)) {
        return x + x;
    }
    feholdexcept(&env);
    rounded = rintl(x);
    fesetenv(&env);
    return rounded;
}

// Round x to an integer in the current direction.
double rint(double x)
{
    return (double) rintl(x);
}

// Round x to an integer in the current direction.
float rintf(float x)
{
    return (float) rintl(x);
}

// Round x to an integer in the current direction.
long double rintl(long double x)
{
    long double shift = copysignl(_MATH_LDOUBLE_INTEGRAL, x);

    if (! isfinite(x)) {
        return x + x;
    }
    if (fabsl(x) >= _MATH_LDOUBLE_INTEGRAL) {
        return x;
    }
    return copysignl((x + shift) - shift, x);
}

// Round x to a long in the current direction.
long int lrint(double x)
{
    return lrintl(x);
}

// Round x to a long in the current direction.
long int lrintf(float x)
{
    return lrintl(x);
}

// Round x to a long in the current direction.
long int lrintl(long double x)
{
    return (long int) rintl(x);
}

// Round x to a long long in the current direction.
long long int llrint(double x)
{
    return llrintl(x);
}

// Round x to a long long in the current direction.
long long int llrintf(float x)
{
    return llrintl(x);
}

// Round x to a long long in the current direction.
long long int llrintl(long double x)
{
    return (long long int) rintl(x);
}

// Round x to the nearest integer, halfway away from zero.
double round(double x)
{
    return (double) roundl(x);
}

// Round x to the nearest integer, halfway away from zero.
float roundf(float x)
{
    return (float) roundl(x);
}

// Round x to the nearest integer, halfway away from zero.
long double roundl(long double x)
{
    long double whole = truncl(x);

    if (isfinite(x) && fabsl(x - whole) >= _MATH_HALF) {
        return whole + copysignl(1, x);
    }
    return whole;
}

// Round x to the nearest long, halfway away from zero.
long int lround(double x)
{
    return lroundl(x);
}

// Round x to the nearest long, halfway away from zero.
long int lroundf(float x)
{
    return lroundl(x);
}

// Round x to the nearest long, halfway away from zero.
long int lroundl(long double x)
{
    return (long int) roundl(x);
}

// Round x to the nearest long long, halfway away from zero.
long long int llround(double x)
{
    return llroundl(x);
}

// Round x to the nearest long long, halfway away from zero.
long long int llroundf(float x)
{
    return llroundl(x);
}

// Round x to the nearest long long, halfway away from zero.
long long int llroundl(long double x)
{
    return (long long int) roundl(x);
}

// Round x toward zero to an integer.
double trunc(double x)
{
    return (double) truncl(x);
}

// Round x toward zero to an integer.
float truncf(float x)
{
    return (float) truncl(x);
}

// Round x toward zero to an integer.
long double truncl(long double x)
{
    int exp;
    uint64_t sig;

    if (! isfinite(x) || x == 0) {
        return x + x;
    }
    sig = _Exact_SplitLDouble(x, &exp);
    if (exp >= _EXACT_LDOUBLE_INT_SHIFT) {
        return x;
    }
    if (exp < 0) {
        return copysignl(0, x);
    }
    return copysignl(_Exact_JoinLDouble(exp, sig & UINT64_MAX << (_EXACT_LDOUBLE_INT_SHIFT - exp)), x);
}

// Take the remainder of x by y toward zero.
double fmod(double x, double y)
{
    return (double) fmodl(x, y);
}

// Take the remainder of x by y toward zero.
float fmodf(float x, float y)
{
    return (float) fmodl(x, y);
}

// Take the remainder of x by y toward zero.
long double fmodl(long double x, long double y)
{
    uint32_t q;

    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (isinf(x) || y == 0) {
        return _Math_RaiseDomain(x, y);
    }
    if (isinf(y) || x == 0) {
        return x;
    }
    return _Math_TakeRemainder(x, y, _MATH_QUOTIENT_TRUNCATED, &q);
}

// Take IEC 60559's remainder of x by y.
double remainder(double x, double y)
{
    return (double) remainderl(x, y);
}

// Take IEC 60559's remainder of x by y.
float remainderf(float x, float y)
{
    return (float) remainderl(x, y);
}

// Take IEC 60559's remainder of x by y.
long double remainderl(long double x, long double y)
{
    int quo;

    return remquol(x, y, &quo);
}

// Take IEC 60559's remainder of x by y and the quotient's low bits.
double remquo(double x, double y, int *quo)
{
    return (double) remquol(x, y, quo);
}

// Take IEC 60559's remainder of x by y and the quotient's low bits.
float remquof(float x, float y, int *quo)
{
    return (float) remquol(x, y, quo);
}

// Take IEC 60559's remainder of x by y and the quotient's low bits.
long double remquol(long double x, long double y, int *quo)
{
    uint32_t q;
    long double rest;

    *quo = 0;
    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (isinf(x) || y == 0) {
        return _Math_RaiseDomain(x, y);
    }
    if (isinf(y) || x == 0) {
        return x;
    }
    rest = _Math_TakeRemainder(x, y, _MATH_QUOTIENT_NEAREST, &q);
    q &= _MATH_QUOTIENT_MASK;
    *quo = signbit(x) == signbit(y) ? (int) q : -(int) q;
    return rest;
}

// Give x the sign of y.
double copysign(double x, double y)
{
    ((unsigned char *) &x)[_MATH_DOUBLE_SIGN_BYTE] &= _MATH_SIGN_CLEAR;
    ((unsigned char *) &x)[_MATH_DOUBLE_SIGN_BYTE] |= ((unsigned char *) &y)[_MATH_DOUBLE_SIGN_BYTE] & _MATH_SIGN_BIT;
    return x;
}

// Give x the sign of y.
float copysignf(float x, float y)
{
    ((unsigned char *) &x)[_MATH_FLOAT_SIGN_BYTE] &= _MATH_SIGN_CLEAR;
    ((unsigned char *) &x)[_MATH_FLOAT_SIGN_BYTE] |= ((unsigned char *) &y)[_MATH_FLOAT_SIGN_BYTE] & _MATH_SIGN_BIT;
    return x;
}

// Give x the sign of y.
long double copysignl(long double x, long double y)
{
    ((unsigned char *) &x)[_MATH_LDOUBLE_SIGN_BYTE] &= _MATH_SIGN_CLEAR;
    ((unsigned char *) &x)[_MATH_LDOUBLE_SIGN_BYTE] |= ((unsigned char *) &y)[_MATH_LDOUBLE_SIGN_BYTE] & _MATH_SIGN_BIT;
    return x;
}

// Make a quiet NaN of tagp's payload.
double nan(const char *tagp)
{
    double x;
    uint64_t bits = _MATH_DOUBLE_QUIET | (_Math_ReadPayload(tagp) & _MATH_DOUBLE_FRAC_MASK);

    memcpy(&x, &bits, sizeof(x));
    return x;
}

// Make a quiet NaN of tagp's payload.
float nanf(const char *tagp)
{
    float x;
    uint32_t bits = _MATH_FLOAT_QUIET | (uint32_t) (_Math_ReadPayload(tagp) & _MATH_FLOAT_FRAC_MASK);

    memcpy(&x, &bits, sizeof(x));
    return x;
}

// Make a quiet NaN of tagp's payload.
long double nanl(const char *tagp)
{
    return _Exact_MakeLDouble(_EXACT_LDOUBLE_EXP_MAX, _MATH_LDOUBLE_QUIET | (_Math_ReadPayload(tagp) & _EXACT_LDOUBLE_FRAC_MASK));
}

// Step x to the next value toward y.
double nextafter(double x, double y)
{
    return nexttoward(x, y);
}

// Step x to the next value toward y.
float nextafterf(float x, float y)
{
    return nexttowardf(x, y);
}

// Step x to the next value toward y.
long double nextafterl(long double x, long double y)
{
    return nexttowardl(x, y);
}

// Step x to the next value toward a long double y.
double nexttoward(double x, long double y)
{
    double next;
    uint64_t bits;

    if (isnan(x) || isnan(y)) {
        return (double) (x + y);
    }
    if (x == y) {
        return (double) y;
    }
    if (x == 0) {
        next = y < 0 ? -_MATH_DOUBLE_TINY : _MATH_DOUBLE_TINY;
    } else {
        memcpy(&bits, &x, sizeof(bits));
        if ((x < y) == (x > 0)) {
            bits++;
        } else {
            bits--;
        }
        memcpy(&next, &bits, sizeof(next));
    }
    _Math_CheckNext(x, fpclassify(next));
    return next;
}

// Step x to the next value toward a long double y.
float nexttowardf(float x, long double y)
{
    float next;
    uint32_t bits;

    if (isnan(x) || isnan(y)) {
        return (float) (x + y);
    }
    if (x == y) {
        return (float) y;
    }
    if (x == 0) {
        next = y < 0 ? -_MATH_FLOAT_TINY : _MATH_FLOAT_TINY;
    } else {
        memcpy(&bits, &x, sizeof(bits));
        if ((x < y) == (x > 0)) {
            bits++;
        } else {
            bits--;
        }
        memcpy(&next, &bits, sizeof(next));
    }
    _Math_CheckNext(x, fpclassify(next));
    return next;
}

// Step x to the next value toward a long double y.
long double nexttowardl(long double x, long double y)
{
    long double next;

    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (x == y) {
        return y;
    }
    if (x == 0) {
        next = y < 0 ? -_MATH_LDOUBLE_TINY : _MATH_LDOUBLE_TINY;
    } else {
        next = _Exact_StepLDouble(x, (x < y) == (x > 0) ? _EXACT_STEP_UP : _EXACT_STEP_DOWN);
    }
    _Math_CheckNext(x, fpclassify(next));
    return next;
}

// Take the positive difference of x and y.
double fdim(double x, double y)
{
    double diff;

    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (x <= y) {
        return 0;
    }
    diff = x - y;
    if (isinf(diff) && isfinite(x) && isfinite(y)) {
        errno = ERANGE;
    }
    return diff;
}

// Take the positive difference of x and y.
float fdimf(float x, float y)
{
    float diff;

    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (x <= y) {
        return 0;
    }
    diff = x - y;
    if (isinf(diff) && isfinite(x) && isfinite(y)) {
        errno = ERANGE;
    }
    return diff;
}

// Take the positive difference of x and y.
long double fdiml(long double x, long double y)
{
    long double diff;

    if (isnan(x) || isnan(y)) {
        return x + y;
    }
    if (x <= y) {
        return 0;
    }
    diff = x - y;
    if (isinf(diff) && isfinite(x) && isfinite(y)) {
        errno = ERANGE;
    }
    return diff;
}

// Take the greater of x and y.
double fmax(double x, double y)
{
    if (isnan(x) || isnan(y)) {
        return isnan(x) ? y : x;
    }
    if (x == y) {
        return signbit(x) != 0 ? y : x;
    }
    return x > y ? x : y;
}

// Take the greater of x and y.
float fmaxf(float x, float y)
{
    if (isnan(x) || isnan(y)) {
        return isnan(x) ? y : x;
    }
    if (x == y) {
        return signbit(x) != 0 ? y : x;
    }
    return x > y ? x : y;
}

// Take the greater of x and y.
long double fmaxl(long double x, long double y)
{
    if (isnan(x) || isnan(y)) {
        return isnan(x) ? y : x;
    }
    if (x == y) {
        return signbit(x) != 0 ? y : x;
    }
    return x > y ? x : y;
}

// Take the lesser of x and y.
double fmin(double x, double y)
{
    if (isnan(x) || isnan(y)) {
        return isnan(x) ? y : x;
    }
    if (x == y) {
        return signbit(x) != 0 ? x : y;
    }
    return x < y ? x : y;
}

// Take the lesser of x and y.
float fminf(float x, float y)
{
    if (isnan(x) || isnan(y)) {
        return isnan(x) ? y : x;
    }
    if (x == y) {
        return signbit(x) != 0 ? x : y;
    }
    return x < y ? x : y;
}

// Take the lesser of x and y.
long double fminl(long double x, long double y)
{
    if (isnan(x) || isnan(y)) {
        return isnan(x) ? y : x;
    }
    if (x == y) {
        return signbit(x) != 0 ? x : y;
    }
    return x < y ? x : y;
}

// Take x times y plus z, rounded once.
double fma(double x, double y, double z)
{
    return (double) _Math_TakeFma(x, y, z, _EXACT_TYPE_DOUBLE);
}

// Take x times y plus z, rounded once.
float fmaf(float x, float y, float z)
{
    return (float) _Math_TakeFma(x, y, z, _EXACT_TYPE_FLOAT);
}

// Take x times y plus z, rounded once.
long double fmal(long double x, long double y, long double z)
{
    return _Math_TakeFma(x, y, z, _EXACT_TYPE_LDOUBLE);
}
