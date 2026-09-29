/*
 * C source file for floating-point encodings.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivancc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivancc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ivancc.  If not, see <https://www.gnu.org/licenses/>.
 */

// Module header.
#include "util/fp.h"

// Return the binary32 bits of a float.
uint32_t Fp_FloatBits(float value)
{
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

// Return the binary64 bits of a double.
uint64_t Fp_DoubleBits(double value)
{
    uint64_t bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

// Return the float binary32 bits stand for.
float Fp_FloatFromBits(uint32_t bits)
{
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

// Return the double binary64 bits stand for.
double Fp_DoubleFromBits(uint64_t bits)
{
    double value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

// Write a value in the x87 extended format.
void Fp_EncodeExtended(long double value, uint8_t *out)
{
    uint16_t top = signbit(value) ? FP_EXTENDED_SIGN_BIT : 0;
    uint64_t mant = 0;
    long double mag = fabsl(value);

    if (isnan(value)) {
        top |= FP_EXTENDED_EXP_MAX;
        mant = FP_EXTENDED_INT_BIT | FP_EXTENDED_QUIET_BIT;
    } else if (isinf(value)) {
        top |= FP_EXTENDED_EXP_MAX;
        mant = FP_EXTENDED_INT_BIT;
    } else if (mag != 0) {
        int exp = 0;
        long double frac = frexpl(mag, &exp);
        int32_t biased = exp - 1 + FP_EXTENDED_BIAS;

        if (biased > 0) {
            top |= (uint16_t) biased;
            mant = (uint64_t) ldexpl(frac, FP_EXTENDED_MANT_BITS);
        } else {
            mant = (uint64_t) ldexpl(mag, FP_EXTENDED_DENORMAL_SHIFT);
        }
    }

    for (int32_t i = 0; i < FP_EXTENDED_TOP_OFF; i++) {
        out[i] = (uint8_t) (mant >> (FP_BITS_PER_BYTE * i));
    }
    out[FP_EXTENDED_TOP_OFF] = (uint8_t) top;
    out[FP_EXTENDED_TOP_OFF + 1] = (uint8_t) (top >> FP_BITS_PER_BYTE);
}

// Read a value written in the x87 extended format.
long double Fp_DecodeExtended(const uint8_t *in)
{
    uint64_t mant = 0;
    uint16_t top = (uint16_t) (in[FP_EXTENDED_TOP_OFF] | in[FP_EXTENDED_TOP_OFF + 1] << FP_BITS_PER_BYTE);
    int32_t biased = top & FP_EXTENDED_EXP_MAX;
    long double value;

    for (int32_t i = 0; i < FP_EXTENDED_TOP_OFF; i++) {
        mant |= (uint64_t) in[i] << (FP_BITS_PER_BYTE * i);
    }

    if (biased == FP_EXTENDED_EXP_MAX) {
        value = mant << 1 ? NAN : INFINITY;
    } else if (biased == 0) {
        value = ldexpl((long double) mant, -FP_EXTENDED_DENORMAL_SHIFT);
    } else {
        value = ldexpl((long double) mant, biased - FP_EXTENDED_BIAS - (FP_EXTENDED_MANT_BITS - 1));
    }
    return top & FP_EXTENDED_SIGN_BIT ? -value : value;
}
