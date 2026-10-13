/*
 * C source file for general utilities.
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
#include <stdlib.h>

// The error a number out of range reports.
#include <errno.h>

// Copying memory.
#include <string.h>

// The heap malloc and its family allocate from.
#include <_mem.h>

// The signal mask abort clears and the exit.
#include <_sys.h>

// The environment getenv searches, and the hook exit flushes the streams by.
#include <_crt.h>

// The signal abort ends the program by.
#include <signal.h>

// The exact arithmetic strtod rounds by.
#include <_bigint.h>

// The floating types strtod converts to.
#include <float.h>

// The decimal point strtod reads.
#include <locale.h>

// The white space and letters strtod reads.
#include <ctype.h>

// The range of a character and an unsigned long long.
#include <limits.h>

// The integer conversions strtol and its family share.
#include <inttypes.h>

// The encoding the multibyte functions follow, and its coding.
#include <_utf8.h>

// Check the type _mem counts in against the header's.
typedef char _Stdlib_CheckSize[sizeof(unsigned long) == sizeof(size_t) ? 1 : -1];

// Check the type _utf8 holds a wide character in against the header's.
typedef char _Stdlib_CheckWchar[sizeof(int) == sizeof(wchar_t) && (wchar_t) -1 < 0 ? 1 : -1];

// Check that long and long long are intmax_t, as strtol and its family take.
typedef char _Stdlib_CheckLong[LONG_MAX == INTMAX_MAX && LLONG_MAX == INTMAX_MAX && ULONG_MAX == UINTMAX_MAX && ULLONG_MAX == UINTMAX_MAX ? 1 : -1];

// The format of float, its exponents one below C's.
const _Stdlib_Format _Stdlib_FloatFormat = {
    .sf_mant     = FLT_MANT_DIG,
    .sf_min      = FLT_MIN_EXP - 1,
    .sf_max      = FLT_MAX_EXP - 1,
    .sf_explicit = 0
};

// The format of double, its exponents one below C's.
const _Stdlib_Format _Stdlib_DoubleFormat = {
    .sf_mant     = DBL_MANT_DIG,
    .sf_min      = DBL_MIN_EXP - 1,
    .sf_max      = DBL_MAX_EXP - 1,
    .sf_explicit = 0
};

// The format of long double, its exponents one below C's, its leading bit kept.
const _Stdlib_Format _Stdlib_LongDoubleFormat = {
    .sf_mant     = LDBL_MANT_DIG,
    .sf_min      = LDBL_MIN_EXP - 1,
    .sf_max      = LDBL_MAX_EXP - 1,
    .sf_explicit = 1
};

// The state of rand, its taps and whether srand has seeded it.
unsigned int _Stdlib_RandState[_STDLIB_RAND_WORDS];
int _Stdlib_RandFront;
int _Stdlib_RandRear;
_Bool _Stdlib_RandSeeded;

// The first block of atexit's, C99's 32 without malloc, and the newest.
_Stdlib_Exits _Stdlib_ExitBase;
_Stdlib_Exits *_Stdlib_ExitTop = &_Stdlib_ExitBase;

// Read character index of str, a wide string or a narrow one.
int _Stdlib_ReadChar(const void *str, _Stdlib_Text text, size_t index)
{
    if (text == _STDLIB_TEXT_WIDE) {
        return ((const wchar_t *) str)[index];
    }
    return ((const unsigned char *) str)[index];
}

// Check whether ch is white space in the "C" locale.
_Bool _Stdlib_IsSpace(int ch)
{
    return ch >= 0 && ch <= UCHAR_MAX && isspace(ch);
}

// Convert the hex digit ch to its value, or _STDLIB_HEX_LIMIT for none.
int _Stdlib_ConvertHexDigit(int ch)
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
    return _STDLIB_HEX_LIMIT;
}

// Match word at index of str in any case, and give its length, or 0.
size_t _Stdlib_MatchWord(const void *str, _Stdlib_Text text, size_t index, const char *word)
{
    size_t i;
    int ch;

    for (i = 0; word[i] != '\0'; i++) {
        ch = _Stdlib_ReadChar(str, text, index + i);
        if (ch < 0 || ch > UCHAR_MAX || tolower(ch) != word[i]) {
            return 0;
        }
    }
    return i;
}

// Read an exponent after letter at index into *exp, and give the index past it.
size_t _Stdlib_ReadExponent(const void *str, _Stdlib_Text text, size_t index, int letter, long *exp)
{
    size_t i = index + 1;
    _Bool negative = 0;
    long value = 0;
    int ch = _Stdlib_ReadChar(str, text, index);

    *exp = 0;
    if (ch != letter && ch != toupper(letter)) {
        return index;
    }
    ch = _Stdlib_ReadChar(str, text, i);
    if (ch == '+' || ch == '-') {
        negative = ch == '-';
        i++;
    }
    ch = _Stdlib_ReadChar(str, text, i);
    if (ch < '0' || ch > '9') {
        return index;
    }
    for (; ch >= '0' && ch <= '9'; ch = _Stdlib_ReadChar(str, text, i)) {
        if (value < _STDLIB_EXP_LIMIT) {
            value = value * 10 + (ch - '0');
        }
        i++;
    }
    *exp = negative ? -value : value;
    return i;
}

// Read a NaN's payload up to stop as strtoull in base 0, or 0, as glibc's.
unsigned long long _Stdlib_ReadPayload(const void *str, _Stdlib_Text text, size_t index, size_t stop)
{
    unsigned long long value = 0;
    int base = 10;
    int digit;

    if (_Stdlib_ReadChar(str, text, index) == '0') {
        base = 8;
        if (tolower(_Stdlib_ReadChar(str, text, index + 1)) == 'x' && _Stdlib_ConvertHexDigit(_Stdlib_ReadChar(str, text, index + 2)) < _STDLIB_HEX_LIMIT) {
            base = _STDLIB_HEX_LIMIT;
            index += 2;
        }
    }
    for (; index < stop; index++) {
        digit = _Stdlib_ConvertHexDigit(_Stdlib_ReadChar(str, text, index));
        if (digit >= base) {
            return 0;
        }
        value = value > (ULLONG_MAX - (unsigned long long) digit) / (unsigned long long) base ? ULLONG_MAX : value * (unsigned long long) base + (unsigned long long) digit;
    }
    return value;
}

// Read a NaN's parenthesized characters at index, and give the index past them.
size_t _Stdlib_ReadNotANumber(const void *str, _Stdlib_Text text, size_t index, unsigned long long *payload)
{
    size_t i = index + 1;
    int ch;

    *payload = 0;
    if (_Stdlib_ReadChar(str, text, index) != '(') {
        return index;
    }
    for (ch = _Stdlib_ReadChar(str, text, i); ch == '_' || (ch >= 0 && ch <= UCHAR_MAX && isalnum(ch)); ch = _Stdlib_ReadChar(str, text, i)) {
        i++;
    }
    if (ch != ')') {
        return index;
    }
    *payload = _Stdlib_ReadPayload(str, text, index + 1, i);
    return i + 1;
}

// Scan a decimal's digits and exponent at index, and give the index past them.
size_t _Stdlib_ScanDecimal(const void *str, _Stdlib_Text text, size_t index, int point, _Stdlib_Decimal *dec)
{
    size_t i;
    _Bool digits = 0;
    _Bool fraction = 0;
    long exp;
    int ch;

    dec->sd_first = 0;
    dec->sd_count = 0;
    dec->sd_exp = 0;
    dec->sd_sticky = 0;
    for (i = index; ; i++) {
        ch = _Stdlib_ReadChar(str, text, i);
        if (ch == point && ! fraction) {
            fraction = 1;
            continue;
        }
        if (ch < '0' || ch > '9') {
            break;
        }
        digits = 1;
        if (dec->sd_count == 0 && ch == '0') {
            dec->sd_exp -= fraction;
            continue;
        }
        if (dec->sd_count == 0) {
            dec->sd_first = i;
        }
        if (dec->sd_count < _STDLIB_DIGITS) {
            dec->sd_count++;
            dec->sd_exp -= fraction;
        } else {
            dec->sd_sticky |= ch != '0';
            dec->sd_exp += ! fraction;
        }
    }
    if (! digits) {
        return index;
    }
    i = _Stdlib_ReadExponent(str, text, i, 'e', &exp);
    dec->sd_exp += exp;
    return i;
}

// Convert a scanned decimal exactly to 64 bits and what lies below them.
void _Stdlib_ConvertDecimal(const void *str, _Stdlib_Text text, int point, const _Stdlib_Decimal *dec, _Stdlib_Unrounded *value)
{
    _Bigint_Number num;
    _Bigint_Number den;
    size_t i = dec->sd_first;
    long count = dec->sd_count + dec->sd_sticky;
    long exp = dec->sd_exp - dec->sd_sticky;
    long top = dec->sd_count + dec->sd_exp;
    unsigned int chunk = 0;
    unsigned int scale = 1;
    long n;
    int ch;
    int bits;
    int cmp;

    value->su_mant = (unsigned long long) 1 << (_STDLIB_MANT_BITS - 1);
    value->su_rest = _STDLIB_REST_NONE;
    if (dec->sd_count == 0) {
        value->su_mant = 0;
        return;
    }
    if (top > _STDLIB_EXP10_MAX || top < _STDLIB_EXP10_MIN) {
        value->su_exp = top > 0 ? _STDLIB_EXP_LIMIT : -_STDLIB_EXP_LIMIT;
        return;
    }
    _Bigint_SetValue(&num, 0);
    for (n = 0; n < count; n++) {
        ch = n < dec->sd_count ? _Stdlib_ReadChar(str, text, i) : '1';
        i++;
        if (ch == point) {
            ch = _Stdlib_ReadChar(str, text, i);
            i++;
        }
        chunk = chunk * 10 + (unsigned int) (ch - '0');
        scale *= 10;
        if (scale == _STDLIB_CHUNK_SCALE || n == count - 1) {
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
    value->su_exp = exp + bits;
    _Bigint_ShiftBitsLeft(&num, _STDLIB_MANT_BITS - 1);
    value->su_mant = _Bigint_DivideNumber(&num, &den);
    if (num.bn_len != 0) {
        _Bigint_ShiftBitsLeft(&num, 1);
        cmp = _Bigint_CompareNumbers(&num, &den);
        value->su_rest = cmp < 0 ? _STDLIB_REST_BELOW : cmp == 0 ? _STDLIB_REST_HALF : _STDLIB_REST_ABOVE;
    }
}

// Read a hexadecimal's digits and exponent at index, and give the end.
size_t _Stdlib_ReadHex(const void *str, _Stdlib_Text text, size_t index, int point, _Stdlib_Unrounded *value)
{
    unsigned long long mant = 0;
    unsigned int extra = 0;
    _Bool sticky = 0;
    _Bool fraction = 0;
    long count = 0;
    long exp = 0;
    long power;
    size_t i;
    int ch;
    int digit;

    for (i = index; ; i++) {
        ch = _Stdlib_ReadChar(str, text, i);
        if (ch == point && ! fraction) {
            fraction = 1;
            continue;
        }
        digit = _Stdlib_ConvertHexDigit(ch);
        if (digit == _STDLIB_HEX_LIMIT) {
            break;
        }
        if (count == 0 && digit == 0) {
            exp -= fraction * _STDLIB_HEX_BITS;
            continue;
        }
        if (count < _STDLIB_MANT_BITS / _STDLIB_HEX_BITS) {
            mant = mant << _STDLIB_HEX_BITS | (unsigned long long) digit;
            exp -= fraction * _STDLIB_HEX_BITS;
        } else {
            if (count == _STDLIB_MANT_BITS / _STDLIB_HEX_BITS) {
                extra = (unsigned int) digit;
            } else {
                sticky |= digit != 0;
            }
            exp += ! fraction * _STDLIB_HEX_BITS;
        }
        count++;
    }
    i = _Stdlib_ReadExponent(str, text, i, 'p', &power);
    value->su_mant = mant;
    value->su_exp = exp + power + _STDLIB_MANT_BITS - 1;
    value->su_rest = _STDLIB_REST_NONE;
    if (mant == 0) {
        return i;
    }
    while (mant >> (_STDLIB_MANT_BITS - 1) == 0) {
        mant = mant << 1 | extra >> (_STDLIB_HEX_BITS - 1);
        extra = extra << 1 & (_STDLIB_HEX_LIMIT - 1);
        value->su_exp--;
    }
    value->su_mant = mant;
    if (extra > _STDLIB_HEX_TOP || (extra == _STDLIB_HEX_TOP && sticky)) {
        value->su_rest = _STDLIB_REST_ABOVE;
    } else if (extra == _STDLIB_HEX_TOP) {
        value->su_rest = _STDLIB_REST_HALF;
    } else if (extra != 0 || sticky) {
        value->su_rest = _STDLIB_REST_BELOW;
    }
    return i;
}

// Drop the low bits of value's mantissa, keeping what they were against half.
void _Stdlib_DropBits(_Stdlib_Unrounded *value, long bits)
{
    unsigned long long mant = value->su_mant;
    _Bool rest = value->su_rest != _STDLIB_REST_NONE;
    _Bool round;
    _Bool lower;

    if (bits > _STDLIB_MANT_BITS) {
        value->su_mant = 0;
        value->su_rest = mant != 0 || rest ? _STDLIB_REST_BELOW : _STDLIB_REST_NONE;
        return;
    }
    round = mant >> (bits - 1) & 1;
    lower = (bits > 1 && (mant & ~0ULL >> (_STDLIB_MANT_BITS + 1 - bits)) != 0) || rest;
    value->su_mant = bits == _STDLIB_MANT_BITS ? 0 : mant >> bits;
    if (round) {
        value->su_rest = lower ? _STDLIB_REST_ABOVE : _STDLIB_REST_HALF;
    } else {
        value->su_rest = lower ? _STDLIB_REST_BELOW : _STDLIB_REST_NONE;
    }
}

// Round value to fmt to nearest, ties to even, with ERANGE as glibc's.
void _Stdlib_RoundReal(const _Stdlib_Unrounded *value, const _Stdlib_Format *fmt, _Stdlib_Real *real)
{
    _Stdlib_Unrounded part = *value;
    unsigned long long ones = ~0ULL >> (_STDLIB_MANT_BITS - fmt->sf_mant);
    _Bool tiny;

    real->sr_exp = 0;
    real->sr_mant = 0;
    if (part.su_mant == 0) {
        return;
    }
    if (fmt->sf_mant < _STDLIB_MANT_BITS) {
        _Stdlib_DropBits(&part, _STDLIB_MANT_BITS - fmt->sf_mant);
    }
    tiny = part.su_exp < fmt->sf_min && ! (part.su_exp == fmt->sf_min - 1 && part.su_mant == ones && part.su_rest >= _STDLIB_REST_HALF);
    if (part.su_exp < fmt->sf_min) {
        _Stdlib_DropBits(&part, fmt->sf_min - part.su_exp);
    }
    if (part.su_rest > _STDLIB_REST_HALF || (part.su_rest == _STDLIB_REST_HALF && (part.su_mant & 1))) {
        part.su_mant++;
        if (part.su_exp >= fmt->sf_min && (part.su_mant & ones) == 0) {
            part.su_mant = 1ULL << (fmt->sf_mant - 1);
            part.su_exp++;
        }
    }
    if (part.su_exp > fmt->sf_max) {
        real->sr_exp = (unsigned int) (2 * fmt->sf_max + 1);
        real->sr_mant = fmt->sf_explicit ? 1ULL << (fmt->sf_mant - 1) : 0;
        errno = ERANGE;
        return;
    }
    if (part.su_exp >= fmt->sf_min) {
        real->sr_exp = (unsigned int) (part.su_exp + fmt->sf_max);
    } else {
        real->sr_exp = part.su_mant >> (fmt->sf_mant - 1) != 0;
    }
    real->sr_mant = fmt->sf_explicit ? part.su_mant : part.su_mant & ones >> 1;
    if (tiny && part.su_rest != _STDLIB_REST_NONE) {
        errno = ERANGE;
    }
}

// Convert the number at the start of str to fmt's fields, and give its end.
size_t _Stdlib_ConvertReal(const void *str, _Stdlib_Text text, const _Stdlib_Format *fmt, _Stdlib_Real *real)
{
    _Stdlib_Unrounded value;
    _Stdlib_Decimal dec;
    unsigned long long payload;
    int point = (unsigned char) localeconv()->decimal_point[0];
    size_t i = 0;
    size_t end;
    int ch;

    real->sr_negative = 0;
    real->sr_exp = (unsigned int) (2 * fmt->sf_max + 1);
    real->sr_mant = fmt->sf_explicit ? 1ULL << (fmt->sf_mant - 1) : 0;
    while (_Stdlib_IsSpace(_Stdlib_ReadChar(str, text, i))) {
        i++;
    }
    ch = _Stdlib_ReadChar(str, text, i);
    if (ch == '+' || ch == '-') {
        real->sr_negative = ch == '-';
        i++;
    }
    if (_Stdlib_MatchWord(str, text, i, "inf")) {
        end = _Stdlib_MatchWord(str, text, i, "infinity");
        return i + (end != 0 ? end : _Stdlib_MatchWord(str, text, i, "inf"));
    }
    if (_Stdlib_MatchWord(str, text, i, "nan")) {
        end = _Stdlib_ReadNotANumber(str, text, i + _Stdlib_MatchWord(str, text, i, "nan"), &payload);
        real->sr_mant |= 1ULL << (fmt->sf_mant - 2) | (payload & ~0ULL >> (_STDLIB_MANT_BITS + 1 - fmt->sf_mant));
        return end;
    }
    ch = _Stdlib_ReadChar(str, text, i + 1);
    if (_Stdlib_ReadChar(str, text, i) == '0' && (ch == 'x' || ch == 'X')
        && (_Stdlib_ConvertHexDigit(_Stdlib_ReadChar(str, text, i + 2)) < _STDLIB_HEX_LIMIT
            || (_Stdlib_ReadChar(str, text, i + 2) == point && _Stdlib_ConvertHexDigit(_Stdlib_ReadChar(str, text, i + 3)) < _STDLIB_HEX_LIMIT))) {
        end = _Stdlib_ReadHex(str, text, i + 2, point, &value);
    } else {
        end = _Stdlib_ScanDecimal(str, text, i, point, &dec);
        if (end == i) {
            real->sr_negative = 0;
            real->sr_exp = 0;
            real->sr_mant = 0;
            return 0;
        }
        _Stdlib_ConvertDecimal(str, text, point, &dec, &value);
    }
    _Stdlib_RoundReal(&value, fmt, real);
    return end;
}

// Pop the function atexit registered last, or return NULL when none is left.
void (*_Stdlib_PopExit(void))(void)
{
    _Stdlib_Exits *block = _Stdlib_ExitTop;

    while (block->se_count == 0 && block->se_next) {
        _Stdlib_ExitTop = block->se_next;
        free(block);
        block = _Stdlib_ExitTop;
    }
    if (block->se_count == 0) {
        return NULL;
    }
    block->se_count--;
    return block->se_func[block->se_count];
}

// Swap the size bytes at left with those at right.
void _Stdlib_SwapBytes(char *left, char *right, size_t size)
{
    char ch;
    size_t i;

    for (i = 0; i < size; i++) {
        ch = left[i];
        left[i] = right[i];
        right[i] = ch;
    }
}

// Reverse the order of nmemb elements of size bytes at base.
void _Stdlib_ReverseElements(char *base, size_t nmemb, size_t size)
{
    size_t i;

    for (i = 0; i < nmemb / 2; i++) {
        _Stdlib_SwapBytes(base + i * size, base + (nmemb - 1 - i) * size, size);
    }
}

// Rotate nmemb elements of size bytes at base so that element first leads.
void _Stdlib_RotateElements(char *base, size_t nmemb, size_t first, size_t size)
{
    _Stdlib_ReverseElements(base, first, size);
    _Stdlib_ReverseElements(base + first * size, nmemb - first, size);
    _Stdlib_ReverseElements(base, nmemb, size);
}

// Find the index of the first of nmemb sorted elements not below key.
size_t _Stdlib_FindLowerBound(const char *base, size_t nmemb, size_t size, const void *key, int (*compar)(const void *, const void *))
{
    size_t low = 0;
    size_t mid;
    size_t high = nmemb;

    while (low < high) {
        mid = low + (high - low) / 2;
        if (compar(base + mid * size, key) < 0) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return low;
}

// Find the index of the first of nmemb sorted elements above key.
size_t _Stdlib_FindUpperBound(const char *base, size_t nmemb, size_t size, const void *key, int (*compar)(const void *, const void *))
{
    size_t low = 0;
    size_t mid;
    size_t high = nmemb;

    while (low < high) {
        mid = low + (high - low) / 2;
        if (compar(key, base + mid * size) < 0) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }
    return low;
}

// Merge the sorted runs of left and right elements at base without a buffer.
void _Stdlib_MergeRunsInPlace(char *base, size_t left, size_t right, size_t size, int (*compar)(const void *, const void *))
{
    size_t cut1;
    size_t cut2;

    if (left == 0 || right == 0) {
        return;
    }
    if (left + right == 2) {
        if (compar(base + size, base) < 0) {
            _Stdlib_SwapBytes(base, base + size, size);
        }
        return;
    }
    if (left >= right) {
        cut1 = left / 2;
        cut2 = _Stdlib_FindLowerBound(base + left * size, right, size, base + cut1 * size, compar);
    } else {
        cut2 = right / 2;
        cut1 = _Stdlib_FindUpperBound(base, left, size, base + (left + cut2) * size, compar);
    }
    _Stdlib_RotateElements(base + cut1 * size, left - cut1 + cut2, left - cut1, size);
    _Stdlib_MergeRunsInPlace(base, cut1, cut2, size, compar);
    _Stdlib_MergeRunsInPlace(base + (cut1 + cut2) * size, left - cut1, right - cut2, size, compar);
}

// Merge the sorted runs of left and right elements at base through tmp.
void _Stdlib_MergeRuns(char *base, size_t left, size_t right, size_t size, int (*compar)(const void *, const void *), char *tmp)
{
    char *out = base;
    char *end = tmp + left * size;
    char *iter = tmp;
    char *next = base + left * size;
    char *stop = next + right * size;

    memcpy(tmp, base, left * size);
    while (iter < end && next < stop) {
        if (compar(next, iter) < 0) {
            memcpy(out, next, size);
            next += size;
        } else {
            memcpy(out, iter, size);
            iter += size;
        }
        out += size;
    }
    memcpy(out, iter, (size_t) (end - iter));
}

// Sort nmemb elements of size bytes at base stably, merging through tmp if any.
void _Stdlib_SortElements(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *), char *tmp)
{
    size_t half = nmemb / 2;
    char *mid = base + half * size;

    if (nmemb < 2) {
        return;
    }
    _Stdlib_SortElements(base, half, size, compar, tmp);
    _Stdlib_SortElements(mid, nmemb - half, size, compar, tmp);
    if (compar(mid, mid - size) >= 0) {
        return;
    }
    if (tmp) {
        _Stdlib_MergeRuns(base, half, nmemb - half, size, compar, tmp);
    } else {
        _Stdlib_MergeRunsInPlace(base, half, nmemb - half, size, compar);
    }
}

// Get the bytes of the longest character in LC_CTYPE's encoding.
size_t _Stdlib_GetMbCurMax(void)
{
    return _Utf8_GetCharMax();
}

// Convert the start of the string nptr to a double, as strtod does.
double atof(const char *nptr)
{
    return strtod(nptr, NULL);
}

// Convert the start of the string nptr to an int, as strtol in base 10 does.
int atoi(const char *nptr)
{
    return (int) strtol(nptr, NULL, 10);
}

// Convert the start of the string nptr to a long, as strtol in base 10 does.
long atol(const char *nptr)
{
    return strtol(nptr, NULL, 10);
}

// Convert the start of the string nptr to a long long, as strtoll does.
long long atoll(const char *nptr)
{
    return strtoll(nptr, NULL, 10);
}

// Convert the start of the string nptr to a double, correctly rounded.
double strtod(const char *restrict nptr, char **restrict endptr)
{
    _Stdlib_Real real;
    size_t end = _Stdlib_ConvertReal(nptr, _STDLIB_TEXT_NARROW, &_Stdlib_DoubleFormat, &real);
    unsigned long long bits = (unsigned long long) real.sr_exp << (DBL_MANT_DIG - 1) | real.sr_mant;
    double value;

    if (real.sr_negative) {
        bits |= _STDLIB_DOUBLE_SIGN;
    }
    if (endptr != NULL) {
        *endptr = (char *) nptr + end;
    }
    memcpy(&value, &bits, sizeof(value));
    return value;
}

// Convert the start of the string nptr to a float, correctly rounded.
float strtof(const char *restrict nptr, char **restrict endptr)
{
    _Stdlib_Real real;
    size_t end = _Stdlib_ConvertReal(nptr, _STDLIB_TEXT_NARROW, &_Stdlib_FloatFormat, &real);
    unsigned int bits = real.sr_exp << (FLT_MANT_DIG - 1) | (unsigned int) real.sr_mant;
    float value;

    if (real.sr_negative) {
        bits |= _STDLIB_FLOAT_SIGN;
    }
    if (endptr != NULL) {
        *endptr = (char *) nptr + end;
    }
    memcpy(&value, &bits, sizeof(value));
    return value;
}

// Convert the start of the string nptr to a long double, correctly rounded.
long double strtold(const char *restrict nptr, char **restrict endptr)
{
    _Stdlib_Real real;
    size_t end = _Stdlib_ConvertReal(nptr, _STDLIB_TEXT_NARROW, &_Stdlib_LongDoubleFormat, &real);
    unsigned short top = (unsigned short) real.sr_exp;
    long double value = 0;

    if (real.sr_negative) {
        top |= _STDLIB_LDOUBLE_SIGN;
    }
    if (endptr != NULL) {
        *endptr = (char *) nptr + end;
    }
    memcpy(&value, &real.sr_mant, sizeof(real.sr_mant));
    memcpy((char *) &value + sizeof(real.sr_mant), &top, sizeof(top));
    return value;
}

// Convert the start of the string nptr to a long in base.
long strtol(const char *restrict nptr, char **restrict endptr, int base)
{
    return strtoimax(nptr, endptr, base);
}

// Convert the start of the string nptr to a long long in base.
long long strtoll(const char *restrict nptr, char **restrict endptr, int base)
{
    return strtoimax(nptr, endptr, base);
}

// Convert the start of the string nptr to an unsigned long in base.
unsigned long strtoul(const char *restrict nptr, char **restrict endptr, int base)
{
    return strtoumax(nptr, endptr, base);
}

// Convert the start of the string nptr to an unsigned long long in base.
unsigned long long strtoull(const char *restrict nptr, char **restrict endptr, int base)
{
    return strtoumax(nptr, endptr, base);
}

// Give the next number of the sequence srand seeded, as glibc's.
int rand(void)
{
    unsigned int value;
    unsigned int *state = _Stdlib_RandState;

    if (! _Stdlib_RandSeeded) {
        srand(1);
    }
    state[_Stdlib_RandFront] += state[_Stdlib_RandRear];
    value = state[_Stdlib_RandFront];
    _Stdlib_RandFront = (_Stdlib_RandFront + 1) % _STDLIB_RAND_WORDS;
    _Stdlib_RandRear = (_Stdlib_RandRear + 1) % _STDLIB_RAND_WORDS;
    return (int) (value >> 1);
}

// Seed the sequence rand gives, 0 as 1, as glibc's.
void srand(unsigned int seed)
{
    int word = seed == 0 ? 1 : (int) seed;
    int i;

    _Stdlib_RandState[0] = (unsigned int) word;
    for (i = 1; i < _STDLIB_RAND_WORDS; i++) {
        word = _STDLIB_RAND_MUL * (word % _STDLIB_RAND_QUO) - _STDLIB_RAND_REM * (word / _STDLIB_RAND_QUO);
        if (word < 0) {
            word += _STDLIB_RAND_MOD;
        }
        _Stdlib_RandState[i] = (unsigned int) word;
    }
    _Stdlib_RandFront = _STDLIB_RAND_SEP;
    _Stdlib_RandRear = 0;
    _Stdlib_RandSeeded = 1;
    for (i = 0; i < _STDLIB_RAND_DISCARD; i++) {
        rand();
    }
}

// Allocate nmemb objects of size bytes, all bits zero.
void *calloc(size_t nmemb, size_t size)
{
    return _Mem_AllocateObjects(nmemb, size);
}

// Free the memory at ptr, which malloc, calloc or realloc gave.
void free(void *ptr)
{
    _Mem_FreeMemory(ptr);
}

// Allocate size bytes, or return NULL with errno ENOMEM, as glibc does.
void *malloc(size_t size)
{
    return _Mem_AllocateMemory(size);
}

// Resize the memory at ptr to size bytes; a size of 0 frees it, as glibc's.
void *realloc(void *ptr, size_t size)
{
    return _Mem_ResizeMemory(ptr, size);
}

// End the program by SIGABRT past a returning handler or a block, as glibc's.
void abort(void)
{
    unsigned long set = 1UL << (SIGABRT - _SYS_SIGNAL_FIRST);

    raise(SIGABRT);
    signal(SIGABRT, SIG_DFL);
    _Sys_InvokeRtSigprocmask(_SYS_SIG_UNBLOCK, &set, NULL);
    raise(SIGABRT);
    _Sys_InvokeExitGroup(_STDLIB_ABORT_STATUS);
}

// Register func for exit to call, or return -1 out of memory, as glibc's.
int atexit(void (*func)(void))
{
    _Stdlib_Exits *block = _Stdlib_ExitTop;

    if (block->se_count == _STDLIB_EXITS) {
        block = malloc(sizeof(*block));
        if (! block) {
            return -1;
        }
        block->se_next = _Stdlib_ExitTop;
        block->se_count = 0;
        _Stdlib_ExitTop = block;
    }
    block->se_func[block->se_count] = func;
    block->se_count++;
    return 0;
}

// Call atexit's functions, newest first, then flush the streams and end.
void exit(int status)
{
    void (*func)(void);

    while ((func = _Stdlib_PopExit()) != NULL) {
        func();
    }
    if (_Crt_FlushStreams != NULL) {
        _Crt_FlushStreams();
    }
    _Exit(status);
}

// End the program with status, calling nothing atexit registered.
void _Exit(int status)
{
    _Sys_InvokeExitGroup(status);
}

// Return the value of the environment variable name, or NULL, as glibc's.
char *getenv(const char *name)
{
    size_t len = strlen(name);
    char **env;

    if (_Crt_Envp == NULL || len == 0) {
        return NULL;
    }
    for (env = _Crt_Envp; *env; env++) {
        if (strncmp(*env, name, len) == 0 && (*env)[len] == '=') {
            return *env + len + 1;
        }
    }
    return NULL;
}

// Report no command processor, so the emulator needs no fork or exec.
int system(const char *string)
{
    if (string == NULL) {
        return 0;
    }
    errno = _SYS_ENOSYS;
    return -1;
}

// Find an object of nmemb sorted at base that matches key, as glibc's does.
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    int cmp;
    size_t low = 0;
    size_t mid;
    size_t high = nmemb;
    const char *ptr;

    while (low < high) {
        mid = low + (high - low) / 2;
        ptr = (const char *) base + mid * size;
        cmp = compar(key, ptr);
        if (cmp == 0) {
            return (void *) ptr;
        }
        if (cmp < 0) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }
    return NULL;
}

// Sort nmemb objects of size bytes at base, keeping equal ones in order.
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    int saved = errno;
    size_t need = nmemb / 2 * size;
    char stack[_STDLIB_SORT_STACK];
    char *tmp = stack;

    if (need > sizeof(stack)) {
        tmp = malloc(need);
        errno = saved;
    }
    _Stdlib_SortElements(base, nmemb, size, compar, tmp);
    if (tmp != stack) {
        free(tmp);
    }
}

// Give the absolute value of j.
int abs(int j)
{
    return j < 0 ? -j : j;
}

// Give the absolute value of j.
long labs(long j)
{
    return j < 0 ? -j : j;
}

// Give the absolute value of j.
long long llabs(long long j)
{
    return j < 0 ? -j : j;
}

// Divide numer by denom, giving the quotient and the remainder.
div_t div(int numer, int denom)
{
    div_t result;

    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

// Divide numer by denom, giving the quotient and the remainder.
ldiv_t ldiv(long numer, long denom)
{
    ldiv_t result;

    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

// Divide numer by denom, giving the quotient and the remainder.
lldiv_t lldiv(long long numer, long long denom)
{
    lldiv_t result;

    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

// Give the length of the character in the n bytes at str, as mbtowc does.
int mblen(const char *str, size_t n)
{
    return mbtowc(NULL, str, n);
}

// Convert the character in the n bytes at str into *pwc, and give its length.
int mbtowc(wchar_t *restrict pwc, const char *restrict str, size_t n)
{
    int len;
    wchar_t wc;

    if (str == NULL) {
        return 0;
    }
    len = _Utf8_DecodeChar(str, n, &wc);
    if (len < 0) {
        if (len == _UTF8_INVALID) {
            errno = EILSEQ;
        }
        return -1;
    }
    if (pwc != NULL) {
        *pwc = wc;
    }
    return wc == 0 ? 0 : len;
}

// Convert wc to its bytes at str, and give their count, or -1 for none.
int wctomb(char *str, wchar_t wc)
{
    int len;

    if (str == NULL) {
        return 0;
    }
    len = _Utf8_EncodeChar(wc, str);
    if (len < 0) {
        errno = EILSEQ;
        return -1;
    }
    return len;
}

// Convert the multibyte string str to at most n wide characters at pwcs.
size_t mbstowcs(wchar_t *restrict pwcs, const char *restrict str, size_t n)
{
    int len;
    wchar_t wc;
    size_t count = 0;

    while (pwcs == NULL || count < n) {
        len = _Utf8_DecodeChar(str, _UTF8_MAX, &wc);
        if (len < 0) {
            errno = EILSEQ;
            return (size_t) -1;
        }
        if (pwcs != NULL) {
            pwcs[count] = wc;
        }
        if (wc == 0) {
            break;
        }
        str += len;
        count++;
    }
    return count;
}

// Convert the wide string pwcs to at most n bytes of a multibyte string at str.
size_t wcstombs(char *restrict str, const wchar_t *restrict pwcs, size_t n)
{
    int len;
    size_t count = 0;
    char buf[_UTF8_MAX];

    for (;; pwcs++) {
        if (str != NULL && count == n) {
            break;
        }
        len = _Utf8_EncodeChar(*pwcs, buf);
        if (len < 0) {
            errno = EILSEQ;
            return (size_t) -1;
        }
        if (str != NULL && (size_t) len > n - count) {
            break;
        }
        if (str != NULL) {
            memcpy(str + count, buf, (size_t) len);
        }
        if (*pwcs == 0) {
            break;
        }
        count += (size_t) len;
    }
    return count;
}
