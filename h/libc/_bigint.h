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

#ifndef _BIGINT_H
#define _BIGINT_H

// The words of a number, 40960 bits, past the 38247 of strtold's 5^16472.
#define _BIGINT_WORDS 1280

// The bits of a word.
#define _BIGINT_WORD_BITS 32

// The largest power of 5 a word holds, 5^13.
#define _BIGINT_POW5_WORD 1220703125U
#define _BIGINT_POW5_STEP 13

// The largest power of 10 a word holds, 10^9.
#define _BIGINT_POW10_WORD 1000000000U
#define _BIGINT_POW10_STEP 9

// An unsigned number in words, least significant first, its top word nonzero.
typedef struct _Bigint_Number _Bigint_Number;
struct _Bigint_Number {
    int bn_len;                          // the words in use, 0 for zero
    unsigned int bn_word[_BIGINT_WORDS]; // the words, 32 bits in each
};

// Arithmetic
void               _Bigint_SetValue(_Bigint_Number *num, unsigned long long value);
void               _Bigint_MulAddWords(_Bigint_Number *num, unsigned int mul, unsigned int add);
void               _Bigint_MulPow5(_Bigint_Number *num, int exp);
void               _Bigint_ShiftBitsLeft(_Bigint_Number *num, int bits);
void               _Bigint_ShiftBitsRight(_Bigint_Number *num, int bits);
int                _Bigint_CompareNumbers(const _Bigint_Number *num1, const _Bigint_Number *num2);
void               _Bigint_SubtractNumber(_Bigint_Number *num1, const _Bigint_Number *num2);
int                _Bigint_CountBits(const _Bigint_Number *num);
unsigned long long _Bigint_DivideNumber(_Bigint_Number *num, const _Bigint_Number *den);
unsigned int       _Bigint_DivideWord(_Bigint_Number *num, unsigned int den);
unsigned long long _Bigint_SplitBits(_Bigint_Number *num, int bits);

#endif // _BIGINT_H
