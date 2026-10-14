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
#include <_fp.h>
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

// Read a long double's significand.
unsigned long long _Math_ReadSignificand(long double x)
{
    uint64_t sig;

    memcpy(&sig, (unsigned char *) &x + _MATH_LDOUBLE_SIGNIFICAND, sizeof(sig));
    return sig;
}

// Read a long double's sign and biased exponent.
unsigned int _Math_ReadTop(long double x)
{
    uint16_t top;

    memcpy(&top, (unsigned char *) &x + _MATH_LDOUBLE_TOP, sizeof(top));
    return top;
}

// Make a long double of its sign and biased exponent and its significand.
long double _Math_MakeLDouble(unsigned int top, unsigned long long sig)
{
    long double x = 0.0L;
    uint64_t low = sig;
    uint16_t high = (uint16_t) top;

    memcpy((unsigned char *) &x + _MATH_LDOUBLE_SIGNIFICAND, &low, sizeof(low));
    memcpy((unsigned char *) &x + _MATH_LDOUBLE_TOP, &high, sizeof(high));
    return x;
}

// Split a long double into its normalized significand and its exponent.
unsigned long long _Math_SplitLDouble(long double x, int *exp)
{
    uint64_t sig = _Math_ReadSignificand(x);
    uint32_t biased = _Math_ReadTop(x) & _MATH_LDOUBLE_EXP_MAX;

    *exp = biased == 0 ? _MATH_LDOUBLE_EXP_LOW : (int) biased - _MATH_LDOUBLE_BIAS;
    for (; (sig & _MATH_LDOUBLE_INT_BIT) == 0; sig <<= 1) {
        (*exp)--;
    }
    return sig;
}

// Join a significand and an exponent into a long double.
long double _Math_JoinLDouble(int exp, unsigned long long sig)
{
    int shift;
    uint64_t bits = sig;

    if (bits == 0) {
        return 0.0L;
    }
    for (; (bits & _MATH_LDOUBLE_INT_BIT) == 0; bits <<= 1) {
        exp--;
    }
    if (exp >= _MATH_LDOUBLE_EXP_LOW) {
        return _Math_MakeLDouble((unsigned int) (exp + _MATH_LDOUBLE_BIAS), bits);
    }
    shift = _MATH_LDOUBLE_EXP_LOW - exp;
    return _Math_MakeLDouble(0, shift > _MATH_LDOUBLE_INT_SHIFT ? 0 : bits >> shift);
}

