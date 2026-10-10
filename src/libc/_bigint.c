/*
 * C source file for the big integers exact conversions take.
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

// Module header.
#include <_bigint.h>

// Set num to value.
void _Bigint_Set(struct _Bigint_Number *num, unsigned long long value)
{
    num->bn_len = 0;
    while (value != 0) {
        num->bn_word[num->bn_len] = (unsigned int) value;
        num->bn_len++;
        value >>= _BIGINT_WORD_BITS;
    }
}

// Multiply num by mul and add add.
void _Bigint_MulAdd(struct _Bigint_Number *num, unsigned int mul, unsigned int add)
{
    unsigned long long carry = add;
    int i;

    for (i = 0; i < num->bn_len; i++) {
        carry += (unsigned long long) num->bn_word[i] * mul;
        num->bn_word[i] = (unsigned int) carry;
        carry >>= _BIGINT_WORD_BITS;
    }
    if (carry != 0) {
        num->bn_word[num->bn_len] = (unsigned int) carry;
        num->bn_len++;
    }
}

// Multiply num by 5^exp.
void _Bigint_MulPow5(struct _Bigint_Number *num, int exp)
{
    unsigned int mul = 1;

    while (exp >= _BIGINT_POW5_STEP) {
        _Bigint_MulAdd(num, _BIGINT_POW5_WORD, 0);
        exp -= _BIGINT_POW5_STEP;
    }
    while (exp > 0) {
        mul *= 5;
        exp--;
    }
    _Bigint_MulAdd(num, mul, 0);
}

// Multiply num by 2^bits.
void _Bigint_ShiftLeft(struct _Bigint_Number *num, int bits)
{
    int words = bits / _BIGINT_WORD_BITS;
    int shift = bits % _BIGINT_WORD_BITS;
    unsigned int top;
    int i;

    if (num->bn_len == 0) {
        return;
    }
    if (shift != 0) {
        top = num->bn_word[num->bn_len - 1] >> (_BIGINT_WORD_BITS - shift);
        for (i = num->bn_len - 1; i > 0; i--) {
            num->bn_word[i] = num->bn_word[i] << shift | num->bn_word[i - 1] >> (_BIGINT_WORD_BITS - shift);
        }
        num->bn_word[0] <<= shift;
        if (top != 0) {
            num->bn_word[num->bn_len] = top;
            num->bn_len++;
        }
    }
    if (words != 0) {
        for (i = num->bn_len - 1; i >= 0; i--) {
            num->bn_word[i + words] = num->bn_word[i];
        }
        for (i = 0; i < words; i++) {
            num->bn_word[i] = 0;
        }
        num->bn_len += words;
    }
}

// Divide num by 2^bits, dropping the bits shifted out.
void _Bigint_ShiftRight(struct _Bigint_Number *num, int bits)
{
    int words = bits / _BIGINT_WORD_BITS;
    int shift = bits % _BIGINT_WORD_BITS;
    int i;

    if (words >= num->bn_len) {
        num->bn_len = 0;
        return;
    }
    for (i = 0; i < num->bn_len - words; i++) {
        num->bn_word[i] = num->bn_word[i + words];
    }
    num->bn_len -= words;
    if (shift != 0) {
        for (i = 0; i < num->bn_len - 1; i++) {
            num->bn_word[i] = num->bn_word[i] >> shift | num->bn_word[i + 1] << (_BIGINT_WORD_BITS - shift);
        }
        num->bn_word[num->bn_len - 1] >>= shift;
        if (num->bn_word[num->bn_len - 1] == 0) {
            num->bn_len--;
        }
    }
}

// Compare num1 with num2, as strcmp does.
int _Bigint_Compare(const struct _Bigint_Number *num1, const struct _Bigint_Number *num2)
{
    int i;

    if (num1->bn_len != num2->bn_len) {
        return num1->bn_len < num2->bn_len ? -1 : 1;
    }
    for (i = num1->bn_len - 1; i >= 0; i--) {
        if (num1->bn_word[i] != num2->bn_word[i]) {
            return num1->bn_word[i] < num2->bn_word[i] ? -1 : 1;
        }
    }
    return 0;
}

// Subtract num2 from num1, which is no smaller.
void _Bigint_Sub(struct _Bigint_Number *num1, const struct _Bigint_Number *num2)
{
    unsigned long long borrow = 0;
    unsigned long long word;
    int i;

    for (i = 0; i < num1->bn_len; i++) {
        word = (unsigned long long) num1->bn_word[i] - borrow - (i < num2->bn_len ? num2->bn_word[i] : 0);
        num1->bn_word[i] = (unsigned int) word;
        borrow = word >> _BIGINT_WORD_BITS != 0;
    }
    while (num1->bn_len > 0 && num1->bn_word[num1->bn_len - 1] == 0) {
        num1->bn_len--;
    }
}

// Give the number of bits num takes, 0 for zero.
int _Bigint_Bits(const struct _Bigint_Number *num)
{
    unsigned int top;
    int bits;

    if (num->bn_len == 0) {
        return 0;
    }
    bits = (num->bn_len - 1) * _BIGINT_WORD_BITS;
    for (top = num->bn_word[num->bn_len - 1]; top != 0; top >>= 1) {
        bits++;
    }
    return bits;
}

// Divide num by den, a quotient under 2^64, and leave the remainder in num.
unsigned long long _Bigint_Divide(struct _Bigint_Number *num, const struct _Bigint_Number *den)
{
    struct _Bigint_Number part;
    unsigned long long quot = 0;
    int shift = _Bigint_Bits(num) - _Bigint_Bits(den);
    int i;

    if (shift < 0) {
        return 0;
    }
    part.bn_len = den->bn_len;
    for (i = 0; i < den->bn_len; i++) {
        part.bn_word[i] = den->bn_word[i];
    }
    _Bigint_ShiftLeft(&part, shift);
    for (; shift >= 0; shift--) {
        quot <<= 1;
        if (_Bigint_Compare(num, &part) >= 0) {
            _Bigint_Sub(num, &part);
            quot |= 1;
        }
        _Bigint_ShiftRight(&part, 1);
    }
    return quot;
}
