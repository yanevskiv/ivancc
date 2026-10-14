/*
 * C source file for floating-point formats and arithmetic.
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
 * You should have received a copy of the GNU General Public License
 * along with ivcc.  If not, see <https://www.gnu.org/licenses/>.
 */

// Module header.
#include "util/fp.h"

// The binary32 format, a float's.
const Fp_Format Fp_FormatBinary32 = {
    .ff_bits     = 32,
    .ff_mant     = 24,
    .ff_min      = -126,
    .ff_max      = 127,
    .ff_explicit = false
};

// The binary64 format, a double's.
const Fp_Format Fp_FormatBinary64 = {
    .ff_bits     = 64,
    .ff_mant     = 53,
    .ff_min      = -1022,
    .ff_max      = 1023,
    .ff_explicit = false
};

// The x87's extended format, its integer bit stored.
const Fp_Format Fp_FormatBinary80 = {
    .ff_bits     = 80,
    .ff_mant     = 64,
    .ff_min      = -16382,
    .ff_max      = 16383,
    .ff_explicit = true
};

// Make a wide integer of its two words.
Fp_Wide Fp_WideMake(uint64_t hi, uint64_t lo)
{
    Fp_Wide a = {
        .fw_hi = hi,
        .fw_lo = lo
    };
    return a;
}

// Return whether a wide integer is zero.
bool Fp_WideIsZero(Fp_Wide a)
{
    return (a.fw_hi | a.fw_lo) == 0;
}

// Compare two wide integers: below, at or above zero as a is less, equal, more.
int32_t Fp_WideCmp(Fp_Wide a, Fp_Wide b)
{
    if (a.fw_hi != b.fw_hi) {
        return (a.fw_hi > b.fw_hi) - (a.fw_hi < b.fw_hi);
    }
    return (a.fw_lo > b.fw_lo) - (a.fw_lo < b.fw_lo);
}

// Add two wide integers, modulo 2^128.
Fp_Wide Fp_WideAdd(Fp_Wide a, Fp_Wide b)
{
    uint64_t lo = a.fw_lo + b.fw_lo;
    return Fp_WideMake(a.fw_hi + b.fw_hi + (lo < a.fw_lo), lo);
}

// Subtract b from a, modulo 2^128.
Fp_Wide Fp_WideSub(Fp_Wide a, Fp_Wide b)
{
    return Fp_WideMake(a.fw_hi - b.fw_hi - (a.fw_lo < b.fw_lo), a.fw_lo - b.fw_lo);
}

// Multiply two words into their full product, taken in halves.
Fp_Wide Fp_WideMul(uint64_t a, uint64_t b)
{
    uint64_t a_lo = a & FP_HALF_MASK;
    uint64_t a_hi = a >> FP_HALF_BITS;
    uint64_t b_lo = b & FP_HALF_MASK;
    uint64_t b_hi = b >> FP_HALF_BITS;
    uint64_t low = a_lo * b_lo;
    uint64_t cross1 = a_lo * b_hi;
    uint64_t cross2 = a_hi * b_lo;
    uint64_t mid = (low >> FP_HALF_BITS) + (cross1 & FP_HALF_MASK) + (cross2 & FP_HALF_MASK);

    return Fp_WideMake(a_hi * b_hi + (cross1 >> FP_HALF_BITS) + (cross2 >> FP_HALF_BITS) + (mid >> FP_HALF_BITS),
                       (low & FP_HALF_MASK) | mid << FP_HALF_BITS);
}

// Shift a wide integer left by count bits, any count.
Fp_Wide Fp_WideShl(Fp_Wide a, int32_t count)
{
    if (count <= 0) {
        return a;
    }
    if (count >= FP_WIDE_BITS) {
        return Fp_WideMake(0, 0);
    }
    if (count >= FP_WORD_BITS) {
        return Fp_WideMake(a.fw_lo << (count - FP_WORD_BITS), 0);
    }
    return Fp_WideMake(a.fw_hi << count | a.fw_lo >> (FP_WORD_BITS - count), a.fw_lo << count);
}

// Shift a wide integer right by count bits, any count.
Fp_Wide Fp_WideShr(Fp_Wide a, int32_t count)
{
    if (count <= 0) {
        return a;
    }
    if (count >= FP_WIDE_BITS) {
        return Fp_WideMake(0, 0);
    }
    if (count >= FP_WORD_BITS) {
        return Fp_WideMake(0, a.fw_hi >> (count - FP_WORD_BITS));
    }
    return Fp_WideMake(a.fw_hi >> count, a.fw_lo >> count | a.fw_hi << (FP_WORD_BITS - count));
}

