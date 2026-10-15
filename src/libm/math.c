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
#include <_basic_math.h>

// Take the arc cosine of x.
double acos(double x)
{
    return __basic_math_acos(x);
}

// Take the arc cosine of x.
float acosf(float x)
{
    return __basic_math_acosf(x);
}

// Take the arc cosine of x.
long double acosl(long double x)
{
    return __basic_math_acosl(x);
}

// Take the arc sine of x.
double asin(double x)
{
    return __basic_math_asin(x);
}

// Take the arc sine of x.
float asinf(float x)
{
    return __basic_math_asinf(x);
}

// Take the arc sine of x.
long double asinl(long double x)
{
    return __basic_math_asinl(x);
}

// Take the arc tangent of x.
double atan(double x)
{
    return __basic_math_atan(x);
}

// Take the arc tangent of x.
float atanf(float x)
{
    return __basic_math_atanf(x);
}

// Take the arc tangent of x.
long double atanl(long double x)
{
    return __basic_math_atanl(x);
}

// Take the arc tangent of y over x by both signs.
double atan2(double y, double x)
{
    return __basic_math_atan2(y, x);
}

// Take the arc tangent of y over x by both signs.
float atan2f(float y, float x)
{
    return __basic_math_atan2f(y, x);
}

// Take the arc tangent of y over x by both signs.
long double atan2l(long double y, long double x)
{
    return __basic_math_atan2l(y, x);
}

// Take the cosine of x.
double cos(double x)
{
    return __basic_math_cos(x);
}

// Take the cosine of x.
float cosf(float x)
{
    return __basic_math_cosf(x);
}

// Take the cosine of x.
long double cosl(long double x)
{
    return __basic_math_cosl(x);
}

// Take the sine of x.
double sin(double x)
{
    return __basic_math_sin(x);
}

// Take the sine of x.
float sinf(float x)
{
    return __basic_math_sinf(x);
}

// Take the sine of x.
long double sinl(long double x)
{
    return __basic_math_sinl(x);
}

// Take the tangent of x.
double tan(double x)
{
    return __basic_math_tan(x);
}

// Take the tangent of x.
float tanf(float x)
{
    return __basic_math_tanf(x);
}

// Take the tangent of x.
long double tanl(long double x)
{
    return __basic_math_tanl(x);
}

// Take the inverse hyperbolic cosine of x.
double acosh(double x)
{
    return __basic_math_acosh(x);
}

// Take the inverse hyperbolic cosine of x.
float acoshf(float x)
{
    return __basic_math_acoshf(x);
}

// Take the inverse hyperbolic cosine of x.
long double acoshl(long double x)
{
    return __basic_math_acoshl(x);
}

// Take the inverse hyperbolic sine of x.
double asinh(double x)
{
    return __basic_math_asinh(x);
}

// Take the inverse hyperbolic sine of x.
float asinhf(float x)
{
    return __basic_math_asinhf(x);
}

// Take the inverse hyperbolic sine of x.
long double asinhl(long double x)
{
    return __basic_math_asinhl(x);
}

// Take the inverse hyperbolic tangent of x.
double atanh(double x)
{
    return __basic_math_atanh(x);
}

// Take the inverse hyperbolic tangent of x.
float atanhf(float x)
{
    return __basic_math_atanhf(x);
}

// Take the inverse hyperbolic tangent of x.
long double atanhl(long double x)
{
    return __basic_math_atanhl(x);
}

// Take the hyperbolic cosine of x.
double cosh(double x)
{
    return __basic_math_cosh(x);
}

// Take the hyperbolic cosine of x.
float coshf(float x)
{
    return __basic_math_coshf(x);
}

// Take the hyperbolic cosine of x.
long double coshl(long double x)
{
    return __basic_math_coshl(x);
}

// Take the hyperbolic sine of x.
double sinh(double x)
{
    return __basic_math_sinh(x);
}

// Take the hyperbolic sine of x.
float sinhf(float x)
{
    return __basic_math_sinhf(x);
}

// Take the hyperbolic sine of x.
long double sinhl(long double x)
{
    return __basic_math_sinhl(x);
}

