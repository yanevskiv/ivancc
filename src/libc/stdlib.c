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

// The error the memory functions report.
#include <errno.h>

// Clearing and copying memory.
#include <string.h>

// The break the heap grows by, the signal mask abort clears and the exit.
#include <_sys.h>

// The environment getenv searches.
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

// Check that a block's header keeps the memory after it aligned.
typedef char _Stdlib_CheckBlock[sizeof(struct _Stdlib_Block) == _STDLIB_ALIGN && _STDLIB_BLOCK_MIN == 2 * _STDLIB_ALIGN ? 1 : -1];

// The formats of float, double and long double, exponents one below C's.
const struct _Stdlib_Format _Stdlib_FloatFormat = {FLT_MANT_DIG, FLT_MIN_EXP - 1, FLT_MAX_EXP - 1, 0};
const struct _Stdlib_Format _Stdlib_DoubleFormat = {DBL_MANT_DIG, DBL_MIN_EXP - 1, DBL_MAX_EXP - 1, 0};
const struct _Stdlib_Format _Stdlib_LongDoubleFormat = {LDBL_MANT_DIG, LDBL_MIN_EXP - 1, LDBL_MAX_EXP - 1, 1};

// The free blocks, in address order.
struct _Stdlib_Block *_Stdlib_FreeList;

// The first block of atexit's, C99's 32 without malloc, and the newest.
struct _Stdlib_Exits _Stdlib_ExitBase;
struct _Stdlib_Exits *_Stdlib_ExitTop = &_Stdlib_ExitBase;

// Read character index of str, a wide string when wide.
int _Stdlib_CharAt(const void *str, _Bool wide, size_t index)
{
    if (wide) {
        return ((const wchar_t *) str)[index];
    }
    return ((const unsigned char *) str)[index];
}

// Check whether ch is white space in the "C" locale.
_Bool _Stdlib_IsSpace(int ch)
{
    return ch >= 0 && ch <= UCHAR_MAX && isspace(ch);
}

// Give the value of the hexadecimal digit ch, or _STDLIB_HEX_LIMIT for none.
int _Stdlib_HexDigit(int ch)
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

// Give the length of word if str holds it at index in any case, or 0.
size_t _Stdlib_Match(const void *str, _Bool wide, size_t index, const char *word)
{
    size_t i;
    int ch;

    for (i = 0; word[i] != '\0'; i++) {
        ch = _Stdlib_CharAt(str, wide, index + i);
        if (ch < 0 || ch > UCHAR_MAX || tolower(ch) != word[i]) {
            return 0;
        }
    }
    return i;
}

// Read an exponent after letter at index into *exp, and give the index past it.
size_t _Stdlib_Exponent(const void *str, _Bool wide, size_t index, int letter, long *exp)
{
    size_t i = index + 1;
    _Bool negative = 0;
    long value = 0;
    int ch = _Stdlib_CharAt(str, wide, index);

    *exp = 0;
    if (ch != letter && ch != toupper(letter)) {
        return index;
    }
    ch = _Stdlib_CharAt(str, wide, i);
    if (ch == '+' || ch == '-') {
        negative = ch == '-';
        i++;
    }
    ch = _Stdlib_CharAt(str, wide, i);
    if (ch < '0' || ch > '9') {
        return index;
    }
    for (; ch >= '0' && ch <= '9'; ch = _Stdlib_CharAt(str, wide, i)) {
        if (value < _STDLIB_EXP_LIMIT) {
            value = value * 10 + (ch - '0');
        }
        i++;
    }
    *exp = negative ? -value : value;
    return i;
}