// Shift a wide integer right, setting its lowest bit if a set bit fell off.
Fp_Wide Fp_WideShrSticky(Fp_Wide a, int32_t count)
{
    Fp_Wide kept = Fp_WideShr(a, count);

    if (! Fp_WideIsZero(Fp_WideAnd(a, Fp_WideMask(count)))) {
        kept.fw_lo |= 1;
    }
    return kept;
}

// Make the wide integer whose lowest count bits are set.
Fp_Wide Fp_WideMask(int32_t count)
{
    Fp_Wide one = Fp_WideMake(0, 1);
    return Fp_WideSub(Fp_WideShl(one, count), one);
}

// Return the bits two wide integers both have.
Fp_Wide Fp_WideAnd(Fp_Wide a, Fp_Wide b)
{
    return Fp_WideMake(a.fw_hi & b.fw_hi, a.fw_lo & b.fw_lo);
}

// Return the bits either of two wide integers has.
Fp_Wide Fp_WideOr(Fp_Wide a, Fp_Wide b)
{
    return Fp_WideMake(a.fw_hi | b.fw_hi, a.fw_lo | b.fw_lo);
}

// Count the clear bits above a wide integer's top set one, 128 for zero.
int32_t Fp_WideCountZeros(Fp_Wide a)
{
    uint64_t word = a.fw_hi ? a.fw_hi : a.fw_lo;
    int32_t count = a.fw_hi ? 0 : FP_WORD_BITS;

    if (word == 0) {
        return FP_WIDE_BITS;
    }
    for (; ! (word & FP_WORD_TOP); word <<= 1) {
        count++;
    }
    return count;
}

// Make the exact value mant times 2^low.
Fp_Value Fp_FormatMake(Fp_Sign sign, Fp_Wide mant, int32_t low)
{
    int32_t zeros = Fp_WideCountZeros(mant);
    Fp_Value value = {
        .fv_class = FP_CLASS_ZERO,
        .fv_sign  = sign,
        .fv_exp   = 0,
        .fv_mant  = Fp_WideMake(0, 0)
    };

    if (zeros < FP_WIDE_BITS) {
        value.fv_class = FP_CLASS_NORMAL;
        value.fv_exp = low + FP_WIDE_BITS - 1 - zeros;
        value.fv_mant = Fp_WideShl(mant, zeros);
    }
    return value;
}

// Unpack a format's bits into the value they encode.
Fp_Value Fp_FormatUnpack(const Fp_Format *fmt, Fp_Wide bits)
{
    int32_t frac_bits = fmt->ff_explicit ? fmt->ff_mant : fmt->ff_mant - 1;
    int32_t all_ones = 2 * fmt->ff_max + 1;
    Fp_Sign sign = Fp_WideShr(bits, fmt->ff_bits - 1).fw_lo & 1 ? FP_SIGN_NEGATIVE : FP_SIGN_POSITIVE;
    int32_t biased = (int32_t) (Fp_WideShr(bits, frac_bits).fw_lo & (uint64_t) all_ones);
    Fp_Wide frac = Fp_WideAnd(bits, Fp_WideMask(frac_bits));
    Fp_Wide fraction = Fp_WideAnd(bits, Fp_WideMask(fmt->ff_mant - 1));
    Fp_Wide lead = Fp_WideShl(Fp_WideMake(0, 1), fmt->ff_mant - 1);
    Fp_Value value = Fp_FormatMake(sign, Fp_WideMake(0, 0), 0);

    if (fmt->ff_explicit && biased != 0 && Fp_WideIsZero(Fp_WideAnd(frac, lead))) {
        value.fv_class = FP_CLASS_UNSUPPORTED;
        return value;
    }
    if (biased == all_ones) {
        value.fv_class = FP_CLASS_INFINITY;
        if (! Fp_WideIsZero(fraction)) {
            value.fv_class = FP_CLASS_NAN;
            value.fv_mant = Fp_WideShl(fraction, FP_WIDE_BITS - (fmt->ff_mant - 1));
        }
        return value;
    }
    if (biased == 0) {
        value = Fp_FormatMake(sign, frac, fmt->ff_min - (fmt->ff_mant - 1));
        if (value.fv_class != FP_CLASS_ZERO) {
            value.fv_class = FP_CLASS_SUBNORMAL;
        }
        return value;
    }
    return Fp_FormatMake(sign, fmt->ff_explicit ? frac : Fp_WideOr(frac, lead), biased - fmt->ff_max - (fmt->ff_mant - 1));
}

