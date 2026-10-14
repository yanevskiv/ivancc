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
    memcpy(&sig, (unsigned char *) &x + _MATH_LDOUBLE_SIGNIFICAND, sizeof(sig));
    memcpy(&top, (unsigned char *) &x + _MATH_LDOUBLE_TOP, sizeof(top));
    uint32_t exp = top & _MATH_LDOUBLE_EXP_MAX;

    if (exp == 0) {
        return sig >> _MATH_LDOUBLE_INT_SHIFT ? FP_NORMAL : _Math_ClassifyFields(exp, _MATH_LDOUBLE_EXP_MAX, sig);
    }
    if (! (sig >> _MATH_LDOUBLE_INT_SHIFT)) {
        return FP_NAN;
    }
    return _Math_ClassifyFields(exp, _MATH_LDOUBLE_EXP_MAX, sig & _MATH_LDOUBLE_FRAC_MASK);
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