// Take the hyperbolic tangent of x.
double tanh(double x)
{
    return __basic_math_tanh(x);
}

// Take the hyperbolic tangent of x.
float tanhf(float x)
{
    return __basic_math_tanhf(x);
}

// Take the hyperbolic tangent of x.
long double tanhl(long double x)
{
    return __basic_math_tanhl(x);
}

// Take e to x.
double exp(double x)
{
    return __basic_math_exp(x);
}

// Take e to x.
float expf(float x)
{
    return __basic_math_expf(x);
}

// Take e to x.
long double expl(long double x)
{
    return __basic_math_expl(x);
}

// Take 2 to x.
double exp2(double x)
{
    return __basic_math_exp2(x);
}

// Take 2 to x.
float exp2f(float x)
{
    return __basic_math_exp2f(x);
}

// Take 2 to x.
long double exp2l(long double x)
{
    return __basic_math_exp2l(x);
}

// Take e to x, less 1.
double expm1(double x)
{
    return __basic_math_expm1(x);
}

// Take e to x, less 1.
float expm1f(float x)
{
    return __basic_math_expm1f(x);
}

// Take e to x, less 1.
long double expm1l(long double x)
{
    return __basic_math_expm1l(x);
}

// Break value into a fraction and a power of 2.
double frexp(double value, int *exp)
{
    return __basic_math_frexp(value, exp);
}

// Break value into a fraction and a power of 2.
float frexpf(float value, int *exp)
{
    return __basic_math_frexpf(value, exp);
}

// Break value into a fraction and a power of 2.
long double frexpl(long double value, int *exp)
{
    return __basic_math_frexpl(value, exp);
}

// Extract x's exponent as an int.
int ilogb(double x)
{
    return __basic_math_ilogb(x);
}

// Extract x's exponent as an int.
int ilogbf(float x)
{
    return __basic_math_ilogbf(x);
}

// Extract x's exponent as an int.
int ilogbl(long double x)
{
    return __basic_math_ilogbl(x);
}

// Multiply x by 2 to exp.
double ldexp(double x, int exp)
{
    return __basic_math_ldexp(x, exp);
}

// Multiply x by 2 to exp.
float ldexpf(float x, int exp)
{
    return __basic_math_ldexpf(x, exp);
}

// Multiply x by 2 to exp.
long double ldexpl(long double x, int exp)
{
    return __basic_math_ldexpl(x, exp);
}

// Take the natural logarithm of x.
double log(double x)
{
    return __basic_math_log(x);
}

// Take the natural logarithm of x.
float logf(float x)
{
    return __basic_math_logf(x);
}

// Take the natural logarithm of x.
long double logl(long double x)
{
    return __basic_math_logl(x);
}

// Take the base-10 logarithm of x.
double log10(double x)
{
    return __basic_math_log10(x);
}

// Take the base-10 logarithm of x.
float log10f(float x)
{
    return __basic_math_log10f(x);
}

// Take the base-10 logarithm of x.
long double log10l(long double x)
{
    return __basic_math_log10l(x);
}

// Take the natural logarithm of 1 plus x.
double log1p(double x)
{
    return __basic_math_log1p(x);
}

// Take the natural logarithm of 1 plus x.
float log1pf(float x)
{
    return __basic_math_log1pf(x);
}

// Take the natural logarithm of 1 plus x.
long double log1pl(long double x)
{
    return __basic_math_log1pl(x);
}

// Take the base-2 logarithm of x.
double log2(double x)
{
    return __basic_math_log2(x);
}

// Take the base-2 logarithm of x.
float log2f(float x)
{
    return __basic_math_log2f(x);
}

// Take the base-2 logarithm of x.
long double log2l(long double x)
{
    return __basic_math_log2l(x);
}

// Extract x's exponent as a floating value.
double logb(double x)
{
    return __basic_math_logb(x);
}

// Extract x's exponent as a floating value.
float logbf(float x)
{
    return __basic_math_logbf(x);
}

// Extract x's exponent as a floating value.
long double logbl(long double x)
{
    return __basic_math_logbl(x);
}