// Pack a value a format holds exactly into its bits.
Fp_Wide Fp_FormatPack(const Fp_Format *fmt, const Fp_Value *value)
{
    int32_t frac_bits = fmt->ff_explicit ? fmt->ff_mant : fmt->ff_mant - 1;
    int32_t all_ones = 2 * fmt->ff_max + 1;
    Fp_Wide lead = Fp_WideShl(Fp_WideMake(0, 1), fmt->ff_mant - 1);
    Fp_Wide sig = Fp_WideMake(0, 0);
    int32_t biased = 0;
    Fp_Wide bits;

    switch (value->fv_class) {
        case FP_CLASS_ZERO: {
        } break;
        case FP_CLASS_INFINITY: {
            biased = all_ones;
            sig = fmt->ff_explicit ? lead : sig;
        } break;
        case FP_CLASS_NAN: {
            biased = all_ones;
            sig = Fp_WideShr(value->fv_mant, FP_WIDE_BITS - (fmt->ff_mant - 1));
            sig = fmt->ff_explicit ? Fp_WideOr(sig, lead) : sig;
        } break;
        default: {
            if (value->fv_exp < fmt->ff_min) {
                sig = Fp_WideShr(value->fv_mant, FP_WIDE_BITS - fmt->ff_mant + (fmt->ff_min - value->fv_exp));
            } else {
                biased = value->fv_exp + fmt->ff_max;
                sig = Fp_WideShr(value->fv_mant, FP_WIDE_BITS - fmt->ff_mant);
                sig = fmt->ff_explicit ? sig : Fp_WideAnd(sig, Fp_WideMask(fmt->ff_mant - 1));
            }
        }
    }
    bits = Fp_WideOr(sig, Fp_WideShl(Fp_WideMake(0, (uint64_t) biased), frac_bits));
    if (value->fv_sign == FP_SIGN_NEGATIVE) {
        bits = Fp_WideOr(bits, Fp_WideShl(Fp_WideMake(0, 1), fmt->ff_bits - 1));
    }
    return bits;
}

// Return whether a value is no number: a NaN, or an encoding of none.
bool Fp_FormatIsNan(const Fp_Value *value)
{
    return value->fv_class == FP_CLASS_NAN || value->fv_class == FP_CLASS_UNSUPPORTED;
}

// Return whether a value signals: an SNaN, or an encoding of no number.
bool Fp_FormatIsSignalling(const Fp_Value *value)
{
    return (value->fv_class == FP_CLASS_NAN && ! (value->fv_mant.fw_hi & FP_WORD_TOP))
           || value->fv_class == FP_CLASS_UNSUPPORTED;
}

// Return whether the kept bits are to be rounded up, by the context's mode.
bool Fp_RoundIsUp(const Fp_Context *ctx, Fp_Sign sign, Fp_Wide kept, Fp_RoundRest rest)
{
    switch (ctx->fc_mode) {
        case FP_ROUND_NEAREST_EVEN: {
            return rest == FP_ROUND_REST_ABOVE || (rest == FP_ROUND_REST_HALF && (kept.fw_lo & 1));
        } break;
        case FP_ROUND_NEAREST_AWAY: {
            return rest >= FP_ROUND_REST_HALF;
        } break;
        case FP_ROUND_UP: {
            return rest != FP_ROUND_REST_NONE && sign == FP_SIGN_POSITIVE;
        } break;
        case FP_ROUND_DOWN: {
            return rest != FP_ROUND_REST_NONE && sign == FP_SIGN_NEGATIVE;
        } break;
        default: {
            return false;
        }
    }
}