// Read a NaN's payload up to stop as strtoull in base 0, or 0, as glibc's.
unsigned long long _Stdlib_Payload(const void *str, _Bool wide, size_t index, size_t stop)
{
    unsigned long long value = 0;
    int base = 10;
    int digit;

    if (_Stdlib_CharAt(str, wide, index) == '0') {
        base = 8;
        if (tolower(_Stdlib_CharAt(str, wide, index + 1)) == 'x' && _Stdlib_HexDigit(_Stdlib_CharAt(str, wide, index + 2)) < _STDLIB_HEX_LIMIT) {
            base = _STDLIB_HEX_LIMIT;
            index += 2;
        }
    }
    for (; index < stop; index++) {
        digit = _Stdlib_HexDigit(_Stdlib_CharAt(str, wide, index));
        if (digit >= base) {
            return 0;
        }
        value = value > (ULLONG_MAX - (unsigned long long) digit) / (unsigned long long) base ? ULLONG_MAX : value * (unsigned long long) base + (unsigned long long) digit;
    }
    return value;
}

// Read a NaN's parenthesized characters at index, and give the index past them.
size_t _Stdlib_NotANumber(const void *str, _Bool wide, size_t index, unsigned long long *payload)
{
    size_t i = index + 1;
    int ch;

    *payload = 0;
    if (_Stdlib_CharAt(str, wide, index) != '(') {
        return index;
    }
    for (ch = _Stdlib_CharAt(str, wide, i); ch == '_' || (ch >= 0 && ch <= UCHAR_MAX && isalnum(ch)); ch = _Stdlib_CharAt(str, wide, i)) {
        i++;
    }
    if (ch != ')') {
        return index;
    }
    *payload = _Stdlib_Payload(str, wide, index + 1, i);
    return i + 1;
}

// Scan a decimal's digits and exponent at index, and give the index past them.
size_t _Stdlib_ScanDecimal(const void *str, _Bool wide, size_t index, int point, struct _Stdlib_Decimal *dec)
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
        ch = _Stdlib_CharAt(str, wide, i);
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
    i = _Stdlib_Exponent(str, wide, i, 'e', &exp);
    dec->sd_exp += exp;
    return i;
}

// Convert a scanned decimal exactly to 64 bits and what lies below them.
void _Stdlib_Decimal(const void *str, _Bool wide, int point, const struct _Stdlib_Decimal *dec, struct _Stdlib_Unrounded *value)
{
    struct _Bigint_Number num;
    struct _Bigint_Number den;
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
    _Bigint_Set(&num, 0);
    for (n = 0; n < count; n++) {
        ch = n < dec->sd_count ? _Stdlib_CharAt(str, wide, i) : '1';
        i++;
        if (ch == point) {
            ch = _Stdlib_CharAt(str, wide, i);
            i++;
        }
        chunk = chunk * 10 + (unsigned int) (ch - '0');
        scale *= 10;
        if (scale == _STDLIB_CHUNK_SCALE || n == count - 1) {
            _Bigint_MulAdd(&num, scale, chunk);
            chunk = 0;
            scale = 1;
        }
    }
    _Bigint_Set(&den, 1);
    if (exp >= 0) {
        _Bigint_MulPow5(&num, (int) exp);
    } else {
        _Bigint_MulPow5(&den, (int) -exp);
    }
    bits = _Bigint_Bits(&num) - _Bigint_Bits(&den);
    if (bits > 0) {
        _Bigint_ShiftLeft(&den, bits);
    } else {
        _Bigint_ShiftLeft(&num, -bits);
    }
    if (_Bigint_Compare(&num, &den) < 0) {
        _Bigint_ShiftLeft(&num, 1);
        bits--;
    }
    value->su_exp = exp + bits;
    _Bigint_ShiftLeft(&num, _STDLIB_MANT_BITS - 1);
    value->su_mant = _Bigint_Divide(&num, &den);
    if (num.bn_len != 0) {
        _Bigint_ShiftLeft(&num, 1);
        cmp = _Bigint_Compare(&num, &den);
        value->su_rest = cmp < 0 ? _STDLIB_REST_BELOW : cmp == 0 ? _STDLIB_REST_HALF : _STDLIB_REST_ABOVE;
    }
}

