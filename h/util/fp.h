/*
 * C header file for floating-point encodings.
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

#ifndef FP_H
#define FP_H

// Standard headers.
#include <math.h>
#include <stdint.h>
#include <string.h>

// Bits in a byte, for splitting a significand into bytes.
#define FP_BITS_PER_BYTE 8

// Bytes the x87 extended format occupies.
#define FP_EXTENDED_SIZE 10

// Offset of the sign and exponent after the significand's bytes.
#define FP_EXTENDED_TOP_OFF 8

// Bits in the extended format's significand, its integer bit included.
#define FP_EXTENDED_MANT_BITS 64

// The extended format's exponent bias and its all-ones exponent.
#define FP_EXTENDED_BIAS    16383
#define FP_EXTENDED_EXP_MAX 0x7FFF

// The power of two the extended format's smallest denormal digit weighs.
#define FP_EXTENDED_DENORMAL_SHIFT 16445

// The integer bit, and the quiet bit a NaN carries below it.
#define FP_EXTENDED_INT_BIT   ((uint64_t) 1 << 63)
#define FP_EXTENDED_QUIET_BIT ((uint64_t) 1 << 62)

// The sign bit of the extended format's top two bytes.
#define FP_EXTENDED_SIGN_BIT 0x8000

// IEEE binary32 and binary64 bit patterns
uint32_t Fp_FloatBits(float value);
uint64_t Fp_DoubleBits(double value);
float    Fp_FloatFromBits(uint32_t bits);
double   Fp_DoubleFromBits(uint64_t bits);

// The x87 80-bit extended format
void        Fp_EncodeExtended(long double value, uint8_t *out);
long double Fp_DecodeExtended(const uint8_t *in);

#endif // FP_H