// Round a finite nonzero value to its top keep bits, keep at most 128.
Fp_Value Fp_RoundToBits(const Fp_Value *value, int32_t keep, Fp_Context *ctx)
{
    int32_t shift = FP_WIDE_BITS - keep;
    Fp_Wide kept = Fp_WideShr(value->fv_mant, shift);
    Fp_Wide below = Fp_WideAnd(value->fv_mant, Fp_WideMask(shift));
    Fp_RoundRest rest = FP_ROUND_REST_BELOW;

    if (Fp_WideIsZero(below)) {
        rest = FP_ROUND_REST_NONE;
    } else if (shift <= FP_WIDE_BITS) {
        int32_t order = Fp_WideCmp(below, Fp_WideShl(Fp_WideMake(0, 1), shift - 1));
        rest = order < 0 ? FP_ROUND_REST_BELOW : order == 0 ? FP_ROUND_REST_HALF : FP_ROUND_REST_ABOVE;
    }
    if (Fp_RoundIsUp(ctx, value->fv_sign, kept, rest)) {
        kept = Fp_WideAdd(kept, Fp_WideMake(0, 1));
        ctx->fc_flags |= FP_FLAG_ROUNDED_UP;
    }
    if (rest != FP_ROUND_REST_NONE) {
        ctx->fc_flags |= FP_FLAG_INEXACT;
    }
    return Fp_FormatMake(value->fv_sign, kept, value->fv_exp - keep + 1);
}

// Round an exact value to a format, its flags as IEEE 754 says.
Fp_Value Fp_RoundValue(const Fp_Value *exact, const Fp_Format *fmt, Fp_Context *ctx)
{
    Fp_Context local = *ctx;
    Fp_Value result = *exact;
    bool below = exact->fv_exp < fmt->ff_min;
    bool tiny = below;

    if (Fp_FormatIsNan(exact)) {
        ctx->fc_flags |= Fp_FormatIsSignalling(exact) ? FP_FLAG_INVALID : 0;
        return result;
    }
    if (exact->fv_class == FP_CLASS_ZERO || exact->fv_class == FP_CLASS_INFINITY) {
        return result;
    }
    if (below && ctx->fc_tininess == FP_ROUND_TINY_AFTER) {
        tiny = Fp_RoundToBits(exact, fmt->ff_mant, &local).fv_exp < fmt->ff_min;
    }
    local.fc_flags = 0;
    result = Fp_RoundToBits(exact, below ? fmt->ff_mant - (fmt->ff_min - exact->fv_exp) : fmt->ff_mant, &local);
    if (tiny) {
        local.fc_flags |= FP_FLAG_TINY | (local.fc_flags & FP_FLAG_INEXACT ? FP_FLAG_UNDERFLOW : 0);
    }
    if (result.fv_class != FP_CLASS_ZERO && result.fv_exp > fmt->ff_max) {
        bool infinite = ctx->fc_mode == FP_ROUND_NEAREST_EVEN || ctx->fc_mode == FP_ROUND_NEAREST_AWAY
                        || (ctx->fc_mode == FP_ROUND_UP && result.fv_sign == FP_SIGN_POSITIVE)
                        || (ctx->fc_mode == FP_ROUND_DOWN && result.fv_sign == FP_SIGN_NEGATIVE);

        local.fc_flags |= FP_FLAG_OVERFLOW | FP_FLAG_INEXACT;
        local.fc_flags &= ~(uint32_t) FP_FLAG_ROUNDED_UP;
        result = Fp_FormatMake(result.fv_sign, Fp_WideMask(fmt->ff_mant), fmt->ff_max - fmt->ff_mant + 1);
        if (infinite) {
            local.fc_flags |= FP_FLAG_ROUNDED_UP;
            result.fv_class = FP_CLASS_INFINITY;
            result.fv_exp = 0;
            result.fv_mant = Fp_WideMake(0, 0);
        }
    }
    if (result.fv_class == FP_CLASS_NORMAL && result.fv_exp < fmt->ff_min) {
        result.fv_class = FP_CLASS_SUBNORMAL;
    }
    ctx->fc_flags |= local.fc_flags;
    return result;
}

// Give the NaN an invalid operation gives, raising invalid.
Fp_Value Fp_ArithInvalid(Fp_Context *ctx)
{
    Fp_Value nan = Fp_FormatMake(FP_SIGN_POSITIVE, Fp_WideMake(0, 0), 0);

    nan.fv_class = FP_CLASS_NAN;
    ctx->fc_flags |= FP_FLAG_INVALID;
    return nan;
}