// Read a hexadecimal's digits and exponent at index, and give the end.
size_t _Stdlib_Hex(const void *str, _Bool wide, size_t index, int point, struct _Stdlib_Unrounded *value)
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
        ch = _Stdlib_CharAt(str, wide, i);
        if (ch == point && ! fraction) {
            fraction = 1;
            continue;
        }
        digit = _Stdlib_HexDigit(ch);
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
    i = _Stdlib_Exponent(str, wide, i, 'p', &power);
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
void _Stdlib_Shift(struct _Stdlib_Unrounded *value, long bits)
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
void _Stdlib_Round(const struct _Stdlib_Unrounded *value, const struct _Stdlib_Format *fmt, struct _Stdlib_Real *real)
{
    struct _Stdlib_Unrounded part = *value;
    unsigned long long ones = ~0ULL >> (_STDLIB_MANT_BITS - fmt->sf_mant);
    _Bool tiny;

    real->sr_exp = 0;
    real->sr_mant = 0;
    if (part.su_mant == 0) {
        return;
    }
    if (fmt->sf_mant < _STDLIB_MANT_BITS) {
        _Stdlib_Shift(&part, _STDLIB_MANT_BITS - fmt->sf_mant);
    }
    tiny = part.su_exp < fmt->sf_min && ! (part.su_exp == fmt->sf_min - 1 && part.su_mant == ones && part.su_rest >= _STDLIB_REST_HALF);
    if (part.su_exp < fmt->sf_min) {
        _Stdlib_Shift(&part, fmt->sf_min - part.su_exp);
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
size_t _Stdlib_ToReal(const void *str, _Bool wide, const struct _Stdlib_Format *fmt, struct _Stdlib_Real *real)
{
    struct _Stdlib_Unrounded value;
    struct _Stdlib_Decimal dec;
    unsigned long long payload;
    int point = (unsigned char) localeconv()->decimal_point[0];
    size_t i = 0;
    size_t end;
    int ch;

    real->sr_negative = 0;
    real->sr_exp = (unsigned int) (2 * fmt->sf_max + 1);
    real->sr_mant = fmt->sf_explicit ? 1ULL << (fmt->sf_mant - 1) : 0;
    while (_Stdlib_IsSpace(_Stdlib_CharAt(str, wide, i))) {
        i++;
    }
    ch = _Stdlib_CharAt(str, wide, i);
    if (ch == '+' || ch == '-') {
        real->sr_negative = ch == '-';
        i++;
    }
    if (_Stdlib_Match(str, wide, i, "inf")) {
        end = _Stdlib_Match(str, wide, i, "infinity");
        return i + (end != 0 ? end : _Stdlib_Match(str, wide, i, "inf"));
    }
    if (_Stdlib_Match(str, wide, i, "nan")) {
        end = _Stdlib_NotANumber(str, wide, i + _Stdlib_Match(str, wide, i, "nan"), &payload);
        real->sr_mant |= 1ULL << (fmt->sf_mant - 2) | (payload & ~0ULL >> (_STDLIB_MANT_BITS + 1 - fmt->sf_mant));
        return end;
    }
    ch = _Stdlib_CharAt(str, wide, i + 1);
    if (_Stdlib_CharAt(str, wide, i) == '0' && (ch == 'x' || ch == 'X')
        && (_Stdlib_HexDigit(_Stdlib_CharAt(str, wide, i + 2)) < _STDLIB_HEX_LIMIT
            || (_Stdlib_CharAt(str, wide, i + 2) == point && _Stdlib_HexDigit(_Stdlib_CharAt(str, wide, i + 3)) < _STDLIB_HEX_LIMIT))) {
        end = _Stdlib_Hex(str, wide, i + 2, point, &value);
    } else {
        end = _Stdlib_ScanDecimal(str, wide, i, point, &dec);
        if (end == i) {
            real->sr_negative = 0;
            real->sr_exp = 0;
            real->sr_mant = 0;
            return 0;
        }
        _Stdlib_Decimal(str, wide, point, &dec, &value);
    }
    _Stdlib_Round(&value, fmt, real);
    return end;
}

// Return the block size for size bytes, or 0 past PTRDIFF_MAX, as glibc's.
size_t _Stdlib_BlockSize(size_t size)
{
    size_t header = sizeof(struct _Stdlib_Block);
    size_t block;

    if (size > ((size_t) -1 >> 1) - header - _STDLIB_ALIGN) {
        return 0;
    }
    block = (size + header + _STDLIB_ALIGN - 1) / _STDLIB_ALIGN * _STDLIB_ALIGN;
    return block < _STDLIB_BLOCK_MIN ? _STDLIB_BLOCK_MIN : block;
}

// Return the link to the first free block of at least size bytes, or NULL.
struct _Stdlib_Block **_Stdlib_Fit(size_t size)
{
    struct _Stdlib_Block **link = &_Stdlib_FreeList;

    while (*link && (*link)->sb_size < size) {
        link = &(*link)->sb_next;
    }
    return *link ? link : NULL;
}

// Return the link to the free block that starts where block ends, or NULL.
struct _Stdlib_Block **_Stdlib_Link(const struct _Stdlib_Block *block)
{
    const char *end = (const char *) block + block->sb_size;
    struct _Stdlib_Block **link = &_Stdlib_FreeList;

    while (*link && (const char *) *link < end) {
        link = &(*link)->sb_next;
    }
    return *link && (const char *) *link == end ? link : NULL;
}

// Take size bytes of the free block at link, and leave the rest of it free.
void *_Stdlib_Take(struct _Stdlib_Block **link, size_t size)
{
    struct _Stdlib_Block *block = *link;

    if (block->sb_size - size < _STDLIB_BLOCK_MIN) {
        *link = block->sb_next;
    } else {
        struct _Stdlib_Block *rest = (struct _Stdlib_Block *) ((char *) block + size);
        rest->sb_size = block->sb_size - size;
        rest->sb_next = block->sb_next;
        block->sb_size = size;
        *link = rest;
    }
    return block + 1;
}

// Free what a block in use holds past size bytes.
void _Stdlib_Trim(struct _Stdlib_Block *block, size_t size)
{
    struct _Stdlib_Block *rest = (struct _Stdlib_Block *) ((char *) block + size);

    if (block->sb_size - size >= _STDLIB_BLOCK_MIN) {
        rest->sb_size = block->sb_size - size;
        block->sb_size = size;
        _Stdlib_Release(rest);
    }
}

// Free a block, in address order and merged with its free neighbours.
void _Stdlib_Release(struct _Stdlib_Block *block)
{
    struct _Stdlib_Block **link = &_Stdlib_FreeList;
    struct _Stdlib_Block *prev = NULL;
    struct _Stdlib_Block *next;

    while (*link && *link < block) {
        prev = *link;
        link = &(*link)->sb_next;
    }
    next = *link;
    if (next && (char *) block + block->sb_size == (char *) next) {
        block->sb_size += next->sb_size;
        next = next->sb_next;
    }
    block->sb_next = next;
    if (prev && (char *) prev + prev->sb_size == (char *) block) {
        prev->sb_size += block->sb_size;
        prev->sb_next = next;
    } else {
        *link = block;
    }
}

// Move the break past at least size more bytes and free them, or return 0.
int _Stdlib_Grow(size_t size)
{
    char *start = _Sys_Brk(NULL);
    struct _Stdlib_Block *block;
    char *end;

    start += (_STDLIB_ALIGN - (unsigned long) start % _STDLIB_ALIGN) % _STDLIB_ALIGN;
    if (size < _STDLIB_GROW) {
        size = _STDLIB_GROW;
    }
    end = start + size;
    if (_Sys_Brk(end) != end) {
        return 0;
    }
    block = (struct _Stdlib_Block *) start;
    block->sb_size = size;
    _Stdlib_Release(block);
    return 1;
}

// Take the function atexit registered last, or return NULL when none is left.
void (*_Stdlib_PopExit(void))(void)
{
    struct _Stdlib_Exits *block = _Stdlib_ExitTop;

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

// Convert the start of the string nptr to a double, as strtod does.
double atof(const char *nptr)
{
    return strtod(nptr, NULL);
}

// Convert the start of the string nptr to a double, correctly rounded.
double strtod(const char *restrict nptr, char **restrict endptr)
{
    struct _Stdlib_Real real;
    size_t end = _Stdlib_ToReal(nptr, 0, &_Stdlib_DoubleFormat, &real);
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
    struct _Stdlib_Real real;
    size_t end = _Stdlib_ToReal(nptr, 0, &_Stdlib_FloatFormat, &real);
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
    struct _Stdlib_Real real;
    size_t end = _Stdlib_ToReal(nptr, 0, &_Stdlib_LongDoubleFormat, &real);
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

// Allocate nmemb objects of size bytes, all bits zero.
void *calloc(size_t nmemb, size_t size)
{
    void *ptr;

    if (size != 0 && nmemb > (size_t) -1 / size) {
        errno = _SYS_ENOMEM;
        return NULL;
    }
    ptr = malloc(nmemb * size);
    if (ptr) {
        memset(ptr, 0, nmemb * size);
    }
    return ptr;
}

// Free the memory at ptr, which malloc, calloc or realloc gave.
void free(void *ptr)
{
    if (ptr) {
        _Stdlib_Release((struct _Stdlib_Block *) ptr - 1);
    }
}

// Allocate size bytes, or return NULL with errno ENOMEM, as glibc does.
void *malloc(size_t size)
{
    size_t need = _Stdlib_BlockSize(size);
    struct _Stdlib_Block **link = need ? _Stdlib_Fit(need) : NULL;

    if (! link && need && _Stdlib_Grow(need)) {
        link = _Stdlib_Fit(need);
    }
    if (! link) {
        errno = _SYS_ENOMEM;
        return NULL;
    }
    return _Stdlib_Take(link, need);
}

// Resize the memory at ptr to size bytes; a size of 0 frees it, as glibc's.
void *realloc(void *ptr, size_t size)
{
    size_t need = _Stdlib_BlockSize(size);
    struct _Stdlib_Block *block;
    struct _Stdlib_Block **link;
    void *moved;

    if (! ptr) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    if (need == 0) {
        errno = _SYS_ENOMEM;
        return NULL;
    }
    block = (struct _Stdlib_Block *) ptr - 1;
    link = need > block->sb_size ? _Stdlib_Link(block) : NULL;
    if (link && block->sb_size + (*link)->sb_size >= need) {
        block->sb_size += (*link)->sb_size;
        *link = (*link)->sb_next;
    }
    if (need <= block->sb_size) {
        _Stdlib_Trim(block, need);
        return ptr;
    }
    moved = malloc(size);
    if (moved) {
        memcpy(moved, ptr, block->sb_size - sizeof(struct _Stdlib_Block));
        free(ptr);
    }
    return moved;
}

// End the program by SIGABRT past a returning handler or a block, as glibc's.
void abort(void)
{
    unsigned long set = 1UL << (SIGABRT - _SYS_SIGNAL_FIRST);

    raise(SIGABRT);
    signal(SIGABRT, SIG_DFL);
    _Sys_RtSigprocmask(_SYS_SIG_UNBLOCK, &set, NULL);
    raise(SIGABRT);
    _Sys_ExitGroup(_STDLIB_ABORT_STATUS);
}

// Register func for exit to call, or return -1 out of memory, as glibc's.
int atexit(void (*func)(void))
{
    struct _Stdlib_Exits *block = _Stdlib_ExitTop;

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

// Call the functions atexit registered, newest first, then end with status.
void exit(int status)
{
    void (*func)(void);

    while ((func = _Stdlib_PopExit()) != NULL) {
        func();
    }
    _Exit(status);
}

// End the program with status, calling nothing atexit registered.
void _Exit(int status)
{
    _Sys_ExitGroup(status);
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