// Step a long double's magnitude to the next value up or down.
long double _Math_StepLDouble(long double x, _Math_Step step)
{
    uint32_t top = _Math_ReadTop(x);
    uint64_t sig = _Math_ReadSignificand(x);

    if (step == _MATH_STEP_UP && sig == UINT64_MAX) {
        sig = _MATH_LDOUBLE_INT_BIT;
        top++;
    } else if (step == _MATH_STEP_UP) {
        sig++;
    } else if ((top & _MATH_LDOUBLE_EXP_MAX) != 0 && sig == _MATH_LDOUBLE_INT_BIT) {
        top--;
        sig = (top & _MATH_LDOUBLE_EXP_MAX) == 0 ? _MATH_LDOUBLE_FRAC_MASK : UINT64_MAX;
    } else {
        sig--;
    }
    if ((top & _MATH_LDOUBLE_EXP_MAX) == 0 && sig >= _MATH_LDOUBLE_INT_BIT) {
        top++;
    }
    return _Math_MakeLDouble(top, sig);
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
    return x * _Math_MakeLDouble((unsigned int) (n + _MATH_LDOUBLE_BIAS), _MATH_LDOUBLE_INT_BIT);
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
    uint64_t rest = _Math_SplitLDouble(x, &ex);
    uint64_t div = _Math_SplitLDouble(y, &ey);

    for (int shift = ex - ey; shift >= 0; shift--) {
        if (carry != 0 || rest >= div) {
            rest -= div;
            q++;
        }
        if (shift > 0) {
            carry = rest & _MATH_LDOUBLE_INT_BIT;
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
    return copysignl(_Math_JoinLDouble(ex < ey ? ex : ey, rest), sign);
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

// Set w to a 64-bit value.
void _Math_WideSet(_Math_Wide *w, unsigned long long low)
{
    memset(w, 0, sizeof(*w));
    w->mw_limb[0] = (uint32_t) low;
    w->mw_limb[1] = (uint32_t) (low >> _MATH_LIMB_BITS);
}

// Read w's low 64 bits.
unsigned long long _Math_WideReadLow(const _Math_Wide *w)
{
    return (uint64_t) w->mw_limb[1] << _MATH_LIMB_BITS | w->mw_limb[0];
}

// Read w's bit at a place.
unsigned int _Math_WideReadBit(const _Math_Wide *w, int at)
{
    return w->mw_limb[at / _MATH_LIMB_BITS] >> (at % _MATH_LIMB_BITS) & _MATH_LOW_BIT;
}

// Find the place of w's top set bit.
int _Math_WideTop(const _Math_Wide *w)
{
    for (int i = _MATH_WIDE_LIMBS - 1; i >= 0; i--) {
        int bit = _MATH_LIMB_BITS - 1;

        if (w->mw_limb[i] == 0) {
            continue;
        }
        while ((w->mw_limb[i] >> bit) == 0) {
            bit--;
        }
        return i * _MATH_LIMB_BITS + bit;
    }
    return _MATH_WIDE_NONE;
}

// Shift w left by n bits, no set bit passing its top.
void _Math_WideShiftLeft(_Math_Wide *w, int n)
{
    int limbs = n / _MATH_LIMB_BITS;
    int bits = n % _MATH_LIMB_BITS;

    for (int i = _MATH_WIDE_LIMBS - 1; i >= 0; i--) {
        uint32_t high = i - limbs >= 0 ? w->mw_limb[i - limbs] : 0;
        uint32_t low = i - limbs - 1 >= 0 ? w->mw_limb[i - limbs - 1] : 0;

        w->mw_limb[i] = bits == 0 ? high : (uint32_t) (high << bits | low >> (_MATH_LIMB_BITS - bits));
    }
}

// Shift w right by n bits, returning whether a set bit fell off.
_Bool _Math_WideShiftRight(_Math_Wide *w, int n)
{
    _Bool lost = 0;
    int limbs = n / _MATH_LIMB_BITS;
    int bits = n % _MATH_LIMB_BITS;

    if (n >= _MATH_WIDE_BITS) {
        lost = _Math_WideTop(w) != _MATH_WIDE_NONE;
        memset(w, 0, sizeof(*w));
        return lost;
    }
    for (int i = 0; i < limbs; i++) {
        lost = lost || w->mw_limb[i] != 0;
    }
    lost = lost || (w->mw_limb[limbs] & ((_MATH_LOW_BIT << bits) - 1)) != 0;
    for (int i = 0; i < _MATH_WIDE_LIMBS; i++) {
        uint32_t low = i + limbs < _MATH_WIDE_LIMBS ? w->mw_limb[i + limbs] : 0;
        uint32_t high = i + limbs + 1 < _MATH_WIDE_LIMBS ? w->mw_limb[i + limbs + 1] : 0;

        w->mw_limb[i] = bits == 0 ? low : (uint32_t) (low >> bits | high << (_MATH_LIMB_BITS - bits));
    }
    return lost;
}

// Shift w left by n bits, or right by -n, returning whether a set bit fell off.
_Bool _Math_WideAlign(_Math_Wide *w, int n)
{
    if (n >= 0) {
        _Math_WideShiftLeft(w, n);
        return 0;
    }
    return _Math_WideShiftRight(w, -n);
}

// Compare a with b, negative, zero or positive.
int _Math_WideCompare(const _Math_Wide *a, const _Math_Wide *b)
{
    for (int i = _MATH_WIDE_LIMBS - 1; i >= 0; i--) {
        if (a->mw_limb[i] != b->mw_limb[i]) {
            return a->mw_limb[i] < b->mw_limb[i] ? -1 : 1;
        }
    }
    return 0;
}

// Add add to w.
void _Math_WideAdd(_Math_Wide *w, const _Math_Wide *add)
{
    uint64_t carry = 0;

    for (int i = 0; i < _MATH_WIDE_LIMBS; i++) {
        carry += (uint64_t) w->mw_limb[i] + add->mw_limb[i];
        w->mw_limb[i] = (uint32_t) carry;
        carry >>= _MATH_LIMB_BITS;
    }
}

// Subtract sub from w, sub not above it.
void _Math_WideSubtract(_Math_Wide *w, const _Math_Wide *sub)
{
    uint64_t borrow = 0;

    for (int i = 0; i < _MATH_WIDE_LIMBS; i++) {
        uint64_t diff = (uint64_t) w->mw_limb[i] - sub->mw_limb[i] - borrow;

        w->mw_limb[i] = (uint32_t) diff;
        borrow = diff >> _MATH_LIMB_BITS & _MATH_LOW_BIT;
    }
}

// Multiply a by b into to, the product below w's top.
void _Math_WideMultiply(_Math_Wide *to, const _Math_Wide *a, const _Math_Wide *b)
{
    _Math_Wide product;

    memset(&product, 0, sizeof(product));
    for (int i = 0; i < _MATH_WIDE_LIMBS; i++) {
        uint64_t carry = 0;

        for (int j = 0; i + j < _MATH_WIDE_LIMBS; j++) {
            carry += (uint64_t) a->mw_limb[i] * b->mw_limb[j] + product.mw_limb[i + j];
            product.mw_limb[i + j] = (uint32_t) carry;
            carry >>= _MATH_LIMB_BITS;
        }
    }
    *to = product;
}

// Raise base to the kth power into to.
void _Math_WidePower(_Math_Wide *to, const _Math_Wide *base, int k)
{
    _Math_WideSet(to, _MATH_LOW_BIT);
    for (int i = 0; i < k; i++) {
        _Math_WideMultiply(to, to, base);
    }
}

// Make the exact value of a finite nonzero long double.
void _Math_MakeExact(_Math_Exact *exact, long double x)
{
    int exp;

    _Math_WideSet(&exact->me_sig, _Math_SplitLDouble(x, &exp));
    exact->me_unit = exp - _MATH_LDOUBLE_INT_SHIFT;
    exact->me_negative = signbit(x) != 0;
    exact->me_sticky = 0;
}

// Multiply exact values a and b, neither sticky, into product.
void _Math_MultiplyExact(_Math_Exact *product, const _Math_Exact *a, const _Math_Exact *b)
{
    int unit = a->me_unit + b->me_unit;
    _Bool negative = a->me_negative != b->me_negative;

    _Math_WideMultiply(&product->me_sig, &a->me_sig, &b->me_sig);
    product->me_unit = unit;
    product->me_negative = negative;
    product->me_sticky = 0;
}

// Add exact values a and b, neither sticky, into sum, below a window sticky.
void _Math_AddExact(_Math_Exact *sum, const _Math_Exact *a, const _Math_Exact *b)
{
    int order;
    _Bool lost;
    _Math_Wide one;
    _Math_Wide wa = a->me_sig;
    _Math_Wide wb = b->me_sig;
    int top_a = a->me_unit + _Math_WideTop(&wa);
    int top_b = b->me_unit + _Math_WideTop(&wb);
    int base = (top_a > top_b ? top_a : top_b) + 1 - _MATH_SUM_WINDOW;

    lost = _Math_WideAlign(&wa, a->me_unit - base);
    lost = _Math_WideAlign(&wb, b->me_unit - base) || lost;
    order = _Math_WideCompare(&wa, &wb);
    sum->me_unit = base;
    sum->me_sticky = lost;
    if (a->me_negative == b->me_negative) {
        sum->me_sig = wa;
        _Math_WideAdd(&sum->me_sig, &wb);
        sum->me_negative = a->me_negative;
        return;
    }
    sum->me_sig = order >= 0 ? wa : wb;
    _Math_WideSubtract(&sum->me_sig, order >= 0 ? &wb : &wa);
    sum->me_negative = order >= 0 ? a->me_negative : b->me_negative;
    if (lost) {
        _Math_WideSet(&one, _MATH_LOW_BIT);
        _Math_WideSubtract(&sum->me_sig, &one);
    }
    if (order == 0) {
        sum->me_negative = fegetround() == FE_DOWNWARD;
    }
}

// Take the degree's root of an exact radicand into root, a round bit spare.
void _Math_RootExact(_Math_Exact *root, const _Math_Exact *radicand, _Math_Root degree)
{
    _Bool lost;
    _Math_Wide rest;
    _Math_Wide next;
    _Math_Wide trial;
    _Math_Wide power;
    int k = (int) degree;
    _Math_Wide n = radicand->me_sig;
    int shift = k * _MATH_ROOT_BITS - 1 - _Math_WideTop(&n);

    while ((radicand->me_unit - shift) % k != 0) {
        shift--;
    }
    lost = _Math_WideAlign(&n, shift);
    _Math_WideSet(&rest, 0);
    _Math_WideSet(&root->me_sig, 0);
    for (int i = _MATH_ROOT_BITS - 1; i >= 0; i--) {
        _Math_WideShiftLeft(&rest, k);
        for (int j = 0; j < k; j++) {
            rest.mw_limb[0] |= _Math_WideReadBit(&n, k * i + j) << j;
        }
        _Math_WideShiftLeft(&root->me_sig, 1);
        next = root->me_sig;
        next.mw_limb[0] |= _MATH_LOW_BIT;
        _Math_WidePower(&trial, &next, k);
        _Math_WidePower(&power, &root->me_sig, k);
        _Math_WideSubtract(&trial, &power);
        if (_Math_WideCompare(&rest, &trial) >= 0) {
            _Math_WideSubtract(&rest, &trial);
            root->me_sig.mw_limb[0] |= _MATH_LOW_BIT;
        }
    }
    root->me_unit = (radicand->me_unit - shift) / k;
    root->me_negative = radicand->me_negative;
    root->me_sticky = lost || radicand->me_sticky || _Math_WideTop(&rest) != _MATH_WIDE_NONE;
}

// Round exact's significand to its top keep bits in the current direction.
unsigned long long _Math_RoundBits(const _Math_Exact *exact, int keep, int *low, _Bool *inexact)
{
    _Bool up;
    uint64_t kept;
    _Bool half = 0;
    int mode = fegetround();
    _Math_Wide rest = exact->me_sig;
    _Bool below = exact->me_sticky;

    *low = _Math_WideTop(&rest) - keep + 1;
    if (*low <= 0) {
        _Math_WideShiftLeft(&rest, -*low);
    } else {
        below = _Math_WideShiftRight(&rest, *low - 1) || below;
        half = (rest.mw_limb[0] & _MATH_LOW_BIT) != 0;
        _Math_WideShiftRight(&rest, 1);
    }
    kept = _Math_WideReadLow(&rest);
    *inexact = half || below;
    if (mode == FE_TONEAREST) {
        up = half && (below || (kept & _MATH_LOW_BIT) != 0);
    } else if (mode == FE_UPWARD || mode == FE_DOWNWARD) {
        up = *inexact && exact->me_negative == (mode == FE_DOWNWARD);
    } else {
        up = 0;
    }
    if (! up) {
        return kept;
    }
    kept++;
    if (keep > 0 && (keep == LDBL_MANT_DIG ? kept == 0 : kept >> keep != 0)) {
        kept = _MATH_LDOUBLE_INT_BIT >> (LDBL_MANT_DIG - keep);
        (*low)++;
    }
    return kept;
}

// Round an exact value once to type in the current direction, with its flags.
long double _Math_RoundExact(const _Math_Exact *exact, _Math_Type type)
{
    int low;
    int min;
    int max;
    int keep;
    _Bool tiny;
    int digits;
    _Bool inexact;
    uint64_t kept;
    _Bool infinite;
    long double value;
    int mode = fegetround();
    int top = _Math_WideTop(&exact->me_sig);
    int exp = exact->me_unit + top;

    switch (type) {
        case _MATH_TYPE_FLOAT: {
            digits = FLT_MANT_DIG;
            min = FLT_MIN_EXP + _MATH_FRACTION_EXP;
            max = FLT_MAX_EXP + _MATH_FRACTION_EXP;
        } break;
        case _MATH_TYPE_DOUBLE: {
            digits = DBL_MANT_DIG;
            min = DBL_MIN_EXP + _MATH_FRACTION_EXP;
            max = DBL_MAX_EXP + _MATH_FRACTION_EXP;
        } break;
        default: {
            digits = LDBL_MANT_DIG;
            min = LDBL_MIN_EXP + _MATH_FRACTION_EXP;
            max = LDBL_MAX_EXP + _MATH_FRACTION_EXP;
        } break;
    }
    if (top == _MATH_WIDE_NONE) {
        return exact->me_negative ? -0.0L : 0.0L;
    }
    tiny = exp < min;
    if (exp == min - 1) {
        _Math_RoundBits(exact, digits, &low, &inexact);
        tiny = low == top - digits + 1;
    }
    keep = exp < min ? digits - (min - exp) : digits;
    kept = _Math_RoundBits(exact, keep, &low, &inexact);
    if (exp >= min && exact->me_unit + low + keep - 1 > max) {
        infinite = mode == FE_TONEAREST || (mode == FE_UPWARD && ! exact->me_negative) || (mode == FE_DOWNWARD && exact->me_negative);
        feraiseexcept(FE_OVERFLOW | FE_INEXACT);
        value = infinite ? HUGE_VALL : _Math_JoinLDouble(max, UINT64_MAX << (LDBL_MANT_DIG - digits));
        return exact->me_negative ? -value : value;
    }
    if (inexact) {
        feraiseexcept(tiny ? FE_UNDERFLOW | FE_INEXACT : FE_INEXACT);
    }
    value = _Math_JoinLDouble(exact->me_unit + low + _MATH_LDOUBLE_INT_SHIFT, kept);
    return exact->me_negative ? -value : value;
}

// Take x's square or cube root, correctly rounded to type.
long double _Math_TakeRoot(long double x, _Math_Root degree, _Math_Type type)
{
    _Math_Exact root;
    _Math_Exact radicand;

    if (isnan(x)) {
        return x + x;
    }
    if (x == 0 || (isinf(x) && (degree == _MATH_ROOT_CUBE || x > 0))) {
        return x;
    }
    if (x < 0 && degree == _MATH_ROOT_SQUARE) {
        return _Math_RaiseDomain(x, 0);
    }
    _Math_MakeExact(&radicand, x);
    _Math_RootExact(&root, &radicand, degree);
    return _Math_RoundExact(&root, type);
}

// Take the hypotenuse of x and y, correctly rounded to type.
long double _Math_TakeHypot(long double x, long double y, _Math_Type type)
{
    _Math_Exact sum;
    _Math_Exact root;
    _Math_Exact side_x;
    _Math_Exact side_y;
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
    _Math_MakeExact(&side_x, x);
    _Math_MakeExact(&side_y, y);
    _Math_MultiplyExact(&side_x, &side_x, &side_x);
    _Math_MultiplyExact(&side_y, &side_y, &side_y);
    _Math_AddExact(&sum, &side_x, &side_y);
    _Math_RootExact(&root, &sum, _MATH_ROOT_SQUARE);
    result = _Math_RoundExact(&root, type);
    if (isinf(result)) {
        errno = ERANGE;
    }
    return result;
}

// Take x times y plus z, correctly rounded once to type.
long double _Math_TakeFma(long double x, long double y, long double z, _Math_Type type)
{
    _Math_Exact sum;
    _Math_Exact addend;
    _Math_Exact product;

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
    _Math_MakeExact(&product, x);
    _Math_MakeExact(&addend, y);
    _Math_MultiplyExact(&product, &product, &addend);
    if (z == 0) {
        return _Math_RoundExact(&product, type);
    }
    _Math_MakeExact(&addend, z);
    _Math_AddExact(&sum, &product, &addend);
    return _Math_RoundExact(&sum, type);
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
    sig = _Math_SplitLDouble(value, &power);
    *exp = power - _MATH_FRACTION_EXP;
    return copysignl(_Math_JoinLDouble(_MATH_FRACTION_EXP, sig), value);
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
    _Math_SplitLDouble(x, &exp);
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
    _Math_SplitLDouble(x, &exp);
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
    return (double) _Math_TakeRoot(x, _MATH_ROOT_CUBE, _MATH_TYPE_DOUBLE);
}

// Take the real cube root of x.
float cbrtf(float x)
{
    return (float) _Math_TakeRoot(x, _MATH_ROOT_CUBE, _MATH_TYPE_FLOAT);
}

// Take the real cube root of x.
long double cbrtl(long double x)
{
    return _Math_TakeRoot(x, _MATH_ROOT_CUBE, _MATH_TYPE_LDOUBLE);
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
    return (double) _Math_TakeHypot(x, y, _MATH_TYPE_DOUBLE);
}

// Take the square root of the sum of the squares of x and y.
float hypotf(float x, float y)
{
    return (float) _Math_TakeHypot(x, y, _MATH_TYPE_FLOAT);
}

// Take the square root of the sum of the squares of x and y.
long double hypotl(long double x, long double y)
{
    return _Math_TakeHypot(x, y, _MATH_TYPE_LDOUBLE);
}

// Take the nonnegative square root of x.
double sqrt(double x)
{
    return (double) _Math_TakeRoot(x, _MATH_ROOT_SQUARE, _MATH_TYPE_DOUBLE);
}

// Take the nonnegative square root of x.
float sqrtf(float x)
{
    return (float) _Math_TakeRoot(x, _MATH_ROOT_SQUARE, _MATH_TYPE_FLOAT);
}

// Take the nonnegative square root of x.
long double sqrtl(long double x)
{
    return _Math_TakeRoot(x, _MATH_ROOT_SQUARE, _MATH_TYPE_LDOUBLE);
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
    sig = _Math_SplitLDouble(x, &exp);
    if (exp >= _MATH_LDOUBLE_INT_SHIFT) {
        return x;
    }
    if (exp < 0) {
        return copysignl(0, x);
    }
    return copysignl(_Math_JoinLDouble(exp, sig & UINT64_MAX << (_MATH_LDOUBLE_INT_SHIFT - exp)), x);
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
    return _Math_MakeLDouble(_MATH_LDOUBLE_EXP_MAX, _MATH_LDOUBLE_QUIET | (_Math_ReadPayload(tagp) & _MATH_LDOUBLE_FRAC_MASK));
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
        next = _Math_StepLDouble(x, (x < y) == (x > 0) ? _MATH_STEP_UP : _MATH_STEP_DOWN);
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
    return (double) _Math_TakeFma(x, y, z, _MATH_TYPE_DOUBLE);
}

// Take x times y plus z, rounded once.
float fmaf(float x, float y, float z)
{
    return (float) _Math_TakeFma(x, y, z, _MATH_TYPE_FLOAT);
}

// Take x times y plus z, rounded once.
long double fmal(long double x, long double y, long double z)
{
    return _Math_TakeFma(x, y, z, _MATH_TYPE_LDOUBLE);
}