// Give a NaN of no payload for an operation on one, invalid if either signals.
Fp_Value Fp_ArithNan(const Fp_Value *a, const Fp_Value *b, Fp_Context *ctx)
{
    Fp_Value nan = Fp_FormatMake(FP_SIGN_POSITIVE, Fp_WideMake(0, 0), 0);

    nan.fv_class = FP_CLASS_NAN;
    if (Fp_FormatIsSignalling(a) || Fp_FormatIsSignalling(b)) {
        ctx->fc_flags |= FP_FLAG_INVALID;
    }
    return nan;
}

// Add b to a, rounded to a format.
Fp_Value Fp_ArithAdd(const Fp_Value *a, const Fp_Value *b, const Fp_Format *fmt, Fp_Context *ctx)
{
    Fp_Sign zero_sign = ctx->fc_mode == FP_ROUND_DOWN ? FP_SIGN_NEGATIVE : FP_SIGN_POSITIVE;
    const Fp_Value *big = a->fv_exp >= b->fv_exp ? a : b;
    const Fp_Value *small = big == a ? b : a;
    Fp_Value exact;

    if (Fp_FormatIsNan(a) || Fp_FormatIsNan(b)) {
        return Fp_ArithNan(a, b, ctx);
    }
    if (a->fv_class == FP_CLASS_INFINITY) {
        return b->fv_class == FP_CLASS_INFINITY && b->fv_sign != a->fv_sign ? Fp_ArithInvalid(ctx) : *a;
    }
    if (b->fv_class == FP_CLASS_INFINITY) {
        return *b;
    }
    if (a->fv_class == FP_CLASS_ZERO && b->fv_class == FP_CLASS_ZERO) {
        exact = *a;
        exact.fv_sign = a->fv_sign == b->fv_sign ? a->fv_sign : zero_sign;
        return exact;
    }
    if (a->fv_class == FP_CLASS_ZERO) {
        return Fp_RoundValue(b, fmt, ctx);
    }
    if (b->fv_class == FP_CLASS_ZERO) {
        return Fp_RoundValue(a, fmt, ctx);
    }

    Fp_Wide large = Fp_WideShrSticky(big->fv_mant, 1);
    Fp_Wide little = Fp_WideShrSticky(small->fv_mant, big->fv_exp - small->fv_exp + 1);
    int32_t low = big->fv_exp - (FP_WIDE_BITS - 2);

    if (a->fv_sign == b->fv_sign) {
        exact = Fp_FormatMake(a->fv_sign, Fp_WideAdd(large, little), low);
    } else if (Fp_WideCmp(large, little) >= 0) {
        exact = Fp_FormatMake(big->fv_sign, Fp_WideSub(large, little), low);
    } else {
        exact = Fp_FormatMake(small->fv_sign, Fp_WideSub(little, large), low);
    }
    if (exact.fv_class == FP_CLASS_ZERO) {
        exact.fv_sign = zero_sign;
    }
    return Fp_RoundValue(&exact, fmt, ctx);
}

// Subtract b from a, rounded to a format.
Fp_Value Fp_ArithSub(const Fp_Value *a, const Fp_Value *b, const Fp_Format *fmt, Fp_Context *ctx)
{
    Fp_Value negated = *b;

    negated.fv_sign = b->fv_sign == FP_SIGN_NEGATIVE ? FP_SIGN_POSITIVE : FP_SIGN_NEGATIVE;
    return Fp_ArithAdd(a, &negated, fmt, ctx);
}

// Multiply a by b, rounded to a format of 64 significand bits at most.
Fp_Value Fp_ArithMul(const Fp_Value *a, const Fp_Value *b, const Fp_Format *fmt, Fp_Context *ctx)
{
    Fp_Sign sign = a->fv_sign != b->fv_sign ? FP_SIGN_NEGATIVE : FP_SIGN_POSITIVE;
    Fp_Value exact = Fp_FormatMake(sign, Fp_WideMake(0, 0), 0);

    if (Fp_FormatIsNan(a) || Fp_FormatIsNan(b)) {
        return Fp_ArithNan(a, b, ctx);
    }
    if (a->fv_class == FP_CLASS_INFINITY || b->fv_class == FP_CLASS_INFINITY) {
        if (a->fv_class == FP_CLASS_ZERO || b->fv_class == FP_CLASS_ZERO) {
            return Fp_ArithInvalid(ctx);
        }
        exact.fv_class = FP_CLASS_INFINITY;
        return exact;
    }
    if (a->fv_class == FP_CLASS_ZERO || b->fv_class == FP_CLASS_ZERO) {
        return exact;
    }
    exact = Fp_FormatMake(sign, Fp_WideMul(a->fv_mant.fw_hi, b->fv_mant.fw_hi),
                          a->fv_exp + b->fv_exp - 2 * (FP_WORD_BITS - 1));
    return Fp_RoundValue(&exact, fmt, ctx);
}

