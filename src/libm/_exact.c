/*
 * C source file for exact values and long double fields.
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
#include <_exact.h>

// Includes.
#include <_fp.h>
#include <_fpu.h>
#include <_str.h>

// Read a long double's significand.
unsigned long long _Exact_ReadSignificand(long double x)
{
    unsigned long long sig;

    _Str_CopyBytes(&sig, (unsigned char *) &x + _EXACT_LDOUBLE_SIGNIFICAND, sizeof(sig));
    return sig;
}

// Read a long double's sign and biased exponent.
unsigned int _Exact_ReadTop(long double x)
{
    unsigned short top;

    _Str_CopyBytes(&top, (unsigned char *) &x + _EXACT_LDOUBLE_TOP, sizeof(top));
    return top;
}

// Make a long double of its sign and biased exponent and its significand.
long double _Exact_MakeLDouble(unsigned int top, unsigned long long sig)
{
    long double x = 0.0L;
    unsigned long long low = sig;
    unsigned short high = (unsigned short) top;

    _Str_CopyBytes((unsigned char *) &x + _EXACT_LDOUBLE_SIGNIFICAND, &low, sizeof(low));
    _Str_CopyBytes((unsigned char *) &x + _EXACT_LDOUBLE_TOP, &high, sizeof(high));
    return x;
}

// Split a long double into its normalized significand and its exponent.
unsigned long long _Exact_SplitLDouble(long double x, int *exp)
{
    unsigned long long sig = _Exact_ReadSignificand(x);
    unsigned int biased = _Exact_ReadTop(x) & _EXACT_LDOUBLE_EXP_MAX;

    *exp = biased == 0 ? _EXACT_LDOUBLE_EXP_LOW : (int) biased - _EXACT_LDOUBLE_BIAS;
    for (; (sig & _EXACT_LDOUBLE_INT_BIT) == 0; sig <<= 1) {
        (*exp)--;
    }
    return sig;
}

// Join a significand and an exponent into a long double.
long double _Exact_JoinLDouble(int exp, unsigned long long sig)
{
    int shift;
    unsigned long long bits = sig;

    if (bits == 0) {
        return 0.0L;
    }
    for (; (bits & _EXACT_LDOUBLE_INT_BIT) == 0; bits <<= 1) {
        exp--;
    }
    if (exp >= _EXACT_LDOUBLE_EXP_LOW) {
        return _Exact_MakeLDouble((unsigned int) (exp + _EXACT_LDOUBLE_BIAS), bits);
    }
    shift = _EXACT_LDOUBLE_EXP_LOW - exp;
    return _Exact_MakeLDouble(0, shift > _EXACT_LDOUBLE_INT_SHIFT ? 0 : bits >> shift);
}

// Step a long double's magnitude to the next value up or down.
long double _Exact_StepLDouble(long double x, _Exact_Step step)
{
    unsigned int top = _Exact_ReadTop(x);
    unsigned long long sig = _Exact_ReadSignificand(x);

    if (step == _EXACT_STEP_UP && sig == _EXACT_ALL_BITS) {
        sig = _EXACT_LDOUBLE_INT_BIT;
        top++;
    } else if (step == _EXACT_STEP_UP) {
        sig++;
    } else if ((top & _EXACT_LDOUBLE_EXP_MAX) != 0 && sig == _EXACT_LDOUBLE_INT_BIT) {
        top--;
        sig = (top & _EXACT_LDOUBLE_EXP_MAX) == 0 ? _EXACT_LDOUBLE_FRAC_MASK : _EXACT_ALL_BITS;
    } else {
        sig--;
    }
    if ((top & _EXACT_LDOUBLE_EXP_MAX) == 0 && sig >= _EXACT_LDOUBLE_INT_BIT) {
        top++;
    }
    return _Exact_MakeLDouble(top, sig);
}

// Return whether x's sign is negative.
_Bool _Exact_IsNegative(long double x)
{
    return (_Exact_ReadTop(x) & _EXACT_TOP_SIGN) != 0;
}

// Return whether x is a NaN, the x87's refused encodings as glibc takes them.
_Bool _Exact_IsNan(long double x)
{
    unsigned int exp = _Exact_ReadTop(x) & _EXACT_LDOUBLE_EXP_MAX;
    unsigned long long sig = _Exact_ReadSignificand(x);

    return exp != 0 && ((sig & _EXACT_LDOUBLE_INT_BIT) == 0 || (exp == _EXACT_LDOUBLE_EXP_MAX && (sig & _EXACT_LDOUBLE_FRAC_MASK) != 0));
}

// Return whether x is an infinity.
_Bool _Exact_IsInf(long double x)
{
    return (_Exact_ReadTop(x) & _EXACT_LDOUBLE_EXP_MAX) == _EXACT_LDOUBLE_EXP_MAX && _Exact_ReadSignificand(x) == _EXACT_LDOUBLE_INT_BIT;
}

// Clear x's sign.
long double _Exact_ClearSign(long double x)
{
    return _Exact_MakeLDouble(_Exact_ReadTop(x) & ~_EXACT_TOP_SIGN, _Exact_ReadSignificand(x));
}

// Copy y's sign to x.
long double _Exact_CopySign(long double x, long double y)
{
    return _Exact_MakeLDouble((_Exact_ReadTop(x) & ~_EXACT_TOP_SIGN) | (_Exact_ReadTop(y) & _EXACT_TOP_SIGN), _Exact_ReadSignificand(x));
}

// Set w to a 64-bit value.
void _Exact_SetWide(_Exact_Wide *w, unsigned long long low)
{
    _Str_FillBytes(w, 0, sizeof(*w));
    w->ew_limb[0] = (unsigned int) low;
    w->ew_limb[1] = (unsigned int) (low >> _EXACT_LIMB_BITS);
}

// Read w's low 64 bits.
unsigned long long _Exact_ReadWideLow(const _Exact_Wide *w)
{
    return (unsigned long long) w->ew_limb[1] << _EXACT_LIMB_BITS | w->ew_limb[0];
}

// Read w's bit at a place.
unsigned int _Exact_ReadWideBit(const _Exact_Wide *w, int at)
{
    return w->ew_limb[at / _EXACT_LIMB_BITS] >> (at % _EXACT_LIMB_BITS) & _EXACT_LOW_BIT;
}

// Find the place of w's top set bit.
int _Exact_FindWideTop(const _Exact_Wide *w)
{
    for (int i = _EXACT_WIDE_LIMBS - 1; i >= 0; i--) {
        int bit = _EXACT_LIMB_BITS - 1;

        if (w->ew_limb[i] == 0) {
            continue;
        }
        while ((w->ew_limb[i] >> bit) == 0) {
            bit--;
        }
        return i * _EXACT_LIMB_BITS + bit;
    }
    return _EXACT_WIDE_NONE;
}

// Shift w left by n bits, no set bit passing its top.
void _Exact_ShiftWideLeft(_Exact_Wide *w, int n)
{
    int limbs = n / _EXACT_LIMB_BITS;
    int bits = n % _EXACT_LIMB_BITS;

    for (int i = _EXACT_WIDE_LIMBS - 1; i >= 0; i--) {
        unsigned int high = i - limbs >= 0 ? w->ew_limb[i - limbs] : 0;
        unsigned int low = i - limbs - 1 >= 0 ? w->ew_limb[i - limbs - 1] : 0;

        w->ew_limb[i] = bits == 0 ? high : (unsigned int) (high << bits | low >> (_EXACT_LIMB_BITS - bits));
    }
}

// Shift w right by n bits, returning whether a set bit fell off.
_Bool _Exact_ShiftWideRight(_Exact_Wide *w, int n)
{
    _Bool lost = 0;
    int limbs = n / _EXACT_LIMB_BITS;
    int bits = n % _EXACT_LIMB_BITS;

    if (n >= _EXACT_WIDE_BITS) {
        lost = _Exact_FindWideTop(w) != _EXACT_WIDE_NONE;
        _Str_FillBytes(w, 0, sizeof(*w));
        return lost;
    }
    for (int i = 0; i < limbs; i++) {
        lost = lost || w->ew_limb[i] != 0;
    }
    lost = lost || (w->ew_limb[limbs] & ((_EXACT_LOW_BIT << bits) - 1)) != 0;
    for (int i = 0; i < _EXACT_WIDE_LIMBS; i++) {
        unsigned int low = i + limbs < _EXACT_WIDE_LIMBS ? w->ew_limb[i + limbs] : 0;
        unsigned int high = i + limbs + 1 < _EXACT_WIDE_LIMBS ? w->ew_limb[i + limbs + 1] : 0;

        w->ew_limb[i] = bits == 0 ? low : (unsigned int) (low >> bits | high << (_EXACT_LIMB_BITS - bits));
    }
    return lost;
}

// Shift w left by n bits, or right by -n, returning whether a set bit fell off.
_Bool _Exact_AlignWide(_Exact_Wide *w, int n)
{
    if (n >= 0) {
        _Exact_ShiftWideLeft(w, n);
        return 0;
    }
    return _Exact_ShiftWideRight(w, -n);
}

// Compare a with b, negative, zero or positive.
int _Exact_CompareWides(const _Exact_Wide *a, const _Exact_Wide *b)
{
    for (int i = _EXACT_WIDE_LIMBS - 1; i >= 0; i--) {
        if (a->ew_limb[i] != b->ew_limb[i]) {
            return a->ew_limb[i] < b->ew_limb[i] ? -1 : 1;
        }
    }
    return 0;
}

// Add add to w.
void _Exact_AddWide(_Exact_Wide *w, const _Exact_Wide *add)
{
    unsigned long long carry = 0;

    for (int i = 0; i < _EXACT_WIDE_LIMBS; i++) {
        carry += (unsigned long long) w->ew_limb[i] + add->ew_limb[i];
        w->ew_limb[i] = (unsigned int) carry;
        carry >>= _EXACT_LIMB_BITS;
    }
}

// Subtract sub from w, sub not above it.
void _Exact_SubtractWide(_Exact_Wide *w, const _Exact_Wide *sub)
{
    unsigned long long borrow = 0;

    for (int i = 0; i < _EXACT_WIDE_LIMBS; i++) {
        unsigned long long diff = (unsigned long long) w->ew_limb[i] - sub->ew_limb[i] - borrow;

        w->ew_limb[i] = (unsigned int) diff;
        borrow = diff >> _EXACT_LIMB_BITS & _EXACT_LOW_BIT;
    }
}

// Multiply a by b into to, the product below w's top.
void _Exact_MultiplyWides(_Exact_Wide *to, const _Exact_Wide *a, const _Exact_Wide *b)
{
    _Exact_Wide product;

    _Str_FillBytes(&product, 0, sizeof(product));
    for (int i = 0; i < _EXACT_WIDE_LIMBS; i++) {
        unsigned long long carry = 0;

        for (int j = 0; i + j < _EXACT_WIDE_LIMBS; j++) {
            carry += (unsigned long long) a->ew_limb[i] * b->ew_limb[j] + product.ew_limb[i + j];
            product.ew_limb[i + j] = (unsigned int) carry;
            carry >>= _EXACT_LIMB_BITS;
        }
    }
    *to = product;
}

// Raise base to the kth power into to, by squaring.
void _Exact_RaiseWide(_Exact_Wide *to, const _Exact_Wide *base, int k)
{
    _Exact_Wide square = *base;

    _Exact_SetWide(to, _EXACT_LOW_BIT);
    for (; k > 0; k >>= 1) {
        if ((k & 1) != 0) {
            _Exact_MultiplyWides(to, to, &square);
        }
        if (k > 1) {
            _Exact_MultiplyWides(&square, &square, &square);
        }
    }
}

// Divide w by a nonzero d, returning the remainder.
unsigned int _Exact_DivideWideSmall(_Exact_Wide *w, unsigned int d)
{
    unsigned long long rest = 0;

    for (int i = _EXACT_WIDE_LIMBS - 1; i >= 0; i--) {
        rest = rest << _EXACT_LIMB_BITS | w->ew_limb[i];
        w->ew_limb[i] = (unsigned int) (rest / d);
        rest %= d;
    }
    return (unsigned int) rest;
}

// Divide a by a nonzero b into to, the quotient truncated.
void _Exact_DivideWides(_Exact_Wide *to, const _Exact_Wide *a, const _Exact_Wide *b)
{
    _Exact_Wide rest;
    _Exact_Wide quotient;

    _Exact_SetWide(&rest, 0);
    _Exact_SetWide(&quotient, 0);
    for (int i = _Exact_FindWideTop(a); i >= 0; i--) {
        _Exact_ShiftWideLeft(&rest, 1);
        rest.ew_limb[0] |= _Exact_ReadWideBit(a, i);
        if (_Exact_CompareWides(&rest, b) >= 0) {
            _Exact_SubtractWide(&rest, b);
            quotient.ew_limb[i / _EXACT_LIMB_BITS] |= _EXACT_LOW_BIT << (i % _EXACT_LIMB_BITS);
        }
    }
    *to = quotient;
}

// Make the exact value of a finite nonzero long double.
void _Exact_MakeValue(_Exact_Value *exact, long double x)
{
    int exp;

    _Exact_SetWide(&exact->ev_sig, _Exact_SplitLDouble(x, &exp));
    exact->ev_unit = exp - _EXACT_LDOUBLE_INT_SHIFT;
    exact->ev_negative = _Exact_IsNegative(x) != 0;
    exact->ev_sticky = 0;
}

// Multiply exact values a and b, neither sticky, into product.
void _Exact_MultiplyValues(_Exact_Value *product, const _Exact_Value *a, const _Exact_Value *b)
{
    int unit = a->ev_unit + b->ev_unit;
    _Bool negative = a->ev_negative != b->ev_negative;

    _Exact_MultiplyWides(&product->ev_sig, &a->ev_sig, &b->ev_sig);
    product->ev_unit = unit;
    product->ev_negative = negative;
    product->ev_sticky = 0;
}

// Add exact values a and b, neither sticky, into sum, below a window sticky.
void _Exact_AddValues(_Exact_Value *sum, const _Exact_Value *a, const _Exact_Value *b)
{
    int order;
    _Bool lost;
    _Exact_Wide one;
    _Exact_Wide wa = a->ev_sig;
    _Exact_Wide wb = b->ev_sig;
    int top_a = a->ev_unit + _Exact_FindWideTop(&wa);
    int top_b = b->ev_unit + _Exact_FindWideTop(&wb);
    int base = (top_a > top_b ? top_a : top_b) + 1 - _EXACT_SUM_WINDOW;

    lost = _Exact_AlignWide(&wa, a->ev_unit - base);
    lost = _Exact_AlignWide(&wb, b->ev_unit - base) || lost;
    order = _Exact_CompareWides(&wa, &wb);
    sum->ev_unit = base;
    sum->ev_sticky = lost;
    if (a->ev_negative == b->ev_negative) {
        sum->ev_sig = wa;
        _Exact_AddWide(&sum->ev_sig, &wb);
        sum->ev_negative = a->ev_negative;
        return;
    }
    sum->ev_sig = order >= 0 ? wa : wb;
    _Exact_SubtractWide(&sum->ev_sig, order >= 0 ? &wb : &wa);
    sum->ev_negative = order >= 0 ? a->ev_negative : b->ev_negative;
    if (lost) {
        _Exact_SetWide(&one, _EXACT_LOW_BIT);
        _Exact_SubtractWide(&sum->ev_sig, &one);
    }
    if (order == 0) {
        sum->ev_negative = _Fpu_ReadRound() == _FPU_DOWNWARD;
    }
}

// Take the degree's root of an exact radicand into root, a round bit spare.
void _Exact_ExtractRoot(_Exact_Value *root, const _Exact_Value *radicand, _Exact_Root degree)
{
    _Bool lost;
    _Exact_Wide rest;
    _Exact_Wide next;
    _Exact_Wide trial;
    _Exact_Wide power;
    int k = (int) degree;
    _Exact_Wide n = radicand->ev_sig;
    int shift = k * _EXACT_ROOT_BITS - 1 - _Exact_FindWideTop(&n);

    while ((radicand->ev_unit - shift) % k != 0) {
        shift--;
    }
    lost = _Exact_AlignWide(&n, shift);
    _Exact_SetWide(&rest, 0);
    _Exact_SetWide(&root->ev_sig, 0);
    for (int i = _EXACT_ROOT_BITS - 1; i >= 0; i--) {
        _Exact_ShiftWideLeft(&rest, k);
        for (int j = 0; j < k; j++) {
            rest.ew_limb[0] |= _Exact_ReadWideBit(&n, k * i + j) << j;
        }
        _Exact_ShiftWideLeft(&root->ev_sig, 1);
        next = root->ev_sig;
        next.ew_limb[0] |= _EXACT_LOW_BIT;
        _Exact_RaiseWide(&trial, &next, k);
        _Exact_RaiseWide(&power, &root->ev_sig, k);
        _Exact_SubtractWide(&trial, &power);
        if (_Exact_CompareWides(&rest, &trial) >= 0) {
            _Exact_SubtractWide(&rest, &trial);
            root->ev_sig.ew_limb[0] |= _EXACT_LOW_BIT;
        }
    }
    root->ev_unit = (radicand->ev_unit - shift) / k;
    root->ev_negative = radicand->ev_negative;
    root->ev_sticky = lost || radicand->ev_sticky || _Exact_FindWideTop(&rest) != _EXACT_WIDE_NONE;
}

// Round exact's significand to its top keep bits in the current direction.
unsigned long long _Exact_RoundBits(const _Exact_Value *exact, int keep, int *low, _Bool *inexact)
{
    _Bool up;
    unsigned long long kept;
    _Bool half = 0;
    int mode = _Fpu_ReadRound();
    _Exact_Wide rest = exact->ev_sig;
    _Bool below = exact->ev_sticky;

    *low = _Exact_FindWideTop(&rest) - keep + 1;
    if (*low <= 0) {
        _Exact_ShiftWideLeft(&rest, -*low);
    } else {
        below = _Exact_ShiftWideRight(&rest, *low - 1) || below;
        half = (rest.ew_limb[0] & _EXACT_LOW_BIT) != 0;
        _Exact_ShiftWideRight(&rest, 1);
    }
    kept = _Exact_ReadWideLow(&rest);
    *inexact = half || below;
    if (mode == _FPU_TONEAREST) {
        up = half && (below || (kept & _EXACT_LOW_BIT) != 0);
    } else if (mode == _FPU_UPWARD || mode == _FPU_DOWNWARD) {
        up = *inexact && exact->ev_negative == (mode == _FPU_DOWNWARD);
    } else {
        up = 0;
    }
    if (! up) {
        return kept;
    }
    kept++;
    if (keep > 0 && (keep == _FP_LDOUBLE_MANT ? kept == 0 : kept >> keep != 0)) {
        kept = _EXACT_LDOUBLE_INT_BIT >> (_FP_LDOUBLE_MANT - keep);
        (*low)++;
    }
    return kept;
}

// Round an exact value once to type in the current direction, with its flags.
long double _Exact_RoundValue(const _Exact_Value *exact, _Exact_Type type)
{
    int low;
    int min;
    int max;
    int keep;
    _Bool tiny;
    int digits;
    _Bool inexact;
    unsigned long long kept;
    _Bool infinite;
    long double value;
    int mode = _Fpu_ReadRound();
    int top = _Exact_FindWideTop(&exact->ev_sig);
    int exp = exact->ev_unit + top;

    switch (type) {
        case _EXACT_TYPE_FLOAT: {
            digits = _FP_FLOAT_MANT;
            min = _FP_FLOAT_MIN;
            max = _FP_FLOAT_MAX;
        } break;
        case _EXACT_TYPE_DOUBLE: {
            digits = _FP_DOUBLE_MANT;
            min = _FP_DOUBLE_MIN;
            max = _FP_DOUBLE_MAX;
        } break;
        default: {
            digits = _FP_LDOUBLE_MANT;
            min = _FP_LDOUBLE_MIN;
            max = _FP_LDOUBLE_MAX;
        } break;
    }
    if (top == _EXACT_WIDE_NONE) {
        return exact->ev_negative ? -0.0L : 0.0L;
    }
    tiny = exp < min;
    if (exp == min - 1) {
        _Exact_RoundBits(exact, digits, &low, &inexact);
        tiny = low == top - digits + 1;
    }
    keep = exp < min ? digits - (min - exp) : digits;
    kept = _Exact_RoundBits(exact, keep, &low, &inexact);
    if (exp >= min && exact->ev_unit + low + keep - 1 > max) {
        infinite = mode == _FPU_TONEAREST || (mode == _FPU_UPWARD && ! exact->ev_negative) || (mode == _FPU_DOWNWARD && exact->ev_negative);
        _Fpu_RaiseFlags(_FPU_OVERFLOW | _FPU_INEXACT);
        value = infinite ? _EXACT_INFINITY : _Exact_JoinLDouble(max, _EXACT_ALL_BITS << (_FP_LDOUBLE_MANT - digits));
        return exact->ev_negative ? -value : value;
    }
    if (inexact) {
        _Fpu_RaiseFlags(tiny ? _FPU_UNDERFLOW | _FPU_INEXACT : _FPU_INEXACT);
    }
    value = _Exact_JoinLDouble(exact->ev_unit + low + _EXACT_LDOUBLE_INT_SHIFT, kept);
    return exact->ev_negative ? -value : value;
}