// Break value into its integral and fractional parts.
double modf(double value, double *iptr)
{
    return __basic_math_modf(value, iptr);
}

// Break value into its integral and fractional parts.
float modff(float value, float *iptr)
{
    return __basic_math_modff(value, iptr);
}

// Break value into its integral and fractional parts.
long double modfl(long double value, long double *iptr)
{
    return __basic_math_modfl(value, iptr);
}

// Scale x by FLT_RADIX to n.
double scalbn(double x, int n)
{
    return __basic_math_scalbn(x, n);
}

// Scale x by FLT_RADIX to n.
float scalbnf(float x, int n)
{
    return __basic_math_scalbnf(x, n);
}

// Scale x by FLT_RADIX to n.
long double scalbnl(long double x, int n)
{
    return __basic_math_scalbnl(x, n);
}

// Scale x by FLT_RADIX to a long n.
double scalbln(double x, long int n)
{
    return __basic_math_scalbln(x, n);
}

// Scale x by FLT_RADIX to a long n.
float scalblnf(float x, long int n)
{
    return __basic_math_scalblnf(x, n);
}

// Scale x by FLT_RADIX to a long n.
long double scalblnl(long double x, long int n)
{
    return __basic_math_scalblnl(x, n);
}

// Take the real cube root of x.
double cbrt(double x)
{
    return __basic_math_cbrt(x);
}

// Take the real cube root of x.
float cbrtf(float x)
{
    return __basic_math_cbrtf(x);
}

// Take the real cube root of x.
long double cbrtl(long double x)
{
    return __basic_math_cbrtl(x);
}

// Clear the sign of x.
double fabs(double x)
{
    return __basic_math_fabs(x);
}

// Clear the sign of x.
float fabsf(float x)
{
    return __basic_math_fabsf(x);
}

// Clear the sign of x.
long double fabsl(long double x)
{
    return __basic_math_fabsl(x);
}

// Take the square root of the sum of the squares of x and y.
double hypot(double x, double y)
{
    return __basic_math_hypot(x, y);
}

// Take the square root of the sum of the squares of x and y.
float hypotf(float x, float y)
{
    return __basic_math_hypotf(x, y);
}

// Take the square root of the sum of the squares of x and y.
long double hypotl(long double x, long double y)
{
    return __basic_math_hypotl(x, y);
}

// Raise x to the power y.
double pow(double x, double y)
{
    return __basic_math_pow(x, y);
}

// Raise x to the power y.
float powf(float x, float y)
{
    return __basic_math_powf(x, y);
}

// Raise x to the power y.
long double powl(long double x, long double y)
{
    return __basic_math_powl(x, y);
}

// Take the nonnegative square root of x.
double sqrt(double x)
{
    return __basic_math_sqrt(x);
}

// Take the nonnegative square root of x.
float sqrtf(float x)
{
    return __basic_math_sqrtf(x);
}

// Take the nonnegative square root of x.
long double sqrtl(long double x)
{
    return __basic_math_sqrtl(x);
}

// Take the error function of x.
double erf(double x)
{
    return __basic_math_erf(x);
}

// Take the error function of x.
float erff(float x)
{
    return __basic_math_erff(x);
}

// Take the error function of x.
long double erfl(long double x)
{
    return __basic_math_erfl(x);
}

// Take the complementary error function of x.
double erfc(double x)
{
    return __basic_math_erfc(x);
}

// Take the complementary error function of x.
float erfcf(float x)
{
    return __basic_math_erfcf(x);
}

// Take the complementary error function of x.
long double erfcl(long double x)
{
    return __basic_math_erfcl(x);
}

// Take the natural logarithm of the magnitude of gamma of x.
double lgamma(double x)
{
    return __basic_math_lgamma(x);
}

// Take the natural logarithm of the magnitude of gamma of x.
float lgammaf(float x)
{
    return __basic_math_lgammaf(x);
}

// Take the natural logarithm of the magnitude of gamma of x.
long double lgammal(long double x)
{
    return __basic_math_lgammal(x);
}