// Divide a by b, rounded to a format of 64 significand bits at most.
Fp_Value Fp_ArithDiv(const Fp_Value *a, const Fp_Value *b, const Fp_Format *fmt, Fp_Context *ctx)
{
    Fp_Sign sign = a->fv_sign != b->fv_sign ? FP_SIGN_NEGATIVE : FP_SIGN_POSITIVE;
    Fp_Value exact = Fp_FormatMake(sign, Fp_WideMake(0, 0), 0);
    Fp_Wide quotient = Fp_WideMake(0, 0);
    uint64_t rem = a->fv_mant.fw_hi;
    uint64_t divisor = b->fv_mant.fw_hi;
    bool carry = false;

    if (Fp_FormatIsNan(a) || Fp_FormatIsNan(b)) {
        return Fp_ArithNan(a, b, ctx);
    }
    if (a->fv_class == FP_CLASS_INFINITY) {
        if (b->fv_class == FP_CLASS_INFINITY) {
            return Fp_ArithInvalid(ctx);
        }
        exact.fv_class = FP_CLASS_INFINITY;
        return exact;
    }
    if (b->fv_class == FP_CLASS_INFINITY) {
        return exact;
    }
    if (b->fv_class == FP_CLASS_ZERO) {
        if (a->fv_class == FP_CLASS_ZERO) {
            return Fp_ArithInvalid(ctx);
        }
        ctx->fc_flags |= FP_FLAG_DIVIDE;
        exact.fv_class = FP_CLASS_INFINITY;
        return exact;
    }
    if (a->fv_class == FP_CLASS_ZERO) {
        return exact;
    }
    for (int32_t i = 0; i < FP_ARITH_QUOTIENT_BITS; i++) {
        quotient = Fp_WideShl(quotient, 1);
        if (carry || rem >= divisor) {
            rem -= divisor;
            quotient.fw_lo |= 1;
        }
        carry = (rem & FP_WORD_TOP) != 0;
        rem <<= 1;
    }
    if (carry || rem != 0) {
        quotient.fw_lo |= 1;
    }
    exact = Fp_FormatMake(sign, quotient, a->fv_exp - b->fv_exp - (FP_ARITH_QUOTIENT_BITS - 1));
    return Fp_RoundValue(&exact, fmt, ctx);
}

// Compare a with b, invalid for the NaNs the kind says.
Fp_Order Fp_ArithCmp(const Fp_Value *a, const Fp_Value *b, Fp_CompareKind kind, Fp_Context *ctx)
{
    int32_t a_side = a->fv_class == FP_CLASS_ZERO ? 0 : a->fv_sign == FP_SIGN_NEGATIVE ? -1 : 1;
    int32_t b_side = b->fv_class == FP_CLASS_ZERO ? 0 : b->fv_sign == FP_SIGN_NEGATIVE ? -1 : 1;
    int32_t order = 0;

    if (Fp_FormatIsNan(a) || Fp_FormatIsNan(b)) {
        if (kind == FP_COMPARE_SIGNALLING || Fp_FormatIsSignalling(a) || Fp_FormatIsSignalling(b)) {
            ctx->fc_flags |= FP_FLAG_INVALID;
        }
        return FP_ORDER_UNORDERED;
    }
    if (a_side != b_side) {
        return a_side < b_side ? FP_ORDER_LESS : FP_ORDER_GREATER;
    }
    if (a_side == 0) {
        return FP_ORDER_EQUAL;
    }
    if (a->fv_class == FP_CLASS_INFINITY || b->fv_class == FP_CLASS_INFINITY) {
        order = (a->fv_class == FP_CLASS_INFINITY) - (b->fv_class == FP_CLASS_INFINITY);
    } else if (a->fv_exp != b->fv_exp) {
        order = (a->fv_exp > b->fv_exp) - (a->fv_exp < b->fv_exp);
    } else {
        order = Fp_WideCmp(a->fv_mant, b->fv_mant);
    }
    order *= a_side;
    return order < 0 ? FP_ORDER_LESS : order == 0 ? FP_ORDER_EQUAL : FP_ORDER_GREATER;
}

