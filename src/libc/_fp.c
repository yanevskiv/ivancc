/*
 * C source file for the conversion of text to a floating number.
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
#include <_fp.h>

// The exact arithmetic a decimal is rounded by.
#include <_bigint.h>

// The error number of a value out of range.
#include <_sys.h>

// The error number of the last failed call.
#include <_err.h>

// The loop that copies a value's bytes.
#include <_str.h>

// The format of float, its exponents one below C's.
const _Fp_Format _Fp_FloatFormat = {
    .ff_mant     = _FP_FLOAT_MANT,
    .ff_min      = _FP_FLOAT_MIN,
    .ff_max      = _FP_FLOAT_MAX,
    .ff_explicit = 0
};

// The format of double, its exponents one below C's.
const _Fp_Format _Fp_DoubleFormat = {
    .ff_mant     = _FP_DOUBLE_MANT,
    .ff_min      = _FP_DOUBLE_MIN,
    .ff_max      = _FP_DOUBLE_MAX,
    .ff_explicit = 0
};

// The format of long double, its exponents one below C's, its leading bit kept.
const _Fp_Format _Fp_LongDoubleFormat = {
    .ff_mant     = _FP_LDOUBLE_MANT,
    .ff_min      = _FP_LDOUBLE_MIN,
    .ff_max      = _FP_LDOUBLE_MAX,
    .ff_explicit = 1
};

// Read character index of str, a wide string or a narrow one.
int _Fp_ReadChar(const void *str, _Fp_Text text, unsigned long index)
{
    if (text == _FP_TEXT_WIDE) {
        return ((const int *) str)[index];
    }
    return ((const unsigned char *) str)[index];
}

// Check whether ch is white space in the "C" locale.
_Bool _Fp_IsSpace(int ch)
{
    return ch == ' ' || (ch >= '\t' && ch <= '\r');
}

// Check whether ch is a letter or a digit in the "C" locale.
_Bool _Fp_IsAlnum(int ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

// Match word at index of str in any case, and give its length, or 0.
unsigned long _Fp_MatchWord(const void *str, _Fp_Text text, unsigned long index, const char *word)
{
    unsigned long i;
    int ch;

    for (i = 0; word[i] != '\0'; i++) {
        ch = _Fp_ReadChar(str, text, index + i);
        if (_Fp_ToLower(ch) != word[i]) {
            return 0;
        }
    }
    return i;
}

// Read an exponent after letter at index into *exp, and give the index past it.
unsigned long _Fp_ReadExponent(const void *str, _Fp_Text text, unsigned long index, int letter, long *exp)
{
    unsigned long i = index + 1;
    _Bool negative = 0;
    long value = 0;
    int ch = _Fp_ReadChar(str, text, index);

    *exp = 0;
    if (_Fp_ToLower(ch) != letter) {
        return index;
    }
    ch = _Fp_ReadChar(str, text, i);
    if (ch == '+' || ch == '-') {
        negative = ch == '-';
        i++;
    }
    ch = _Fp_ReadChar(str, text, i);
    if (ch < '0' || ch > '9') {
        return index;
    }
    for (; ch >= '0' && ch <= '9'; ch = _Fp_ReadChar(str, text, i)) {
        if (value < _FP_EXP_LIMIT) {
            value = value * 10 + (ch - '0');
        }
        i++;
    }
    *exp = negative ? -value : value;
    return i;
}

// Read a NaN's payload up to stop as strtoull in base 0, or 0, as glibc's.
unsigned long long _Fp_ReadPayload(const void *str, _Fp_Text text, unsigned long index, unsigned long stop)
{
    unsigned long long value = 0;
    int base = 10;
    int digit;

    if (_Fp_ReadChar(str, text, index) == '0') {
        base = 8;
        if (_Fp_ToLower(_Fp_ReadChar(str, text, index + 1)) == 'x' && _Fp_ToHexDigit(_Fp_ReadChar(str, text, index + 2)) < _FP_HEX_LIMIT) {
            base = _FP_HEX_LIMIT;
            index += 2;
        }
    }
    for (; index < stop; index++) {
        digit = _Fp_ToHexDigit(_Fp_ReadChar(str, text, index));
        if (digit >= base) {
            return 0;
        }
        value = value > (_FP_PAYLOAD_MAX - (unsigned long long) digit) / (unsigned long long) base ? _FP_PAYLOAD_MAX : value * (unsigned long long) base + (unsigned long long) digit;
    }
    return value;
}

// Read a NaN's parenthesized characters at index, and give the index past them.
unsigned long _Fp_ReadNotANumber(const void *str, _Fp_Text text, unsigned long index, unsigned long long *payload)
{
    unsigned long i = index + 1;
    int ch;

    *payload = 0;
    if (_Fp_ReadChar(str, text, index) != '(') {
        return index;
    }
    for (ch = _Fp_ReadChar(str, text, i); ch == '_' || _Fp_IsAlnum(ch); ch = _Fp_ReadChar(str, text, i)) {
        i++;
    }
    if (ch != ')') {
        return index;
    }
    *payload = _Fp_ReadPayload(str, text, index + 1, i);
    return i + 1;
}

// Scan a decimal's digits and exponent at index, and give the index past them.
unsigned long _Fp_ScanDecimal(const void *str, _Fp_Text text, unsigned long index, int point, _Fp_Decimal *dec)
{
    unsigned long i;
    _Bool digits = 0;
    _Bool fraction = 0;
    long exp;
    int ch;

    dec->fd_first = 0;
    dec->fd_count = 0;
    dec->fd_exp = 0;
    dec->fd_sticky = 0;
    for (i = index; ; i++) {
        ch = _Fp_ReadChar(str, text, i);
        if (ch == point && ! fraction) {
            fraction = 1;
            continue;
        }
        if (ch < '0' || ch > '9') {
            break;
        }
        digits = 1;
        if (dec->fd_count == 0 && ch == '0') {
            dec->fd_exp -= fraction;
            continue;
        }
        if (dec->fd_count == 0) {
            dec->fd_first = i;
        }
        if (dec->fd_count < _FP_DIGITS) {
            dec->fd_count++;
            dec->fd_exp -= fraction;
        } else {
            dec->fd_sticky |= ch != '0';
            dec->fd_exp += ! fraction;
        }
    }
    if (! digits) {
        return index;
    }
    i = _Fp_ReadExponent(str, text, i, 'e', &exp);
    dec->fd_exp += exp;
    return i;
}

// Read a hexadecimal's digits and exponent at index, and give the end.
unsigned long _Fp_ReadHex(const void *str, _Fp_Text text, unsigned long index, int point, _Fp_Unrounded *value)
{
    unsigned long long mant = 0;
    unsigned int extra = 0;
    _Bool sticky = 0;
    _Bool fraction = 0;
    long count = 0;
    long exp = 0;
    long power;
    unsigned long i;
    int ch;
    int digit;

    for (i = index; ; i++) {
        ch = _Fp_ReadChar(str, text, i);
        if (ch == point && ! fraction) {
            fraction = 1;
            continue;
        }
        digit = _Fp_ToHexDigit(ch);
        if (digit == _FP_HEX_LIMIT) {
            break;
        }
        if (count == 0 && digit == 0) {
            exp -= fraction * _FP_HEX_BITS;
            continue;
        }
        if (count < _FP_MANT_BITS / _FP_HEX_BITS) {
            mant = mant << _FP_HEX_BITS | (unsigned long long) digit;
            exp -= fraction * _FP_HEX_BITS;
        } else {
            if (count == _FP_MANT_BITS / _FP_HEX_BITS) {
                extra = (unsigned int) digit;
            } else {
                sticky |= digit != 0;
            }
            exp += ! fraction * _FP_HEX_BITS;
        }
        count++;
    }
    i = _Fp_ReadExponent(str, text, i, 'p', &power);
    value->fu_mant = mant;
    value->fu_exp = exp + power + _FP_MANT_BITS - 1;
    value->fu_rest = _FP_REST_NONE;
    if (mant == 0) {
        return i;
    }
    while (mant >> (_FP_MANT_BITS - 1) == 0) {
        mant = mant << 1 | extra >> (_FP_HEX_BITS - 1);
        extra = extra << 1 & (_FP_HEX_LIMIT - 1);
        value->fu_exp--;
    }
    value->fu_mant = mant;
    if (extra > _FP_HEX_TOP || (extra == _FP_HEX_TOP && sticky)) {
        value->fu_rest = _FP_REST_ABOVE;
    } else if (extra == _FP_HEX_TOP) {
        value->fu_rest = _FP_REST_HALF;
    } else if (extra != 0 || sticky) {
        value->fu_rest = _FP_REST_BELOW;
    }
    return i;
}

// Drop the low bits of value's mantissa, keeping what they were against half.
void _Fp_DropBits(_Fp_Unrounded *value, long bits)
{
    unsigned long long mant = value->fu_mant;
    _Bool rest = value->fu_rest != _FP_REST_NONE;
    _Bool round;
    _Bool lower;

    if (bits > _FP_MANT_BITS) {
        value->fu_mant = 0;
        value->fu_rest = mant != 0 || rest ? _FP_REST_BELOW : _FP_REST_NONE;
        return;
    }
    round = mant >> (bits - 1) & 1;
    lower = (bits > 1 && (mant & ~0ULL >> (_FP_MANT_BITS + 1 - bits)) != 0) || rest;
    value->fu_mant = bits == _FP_MANT_BITS ? 0 : mant >> bits;
    if (round) {
        value->fu_rest = lower ? _FP_REST_ABOVE : _FP_REST_HALF;
    } else {
        value->fu_rest = lower ? _FP_REST_BELOW : _FP_REST_NONE;
    }
}

// Round value to fmt to nearest, ties to even, with ERANGE as glibc's.
void _Fp_RoundReal(const _Fp_Unrounded *value, const _Fp_Format *fmt, _Fp_Real *real)
{
    _Fp_Unrounded part = *value;
    unsigned long long ones = ~0ULL >> (_FP_MANT_BITS - fmt->ff_mant);
    _Bool tiny;

    real->fr_exp = 0;
    real->fr_mant = 0;
    if (part.fu_mant == 0) {
        return;
    }
    if (fmt->ff_mant < _FP_MANT_BITS) {
        _Fp_DropBits(&part, _FP_MANT_BITS - fmt->ff_mant);
    }
    tiny = part.fu_exp < fmt->ff_min && ! (part.fu_exp == fmt->ff_min - 1 && part.fu_mant == ones && part.fu_rest >= _FP_REST_HALF);
    if (part.fu_exp < fmt->ff_min) {
        _Fp_DropBits(&part, fmt->ff_min - part.fu_exp);
    }
    if (part.fu_rest > _FP_REST_HALF || (part.fu_rest == _FP_REST_HALF && (part.fu_mant & 1))) {
        part.fu_mant++;
        if (part.fu_exp >= fmt->ff_min && (part.fu_mant & ones) == 0) {
            part.fu_mant = 1ULL << (fmt->ff_mant - 1);
            part.fu_exp++;
        }
    }
    if (part.fu_exp > fmt->ff_max) {
        real->fr_exp = (unsigned int) (2 * fmt->ff_max + 1);
        real->fr_mant = fmt->ff_explicit ? 1ULL << (fmt->ff_mant - 1) : 0;
        _Err_Errno = _SYS_ERANGE;
        return;
    }
    if (part.fu_exp >= fmt->ff_min) {
        real->fr_exp = (unsigned int) (part.fu_exp + fmt->ff_max);
    } else {
        real->fr_exp = part.fu_mant >> (fmt->ff_mant - 1) != 0;
    }
    real->fr_mant = fmt->ff_explicit ? part.fu_mant : part.fu_mant & ones >> 1;
    if (tiny && part.fu_rest != _FP_REST_NONE) {
        _Err_Errno = _SYS_ERANGE;
    }
}

// Convert ch to lowercase if it is an uppercase letter of the "C" locale.
int _Fp_ToLower(int ch)
{
    return ch >= 'A' && ch <= 'Z' ? ch - 'A' + 'a' : ch;
}

// Convert the hex digit ch to its value, or _FP_HEX_LIMIT for none.
int _Fp_ToHexDigit(int ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return _FP_HEX_LIMIT;
}

// Convert a scanned decimal exactly to 64 bits and what lies below them.
void _Fp_ToUnrounded(const void *str, _Fp_Text text, int point, const _Fp_Decimal *dec, _Fp_Unrounded *value)
{
    _Bigint_Number num;
    _Bigint_Number den;
    unsigned long i = dec->fd_first;
    long count = dec->fd_count + dec->fd_sticky;
    long exp = dec->fd_exp - dec->fd_sticky;
    long top = dec->fd_count + dec->fd_exp;
    unsigned int chunk = 0;
    unsigned int scale = 1;
    long n;
    int ch;
    int bits;
    int cmp;

    value->fu_mant = (unsigned long long) 1 << (_FP_MANT_BITS - 1);
    value->fu_rest = _FP_REST_NONE;
    if (dec->fd_count == 0) {
        value->fu_mant = 0;
        return;
    }
    if (top > _FP_EXP10_MAX || top < _FP_EXP10_MIN) {
        value->fu_exp = top > 0 ? _FP_EXP_LIMIT : -_FP_EXP_LIMIT;
        return;
    }
    _Bigint_SetValue(&num, 0);
    for (n = 0; n < count; n++) {
        ch = n < dec->fd_count ? _Fp_ReadChar(str, text, i) : '1';
        i++;
        if (ch == point) {
            ch = _Fp_ReadChar(str, text, i);
            i++;
        }
        chunk = chunk * 10 + (unsigned int) (ch - '0');
        scale *= 10;
        if (scale == _FP_CHUNK_SCALE || n == count - 1) {
            _Bigint_MulAddWords(&num, scale, chunk);
            chunk = 0;
            scale = 1;
        }
    }
    _Bigint_SetValue(&den, 1);
    if (exp >= 0) {
        _Bigint_MulPow5(&num, (int) exp);
    } else {
        _Bigint_MulPow5(&den, (int) -exp);
    }
    bits = _Bigint_CountBits(&num) - _Bigint_CountBits(&den);
    if (bits > 0) {
        _Bigint_ShiftBitsLeft(&den, bits);
    } else {
        _Bigint_ShiftBitsLeft(&num, -bits);
    }
    if (_Bigint_CompareNumbers(&num, &den) < 0) {
        _Bigint_ShiftBitsLeft(&num, 1);
        bits--;
    }
    value->fu_exp = exp + bits;
    _Bigint_ShiftBitsLeft(&num, _FP_MANT_BITS - 1);
    value->fu_mant = _Bigint_DivideNumber(&num, &den);
    if (num.bn_len != 0) {
        _Bigint_ShiftBitsLeft(&num, 1);
        cmp = _Bigint_CompareNumbers(&num, &den);
        value->fu_rest = cmp < 0 ? _FP_REST_BELOW : cmp == 0 ? _FP_REST_HALF : _FP_REST_ABOVE;
    }
}

// Convert the number at the start of str to fmt's fields, and give its end.
unsigned long _Fp_ToReal(const void *str, _Fp_Text text, const _Fp_Format *fmt, _Fp_Real *real)
{
    _Fp_Unrounded value;
    _Fp_Decimal dec;
    unsigned long long payload;
    int point = _FP_POINT;
    unsigned long i = 0;
    unsigned long end;
    int ch;

    real->fr_negative = 0;
    real->fr_exp = (unsigned int) (2 * fmt->ff_max + 1);
    real->fr_mant = fmt->ff_explicit ? 1ULL << (fmt->ff_mant - 1) : 0;
    while (_Fp_IsSpace(_Fp_ReadChar(str, text, i))) {
        i++;
    }
    ch = _Fp_ReadChar(str, text, i);
    if (ch == '+' || ch == '-') {
        real->fr_negative = ch == '-';
        i++;
    }
    if (_Fp_MatchWord(str, text, i, "inf")) {
        end = _Fp_MatchWord(str, text, i, "infinity");
        return i + (end != 0 ? end : _Fp_MatchWord(str, text, i, "inf"));
    }
    if (_Fp_MatchWord(str, text, i, "nan")) {
        end = _Fp_ReadNotANumber(str, text, i + _Fp_MatchWord(str, text, i, "nan"), &payload);
        real->fr_mant |= 1ULL << (fmt->ff_mant - 2) | (payload & ~0ULL >> (_FP_MANT_BITS + 1 - fmt->ff_mant));
        return end;
    }
    ch = _Fp_ReadChar(str, text, i + 1);
    if (_Fp_ReadChar(str, text, i) == '0' && (ch == 'x' || ch == 'X')
        && (_Fp_ToHexDigit(_Fp_ReadChar(str, text, i + 2)) < _FP_HEX_LIMIT
            || (_Fp_ReadChar(str, text, i + 2) == point && _Fp_ToHexDigit(_Fp_ReadChar(str, text, i + 3)) < _FP_HEX_LIMIT))) {
        end = _Fp_ReadHex(str, text, i + 2, point, &value);
    } else {
        end = _Fp_ScanDecimal(str, text, i, point, &dec);
        if (end == i) {
            real->fr_negative = 0;
            real->fr_exp = 0;
            real->fr_mant = 0;
            return 0;
        }
        _Fp_ToUnrounded(str, text, point, &dec, &value);
    }
    _Fp_RoundReal(&value, fmt, real);
    return end;
}


// Convert the number at the start of str to a float, and give its end.
float _Fp_ToFloat(const void *str, _Fp_Text text, unsigned long *end)
{
    _Fp_Real real;
    float value;
    unsigned int bits;

    *end = _Fp_ToReal(str, text, &_Fp_FloatFormat, &real);
    bits = real.fr_exp << (_FP_FLOAT_MANT - 1) | (unsigned int) real.fr_mant;
    if (real.fr_negative) {
        bits |= _FP_FLOAT_SIGN;
    }
    _Str_CopyBytes(&value, &bits, sizeof(value));
    return value;
}

// Convert the number at the start of str to a double, and give its end.
double _Fp_ToDouble(const void *str, _Fp_Text text, unsigned long *end)
{
    _Fp_Real real;
    double value;
    unsigned long long bits;

    *end = _Fp_ToReal(str, text, &_Fp_DoubleFormat, &real);
    bits = (unsigned long long) real.fr_exp << (_FP_DOUBLE_MANT - 1) | real.fr_mant;
    if (real.fr_negative) {
        bits |= _FP_DOUBLE_SIGN;
    }
    _Str_CopyBytes(&value, &bits, sizeof(value));
    return value;
}

// Convert the number at the start of str to a long double, and give its end.
long double _Fp_ToLongDouble(const void *str, _Fp_Text text, unsigned long *end)
{
    _Fp_Real real;
    unsigned short top;
    long double value = 0;

    *end = _Fp_ToReal(str, text, &_Fp_LongDoubleFormat, &real);
    top = (unsigned short) real.fr_exp;
    if (real.fr_negative) {
        top |= _FP_LDOUBLE_SIGN;
    }
    _Str_CopyBytes(&value, &real.fr_mant, sizeof(real.fr_mant));
    _Str_CopyBytes((char *) &value + sizeof(real.fr_mant), &top, sizeof(top));
    return value;
}