// Take gamma of x.
double tgamma(double x)
{
    return __basic_math_tgamma(x);
}

// Take gamma of x.
float tgammaf(float x)
{
    return __basic_math_tgammaf(x);
}

// Take gamma of x.
long double tgammal(long double x)
{
    return __basic_math_tgammal(x);
}

// Round x up to an integer.
double ceil(double x)
{
    return __basic_math_ceil(x);
}

// Round x up to an integer.
float ceilf(float x)
{
    return __basic_math_ceilf(x);
}

// Round x up to an integer.
long double ceill(long double x)
{
    return __basic_math_ceill(x);
}

// Round x down to an integer.
double floor(double x)
{
    return __basic_math_floor(x);
}

// Round x down to an integer.
float floorf(float x)
{
    return __basic_math_floorf(x);
}

// Round x down to an integer.
long double floorl(long double x)
{
    return __basic_math_floorl(x);
}

// Round x to an integer in the current direction without inexact.
double nearbyint(double x)
{
    return __basic_math_nearbyint(x);
}

// Round x to an integer in the current direction without inexact.
float nearbyintf(float x)
{
    return __basic_math_nearbyintf(x);
}

// Round x to an integer in the current direction without inexact.
long double nearbyintl(long double x)
{
    return __basic_math_nearbyintl(x);
}

// Round x to an integer in the current direction.
double rint(double x)
{
    return __basic_math_rint(x);
}

// Round x to an integer in the current direction.
float rintf(float x)
{
    return __basic_math_rintf(x);
}

// Round x to an integer in the current direction.
long double rintl(long double x)
{
    return __basic_math_rintl(x);
}

// Round x to a long in the current direction.
long int lrint(double x)
{
    return __basic_math_lrint(x);
}

// Round x to a long in the current direction.
long int lrintf(float x)
{
    return __basic_math_lrintf(x);
}

// Round x to a long in the current direction.
long int lrintl(long double x)
{
    return __basic_math_lrintl(x);
}

// Round x to a long long in the current direction.
long long int llrint(double x)
{
    return __basic_math_llrint(x);
}

// Round x to a long long in the current direction.
long long int llrintf(float x)
{
    return __basic_math_llrintf(x);
}

// Round x to a long long in the current direction.
long long int llrintl(long double x)
{
    return __basic_math_llrintl(x);
}

// Round x to the nearest integer, halfway away from zero.
double round(double x)
{
    return __basic_math_round(x);
}

// Round x to the nearest integer, halfway away from zero.
float roundf(float x)
{
    return __basic_math_roundf(x);
}

// Round x to the nearest integer, halfway away from zero.
long double roundl(long double x)
{
    return __basic_math_roundl(x);
}

// Round x to the nearest long, halfway away from zero.
long int lround(double x)
{
    return __basic_math_lround(x);
}

// Round x to the nearest long, halfway away from zero.
long int lroundf(float x)
{
    return __basic_math_lroundf(x);
}

// Round x to the nearest long, halfway away from zero.
long int lroundl(long double x)
{
    return __basic_math_lroundl(x);
}

// Round x to the nearest long long, halfway away from zero.
long long int llround(double x)
{
    return __basic_math_llround(x);
}

// Round x to the nearest long long, halfway away from zero.
long long int llroundf(float x)
{
    return __basic_math_llroundf(x);
}

// Round x to the nearest long long, halfway away from zero.
long long int llroundl(long double x)
{
    return __basic_math_llroundl(x);
}

// Round x toward zero to an integer.
double trunc(double x)
{
    return __basic_math_trunc(x);
}

// Round x toward zero to an integer.
float truncf(float x)
{
    return __basic_math_truncf(x);
}

// Round x toward zero to an integer.
long double truncl(long double x)
{
    return __basic_math_truncl(x);
}

// Take the remainder of x by y toward zero.
double fmod(double x, double y)
{
    return __basic_math_fmod(x, y);
}

// Take the remainder of x by y toward zero.
float fmodf(float x, float y)
{
    return __basic_math_fmodf(x, y);
}

