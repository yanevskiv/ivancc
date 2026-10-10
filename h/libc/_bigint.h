/*
 * C header file for the big integers exact conversions take.
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
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivancc; see
 * the files LICENSE and COPYING.RUNTIME respectively.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef __BIGINT_H__
#define __BIGINT_H__

// The words of a number, 40960 bits, past the 38247 of strtold's 5^16472.
#define _BIGINT_WORDS 1280

// The bits of a word.
#define _BIGINT_WORD_BITS 32

// The largest power of 5 a word holds, 5^13.
#define _BIGINT_POW5_WORD 1220703125U
#define _BIGINT_POW5_STEP 13

// An unsigned number in words, least significant first, its top word nonzero.
struct _Bigint_Number {
    int bn_len;
    unsigned int bn_word[_BIGINT_WORDS];
};

// Arithmetic
void _Bigint_Set(struct _Bigint_Number *num, unsigned long long value);
void _Bigint_MulAdd(struct _Bigint_Number *num, unsigned int mul, unsigned int add);
void _Bigint_MulPow5(struct _Bigint_Number *num, int exp);
void _Bigint_ShiftLeft(struct _Bigint_Number *num, int bits);
void _Bigint_ShiftRight(struct _Bigint_Number *num, int bits);
int _Bigint_Compare(const struct _Bigint_Number *num1, const struct _Bigint_Number *num2);
void _Bigint_Sub(struct _Bigint_Number *num1, const struct _Bigint_Number *num2);
int _Bigint_Bits(const struct _Bigint_Number *num);
unsigned long long _Bigint_Divide(struct _Bigint_Number *num, const struct _Bigint_Number *den);

#endif // __BIGINT_H__