// Round a value to a signed integer of bits bits, 0 and invalid out of range.
uint64_t Fp_ToInteger(const Fp_Value *value, int32_t bits, Fp_Context *ctx)
{
    Fp_Context local = *ctx;
    Fp_Value whole = *value;
    uint64_t limit = (uint64_t) 1 << (bits - 1);
    uint64_t magnitude = 0;

    if (Fp_FormatIsNan(value) || value->fv_class == FP_CLASS_INFINITY) {
        ctx->fc_flags |= FP_FLAG_INVALID;
        return 0;
    }
    if (value->fv_class == FP_CLASS_ZERO) {
        return 0;
    }
    local.fc_flags = 0;
    if (value->fv_exp < FP_WORD_BITS) {
        whole = Fp_RoundToBits(value, value->fv_exp + 1, &local);
    }
    if (whole.fv_class == FP_CLASS_ZERO) {
        ctx->fc_flags |= local.fc_flags;
        return 0;
    }
    if (whole.fv_exp < FP_WORD_BITS) {
        magnitude = Fp_WideShr(whole.fv_mant, FP_WIDE_BITS - 1 - whole.fv_exp).fw_lo;
    }
    if (whole.fv_exp >= FP_WORD_BITS || magnitude > limit || (magnitude == limit && whole.fv_sign == FP_SIGN_POSITIVE)) {
        ctx->fc_flags |= FP_FLAG_INVALID;
        return 0;
    }
    ctx->fc_flags |= local.fc_flags;
    return whole.fv_sign == FP_SIGN_NEGATIVE ? 0 - magnitude : magnitude;
}

// Round a signed integer to a format.
Fp_Value Fp_ToValue(int64_t integer, const Fp_Format *fmt, Fp_Context *ctx)
{
    Fp_Sign sign = integer < 0 ? FP_SIGN_NEGATIVE : FP_SIGN_POSITIVE;
    uint64_t magnitude = integer < 0 ? 0 - (uint64_t) integer : (uint64_t) integer;
    Fp_Value exact = Fp_FormatMake(sign, Fp_WideMake(0, magnitude), 0);

    return Fp_RoundValue(&exact, fmt, ctx);
}

// Return the binary32 bits of a float.
uint32_t Fp_HostFloatBits(float value)
{
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

// Return the binary64 bits of a double.
uint64_t Fp_HostDoubleBits(double value)
{
    uint64_t bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

// Return the float binary32 bits stand for.
float Fp_HostFloatFromBits(uint32_t bits)
{
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

// Return the double binary64 bits stand for.
double Fp_HostDoubleFromBits(uint64_t bits)
{
    double value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

// Write a value in the x87 extended format, scaled by halving and doubling.
void Fp_HostEncodeExtended(long double value, uint8_t *out)
{
    uint16_t top = signbit(value) ? FP_EXTENDED_SIGN_BIT : 0;
    uint64_t mant = 0;
    long double mag = signbit(value) ? -value : value;
    int32_t exp = 0;

    if (isnan(value)) {
        top |= FP_EXTENDED_EXP_MAX;
        mant = FP_EXTENDED_INT_BIT | FP_EXTENDED_QUIET_BIT;
    } else if (isinf(value)) {
        top |= FP_EXTENDED_EXP_MAX;
        mant = FP_EXTENDED_INT_BIT;
    } else if (mag != 0) {
        for (; mag >= 2; mag /= 2) {
            exp++;
        }
        for (; mag < 1; mag *= 2) {
            exp--;
        }
        int32_t biased = exp + FP_EXTENDED_BIAS;
        mant = (uint64_t) (mag * FP_EXTENDED_INT_BIT);
        if (biased > 0) {
            top |= (uint16_t) biased;
        } else {
            mant = 1 - biased < FP_WORD_BITS ? mant >> (1 - biased) : 0;
        }
    }

    for (int32_t i = 0; i < FP_EXTENDED_TOP_OFF; i++) {
        out[i] = (uint8_t) (mant >> (FP_BITS_PER_BYTE * i));
    }
    out[FP_EXTENDED_TOP_OFF] = (uint8_t) top;
    out[FP_EXTENDED_TOP_OFF + 1] = (uint8_t) (top >> FP_BITS_PER_BYTE);
}