// Take the remainder of x by y toward zero.
long double fmodl(long double x, long double y)
{
    return __basic_math_fmodl(x, y);
}

// Take IEC 60559's remainder of x by y.
double remainder(double x, double y)
{
    return __basic_math_remainder(x, y);
}

// Take IEC 60559's remainder of x by y.
float remainderf(float x, float y)
{
    return __basic_math_remainderf(x, y);
}

// Take IEC 60559's remainder of x by y.
long double remainderl(long double x, long double y)
{
    return __basic_math_remainderl(x, y);
}

// Take IEC 60559's remainder of x by y and the quotient's low bits.
double remquo(double x, double y, int *quo)
{
    return __basic_math_remquo(x, y, quo);
}

// Take IEC 60559's remainder of x by y and the quotient's low bits.
float remquof(float x, float y, int *quo)
{
    return __basic_math_remquof(x, y, quo);
}

// Take IEC 60559's remainder of x by y and the quotient's low bits.
long double remquol(long double x, long double y, int *quo)
{
    return __basic_math_remquol(x, y, quo);
}

// Give x the sign of y.
double copysign(double x, double y)
{
    return __basic_math_copysign(x, y);
}

// Give x the sign of y.
float copysignf(float x, float y)
{
    return __basic_math_copysignf(x, y);
}

// Give x the sign of y.
long double copysignl(long double x, long double y)
{
    return __basic_math_copysignl(x, y);
}

// Make a quiet NaN of tagp's payload.
double nan(const char *tagp)
{
    return __basic_math_nan(tagp);
}

// Make a quiet NaN of tagp's payload.
float nanf(const char *tagp)
{
    return __basic_math_nanf(tagp);
}

// Make a quiet NaN of tagp's payload.
long double nanl(const char *tagp)
{
    return __basic_math_nanl(tagp);
}

// Step x to the next value toward y.
double nextafter(double x, double y)
{
    return __basic_math_nextafter(x, y);
}

// Step x to the next value toward y.
float nextafterf(float x, float y)
{
    return __basic_math_nextafterf(x, y);
}

// Step x to the next value toward y.
long double nextafterl(long double x, long double y)
{
    return __basic_math_nextafterl(x, y);
}

// Step x to the next value toward a long double y.
double nexttoward(double x, long double y)
{
    return __basic_math_nexttoward(x, y);
}

// Step x to the next value toward a long double y.
float nexttowardf(float x, long double y)
{
    return __basic_math_nexttowardf(x, y);
}

// Step x to the next value toward a long double y.
long double nexttowardl(long double x, long double y)
{
    return __basic_math_nexttowardl(x, y);
}

// Take the positive difference of x and y.
double fdim(double x, double y)
{
    return __basic_math_fdim(x, y);
}

// Take the positive difference of x and y.
float fdimf(float x, float y)
{
    return __basic_math_fdimf(x, y);
}

// Take the positive difference of x and y.
long double fdiml(long double x, long double y)
{
    return __basic_math_fdiml(x, y);
}

// Take the greater of x and y.
double fmax(double x, double y)
{
    return __basic_math_fmax(x, y);
}

// Take the greater of x and y.
float fmaxf(float x, float y)
{
    return __basic_math_fmaxf(x, y);
}

// Take the greater of x and y.
long double fmaxl(long double x, long double y)
{
    return __basic_math_fmaxl(x, y);
}

// Take the lesser of x and y.
double fmin(double x, double y)
{
    return __basic_math_fmin(x, y);
}

// Take the lesser of x and y.
float fminf(float x, float y)
{
    return __basic_math_fminf(x, y);
}

// Take the lesser of x and y.
long double fminl(long double x, long double y)
{
    return __basic_math_fminl(x, y);
}

// Take x times y plus z, rounded once.
double fma(double x, double y, double z)
{
    return __basic_math_fma(x, y, z);
}

// Take x times y plus z, rounded once.
float fmaf(float x, float y, float z)
{
    return __basic_math_fmaf(x, y, z);
}

// Take x times y plus z, rounded once.
long double fmal(long double x, long double y, long double z)
{
    return __basic_math_fmal(x, y, z);
}
