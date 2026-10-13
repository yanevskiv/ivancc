/*
 * C source file for formatted output and input.
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
#include <_fmt.h>

// The error an overflowed count and a character without bytes report.
#include <_sys.h>

// The error number, and the texts of a null, an infinity and a NaN.
#include <_err.h>

// The integer a floating value's digits are those of.
#include <_bigint.h>

// Copying into a string, filling the padding and measuring a string.
#include <_str.h>

// A wide character's bytes.
#include <_utf8.h>

// Match a directive's percent sign, flags, width, precision and length.
unsigned long _Fmt_MatchPrintHead(const char *format, int *flags, int *width, int *precision, int *length)
{
    int digit;
    unsigned long i = 0;

    if (format[i] != '%') {
        return 0;
    }

    // Phase: match the flags
    *flags = 0;
    for (i++; ; i++) {
        if (format[i] == '-') {
            *flags |= _FMT_FLAG_LEFT;
        } else if (format[i] == '+') {
            *flags |= _FMT_FLAG_SIGN;
        } else if (format[i] == ' ') {
            *flags |= _FMT_FLAG_SPACE;
        } else if (format[i] == '#') {
            *flags |= _FMT_FLAG_ALT;
        } else if (format[i] == '0') {
            *flags |= _FMT_FLAG_ZERO;
        } else {
            break;
        }
    }

    // Phase: match the width
    *width = 0;
    if (format[i] == '*') {
        *width = _FMT_COUNT_ARGUMENT;
        i++;
    } else {
        for (; format[i] >= '0' && format[i] <= '9'; i++) {
            digit = format[i] - '0';
            *width = *width > (_FMT_INT_MAX - digit) / _FMT_COUNT_BASE ? _FMT_INT_MAX : *width * _FMT_COUNT_BASE + digit;
        }
    }

    // Phase: match the precision
    *precision = _FMT_PRECISION_NONE;
    if (format[i] == '.' && format[i + 1] == '*') {
        *precision = _FMT_COUNT_ARGUMENT;
        i += 2;
    } else if (format[i] == '.') {
        *precision = 0;
        for (i++; format[i] >= '0' && format[i] <= '9'; i++) {
            digit = format[i] - '0';
            *precision = *precision > (_FMT_INT_MAX - digit) / _FMT_COUNT_BASE ? _FMT_INT_MAX : *precision * _FMT_COUNT_BASE + digit;
        }
    }

    // Phase: match the length
    *length = _FMT_LENGTH_NONE;
    if (format[i] == 'h' && format[i + 1] == 'h') {
        *length = _FMT_LENGTH_CHAR;
        i += 2;
    } else if (format[i] == 'h') {
        *length = _FMT_LENGTH_SHORT;
        i++;
    } else if (format[i] == 'l' && format[i + 1] == 'l') {
        *length = _FMT_LENGTH_LLONG;
        i += 2;
    } else if (format[i] == 'l') {
        *length = _FMT_LENGTH_LONG;
        i++;
    } else if (format[i] == 'j') {
        *length = _FMT_LENGTH_INTMAX;
        i++;
    } else if (format[i] == 'z') {
        *length = _FMT_LENGTH_SIZE;
        i++;
    } else if (format[i] == 't') {
        *length = _FMT_LENGTH_PTRDIFF;
        i++;
    } else if (format[i] == 'L') {
        *length = _FMT_LENGTH_LDOUBLE;
        i++;
    }
    return i;
}

// Take the width and precision a directive left to its arguments.
void _Fmt_TakeHead(_Fmt_VaList *list, int *flags, int *width, int *precision)
{
    if (*width == _FMT_COUNT_ARGUMENT) {
        *width = __builtin_va_arg(*list, int);
    }
    if (*width < 0) {
        *flags |= _FMT_FLAG_LEFT;
        *width = *width == _FMT_INT_MIN ? _FMT_INT_MAX : -*width;
    }
    if (*precision == _FMT_COUNT_ARGUMENT) {
        *precision = __builtin_va_arg(*list, int);
    }
    if (*precision < 0) {
        *precision = _FMT_PRECISION_NONE;
    }
}

// Take an unsigned integer argument of the type a length names.
_Fmt_UintmaxType _Fmt_TakeUnsigned(_Fmt_VaList *list, int length)
{
    _Fmt_UintmaxType value;

    switch (length) {
        case _FMT_LENGTH_CHAR: {
            value = (unsigned char) __builtin_va_arg(*list, unsigned);
        } break;
        case _FMT_LENGTH_SHORT: {
            value = (unsigned short) __builtin_va_arg(*list, unsigned);
        } break;
        case _FMT_LENGTH_LONG: {
            value = __builtin_va_arg(*list, unsigned long);
        } break;
        case _FMT_LENGTH_LLONG: {
            value = __builtin_va_arg(*list, unsigned long long);
        } break;
        case _FMT_LENGTH_INTMAX: {
            value = __builtin_va_arg(*list, _Fmt_UintmaxType);
        } break;
        case _FMT_LENGTH_SIZE:
        case _FMT_LENGTH_PTRDIFF: {
            value = __builtin_va_arg(*list, unsigned long);
        } break;
        default: {
            value = __builtin_va_arg(*list, unsigned);
        } break;
    }
    return value;
}

// Match the text before a directive.
_Bool _Fmt_MatchPrintText(const char *format, unsigned long *len)
{
    *len = _Str_CountUnlisted(format, "%");
    return *len > 0;
}

// Match a %% directive.
_Bool _Fmt_MatchPrintPercent(const char *format, _Fmt_VaList *list, unsigned long *len)
{
    int flags;
    int width;
    int length;
    int precision;
    unsigned long at = _Fmt_MatchPrintHead(format, &flags, &width, &precision, &length);

    if (at == 0 || length != _FMT_LENGTH_NONE || format[at] != '%') {
        return 0;
    }
    _Fmt_TakeHead(list, &flags, &width, &precision);
    *len = at + 1;
    return 1;
}

// Match a %c directive with its argument.
_Bool _Fmt_MatchPrintChar(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, char *ch)
{
    int length;
    int precision;
    unsigned long at = _Fmt_MatchPrintHead(format, flags, width, &precision, &length);

    if (at == 0 || length != _FMT_LENGTH_NONE || format[at] != 'c') {
        return 0;
    }
    _Fmt_TakeHead(list, flags, width, &precision);
    *ch = (char) __builtin_va_arg(*list, int);
    *len = at + 1;
    return 1;
}

// Match a %s directive with its argument.
_Bool _Fmt_MatchPrintString(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, const char **str)
{
    int length;
    unsigned long at = _Fmt_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length != _FMT_LENGTH_NONE || format[at] != 's') {
        return 0;
    }
    _Fmt_TakeHead(list, flags, width, precision);
    *str = __builtin_va_arg(*list, const char *);
    *len = at + 1;
    return 1;
}

// Match a %d or %i directive with its argument.
_Bool _Fmt_MatchPrintDecimal(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, _Fmt_IntmaxType *value)
{
    int length;
    unsigned long at = _Fmt_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length == _FMT_LENGTH_LDOUBLE || (format[at] != 'd' && format[at] != 'i')) {
        return 0;
    }
    _Fmt_TakeHead(list, flags, width, precision);
    switch (length) {
        case _FMT_LENGTH_CHAR: {
            *value = (signed char) __builtin_va_arg(*list, int);
        } break;
        case _FMT_LENGTH_SHORT: {
            *value = (short) __builtin_va_arg(*list, int);
        } break;
        case _FMT_LENGTH_LONG: {
            *value = __builtin_va_arg(*list, long);
        } break;
        case _FMT_LENGTH_LLONG: {
            *value = __builtin_va_arg(*list, long long);
        } break;
        case _FMT_LENGTH_INTMAX: {
            *value = __builtin_va_arg(*list, _Fmt_IntmaxType);
        } break;
        case _FMT_LENGTH_SIZE:
        case _FMT_LENGTH_PTRDIFF: {
            *value = __builtin_va_arg(*list, long);
        } break;
        default: {
            *value = __builtin_va_arg(*list, int);
        } break;
    }
    *len = at + 1;
    return 1;
}

// Match a %u directive with its argument.
_Bool _Fmt_MatchPrintUnsigned(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, _Fmt_UintmaxType *value)
{
    int length;
    unsigned long at = _Fmt_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length == _FMT_LENGTH_LDOUBLE || format[at] != 'u') {
        return 0;
    }
    _Fmt_TakeHead(list, flags, width, precision);
    *value = _Fmt_TakeUnsigned(list, length);
    *len = at + 1;
    return 1;
}

// Match a %o directive with its argument.
_Bool _Fmt_MatchPrintOctal(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, _Fmt_UintmaxType *value)
{
    int length;
    unsigned long at = _Fmt_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length == _FMT_LENGTH_LDOUBLE || format[at] != 'o') {
        return 0;
    }
    _Fmt_TakeHead(list, flags, width, precision);
    *value = _Fmt_TakeUnsigned(list, length);
    *len = at + 1;
    return 1;
}

// Match a %x or %X directive with its argument.
_Bool _Fmt_MatchPrintHex(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, _Fmt_UintmaxType *value, _Fmt_Case *letters)
{
    int length;
    unsigned long at = _Fmt_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length == _FMT_LENGTH_LDOUBLE || (format[at] != 'x' && format[at] != 'X')) {
        return 0;
    }
    _Fmt_TakeHead(list, flags, width, precision);
    *value = _Fmt_TakeUnsigned(list, length);
    *letters = format[at] == 'X' ? _FMT_CASE_UPPER : _FMT_CASE_LOWER;
    *len = at + 1;
    return 1;
}

// Match a %p directive with its argument.
_Bool _Fmt_MatchPrintPointer(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, const void **ptr)
{
    int length;
    unsigned long at = _Fmt_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length != _FMT_LENGTH_NONE || format[at] != 'p') {
        return 0;
    }
    _Fmt_TakeHead(list, flags, width, precision);
    *ptr = __builtin_va_arg(*list, const void *);
    *len = at + 1;
    return 1;
}

// Match a %n directive with its argument.
_Bool _Fmt_MatchPrintCount(const char *format, _Fmt_VaList *list, unsigned long *len, int *length, void **dest)
{
    int flags;
    int width;
    int precision;
    unsigned long at = _Fmt_MatchPrintHead(format, &flags, &width, &precision, length);

    if (at == 0 || *length == _FMT_LENGTH_LDOUBLE || format[at] != 'n') {
        return 0;
    }
    _Fmt_TakeHead(list, &flags, &width, &precision);
    *dest = __builtin_va_arg(*list, void *);
    *len = at + 1;
    return 1;
}

// Match a %f or %F directive with its argument.
_Bool _Fmt_MatchPrintFixed(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, long double *value, _Fmt_Case *letters)
{
    int length;
    unsigned long at = _Fmt_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || (format[at] != 'f' && format[at] != 'F')) {
        return 0;
    }
    if (length != _FMT_LENGTH_NONE && length != _FMT_LENGTH_LONG && length != _FMT_LENGTH_LDOUBLE) {
        return 0;
    }
    _Fmt_TakeHead(list, flags, width, precision);
    *value = length == _FMT_LENGTH_LDOUBLE ? __builtin_va_arg(*list, long double) : __builtin_va_arg(*list, double);
    *letters = format[at] == 'F' ? _FMT_CASE_UPPER : _FMT_CASE_LOWER;
    *len = at + 1;
    return 1;
}

// Match a %e or %E directive with its argument.
_Bool _Fmt_MatchPrintExponential(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, long double *value, _Fmt_Case *letters)
{
    int length;
    unsigned long at = _Fmt_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || (format[at] != 'e' && format[at] != 'E')) {
        return 0;
    }
    if (length != _FMT_LENGTH_NONE && length != _FMT_LENGTH_LONG && length != _FMT_LENGTH_LDOUBLE) {
        return 0;
    }
    _Fmt_TakeHead(list, flags, width, precision);
    *value = length == _FMT_LENGTH_LDOUBLE ? __builtin_va_arg(*list, long double) : __builtin_va_arg(*list, double);
    *letters = format[at] == 'E' ? _FMT_CASE_UPPER : _FMT_CASE_LOWER;
    *len = at + 1;
    return 1;
}

// Match a %g or %G directive with its argument.
_Bool _Fmt_MatchPrintGeneral(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, long double *value, _Fmt_Case *letters)
{
    int length;
    unsigned long at = _Fmt_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || (format[at] != 'g' && format[at] != 'G')) {
        return 0;
    }
    if (length != _FMT_LENGTH_NONE && length != _FMT_LENGTH_LONG && length != _FMT_LENGTH_LDOUBLE) {
        return 0;
    }
    _Fmt_TakeHead(list, flags, width, precision);
    *value = length == _FMT_LENGTH_LDOUBLE ? __builtin_va_arg(*list, long double) : __builtin_va_arg(*list, double);
    *letters = format[at] == 'G' ? _FMT_CASE_UPPER : _FMT_CASE_LOWER;
    *len = at + 1;
    return 1;
}

// Match a %a or %A directive with its argument, a double.
_Bool _Fmt_MatchPrintHexReal(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, double *value, _Fmt_Case *letters)
{
    int length;
    unsigned long at = _Fmt_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || (format[at] != 'a' && format[at] != 'A')) {
        return 0;
    }
    if (length != _FMT_LENGTH_NONE && length != _FMT_LENGTH_LONG) {
        return 0;
    }
    _Fmt_TakeHead(list, flags, width, precision);
    *value = __builtin_va_arg(*list, double);
    *letters = format[at] == 'A' ? _FMT_CASE_UPPER : _FMT_CASE_LOWER;
    *len = at + 1;
    return 1;
}

// Match a %La or %LA directive with its argument, a long double.
_Bool _Fmt_MatchPrintHexLongReal(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, long double *value, _Fmt_Case *letters)
{
    int length;
    unsigned long at = _Fmt_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length != _FMT_LENGTH_LDOUBLE || (format[at] != 'a' && format[at] != 'A')) {
        return 0;
    }
    _Fmt_TakeHead(list, flags, width, precision);
    *value = __builtin_va_arg(*list, long double);
    *letters = format[at] == 'A' ? _FMT_CASE_UPPER : _FMT_CASE_LOWER;
    *len = at + 1;
    return 1;
}

// Match a %lc directive with its argument, a wide character.
_Bool _Fmt_MatchPrintWideChar(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, _Fmt_WcharType *wc)
{
    int length;
    int precision;
    unsigned long at = _Fmt_MatchPrintHead(format, flags, width, &precision, &length);

    if (at == 0 || length != _FMT_LENGTH_LONG || format[at] != 'c') {
        return 0;
    }
    _Fmt_TakeHead(list, flags, width, &precision);
    *wc = (_Fmt_WcharType) __builtin_va_arg(*list, _Fmt_WintType);
    *len = at + 1;
    return 1;
}

// Match a %ls directive with its argument, a wide string.
_Bool _Fmt_MatchPrintWideString(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, const _Fmt_WcharType **wstr)
{
    int length;
    unsigned long at = _Fmt_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length != _FMT_LENGTH_LONG || format[at] != 's') {
        return 0;
    }
    _Fmt_TakeHead(list, flags, width, precision);
    *wstr = __builtin_va_arg(*list, const _Fmt_WcharType *);
    *len = at + 1;
    return 1;
}

// Write a value's digits in a base to end at end, none for zero.
unsigned long _Fmt_WriteDigits(char *end, _Fmt_UintmaxType value, int base, const char *digits)
{
    char *ptr = end;

    while (value != 0) {
        ptr--;
        ptr[0] = digits[value % (_Fmt_UintmaxType) base];
        value /= (_Fmt_UintmaxType) base;
    }
    return (unsigned long) (end - ptr);
}

// Store a count at dest as an integer of the type a length names.
void _Fmt_StoreCount(void *dest, int length, unsigned long count)
{
    switch (length) {
        case _FMT_LENGTH_CHAR: {
            *(signed char *) dest = (signed char) count;
        } break;
        case _FMT_LENGTH_SHORT: {
            *(short *) dest = (short) count;
        } break;
        case _FMT_LENGTH_LONG: {
            *(long *) dest = (long) count;
        } break;
        case _FMT_LENGTH_LLONG: {
            *(long long *) dest = (long long) count;
        } break;
        case _FMT_LENGTH_INTMAX: {
            *(_Fmt_IntmaxType *) dest = (_Fmt_IntmaxType) count;
        } break;
        case _FMT_LENGTH_SIZE: {
            *(unsigned long *) dest = count;
        } break;
        case _FMT_LENGTH_PTRDIFF: {
            *(long *) dest = (long) count;
        } break;
        default: {
            *(int *) dest = (int) count;
        } break;
    }
}

// Unpack a floating value into a sign and its bits by a power of two.
_Fmt_Real _Fmt_UnpackReal(long double value, _Fmt_Sign *sign, unsigned long long *mant, int *exp)
{
    unsigned short top;
    unsigned int biased;
    _Fmt_Real kind = _FMT_REAL_FINITE;
    unsigned int ones = (unsigned int) _FMT_LDOUBLE_EXP * 2 - 1;
    unsigned long long lead = 1ULL << (_FMT_LDOUBLE_MANT - 1);

    _Str_CopyBytes(mant, &value, sizeof(*mant));
    _Str_CopyBytes(&top, (char *) &value + sizeof(*mant), sizeof(top));
    biased = top & ones;
    *exp = (int) (biased != 0 ? biased : 1) - (_FMT_LDOUBLE_EXP - 1) - (_FMT_LDOUBLE_MANT - 1);
    *sign = top > ones ? _FMT_SIGN_MINUS : _FMT_SIGN_PLUS;
    if (biased == ones) {
        kind = (*mant & (lead - 1)) != 0 ? _FMT_REAL_NAN : _FMT_REAL_INFINITE;
    }
    if (biased != 0 && ! (*mant & lead)) {
        kind = _FMT_REAL_NAN;
    }
    return kind;
}

// Write a floating value's sign, what it is and its every decimal digit.
void _Fmt_WriteNumeral(long double value, _Fmt_Numeral *num)
{
    int i;
    int exp;
    int step;
    int below;
    int place = 0;
    unsigned int chunk;
    _Bigint_Number big;
    unsigned long long part;
    unsigned long long mant;
    char run[_BIGINT_POW5_STEP];
    char *end = num->fn_digit + sizeof(num->fn_digit);
    char *ptr = end;

    // Phase: take the value apart
    num->fn_kind = _Fmt_UnpackReal(value, &num->fn_sign, &mant, &exp);
    num->fn_len = 0;
    num->fn_exp = 0;
    if (num->fn_kind != _FMT_REAL_FINITE) {
        return;
    }

    // Phase: write the whole part's digits
    _Bigint_SetValue(&big, mant);
    if (exp > 0) {
        _Bigint_ShiftBitsLeft(&big, exp);
    } else {
        _Bigint_ShiftBitsRight(&big, -exp);
    }
    while (big.bn_len > 0) {
        chunk = _Bigint_DivideWord(&big, _BIGINT_POW10_WORD);
        for (i = 0; i < _BIGINT_POW10_STEP; i++) {
            ptr--;
            ptr[0] = (char) ('0' + chunk % _FMT_BASE_DECIMAL);
            chunk /= _FMT_BASE_DECIMAL;
        }
    }
    while (ptr < end && ptr[0] == '0') {
        ptr++;
    }
    num->fn_len = (unsigned long) (end - ptr);
    num->fn_exp = num->fn_len > 0 ? (int) num->fn_len - 1 : 0;
    _Str_MoveBytes(num->fn_digit, ptr, num->fn_len);

    // Phase: write the fraction's digits
    below = exp < 0 ? -exp : 0;
    _Bigint_SetValue(&big, mant);
    _Bigint_SplitBits(&big, below);
    while (big.bn_len > 0) {
        step = below < _BIGINT_POW5_STEP ? below : _BIGINT_POW5_STEP;
        _Bigint_MulPow5(&big, step);
        below -= step;
        part = _Bigint_SplitBits(&big, below);
        for (i = step - 1; i >= 0; i--) {
            run[i] = (char) ('0' + part % _FMT_BASE_DECIMAL);
            part /= _FMT_BASE_DECIMAL;
        }
        for (i = 0; i < step; i++) {
            place++;
            if (num->fn_len == 0 && run[i] != '0') {
                num->fn_exp = -place;
            }
            if (num->fn_len > 0 || run[i] != '0') {
                num->fn_digit[num->fn_len++] = run[i];
            }
        }
    }

    // Phase: drop the zeros at the end
    while (num->fn_len > 0 && num->fn_digit[num->fn_len - 1] == '0') {
        num->fn_len--;
    }
}

// Round num to its first keep digits, to nearest and a tie to even.
void _Fmt_RoundNumeral(_Fmt_Numeral *num, long keep)
{
    _Bool up;
    unsigned long len;
    char *digit = num->fn_digit;

    if (keep >= (long) num->fn_len) {
        return;
    }
    len = keep > 0 ? (unsigned long) keep : 0;
    up = keep >= 0 && digit[len] >= '5';
    if (up && digit[len] == '5' && len + 1 == num->fn_len) {
        up = len > 0 && (digit[len - 1] - '0') % 2 != 0;
    }
    while (up && len > 0 && digit[len - 1] == '9') {
        len--;
    }
    if (up && len == 0) {
        digit[0] = '1';
        len = 1;
        num->fn_exp++;
    } else if (up) {
        digit[len - 1]++;
    }
    while (len > 0 && digit[len - 1] == '0') {
        len--;
    }
    num->fn_len = len;
    if (len == 0) {
        num->fn_exp = 0;
    }
}

// End the sink's string, and return its count, or _FILE_END if it failed.
int _Fmt_EndSink(_Fmt_Sink *sink)
{
    if (sink->fs_stream == _FMT_NULL && sink->fs_size > 0) {
        unsigned long last = sink->fs_size - 1;

        sink->fs_str[sink->fs_count < last ? sink->fs_count : last] = '\0';
    }
    return sink->fs_failed ? _FILE_END : (int) sink->fs_count;
}

// Emit len characters of data, counted though the sink's string be full.
void _Fmt_EmitText(_Fmt_Sink *sink, const char *data, unsigned long len)
{
    unsigned long room;

    if (sink->fs_failed) {
        return;
    }
    if (len > (unsigned long) _FMT_INT_MAX - sink->fs_count) {
        _Err_Errno = _SYS_EOVERFLOW;
        sink->fs_failed = 1;
        return;
    }
    if (sink->fs_stream != _FMT_NULL) {
        sink->fs_failed = _File_PutBytes(sink->fs_stream, data, len) != len;
    } else if (sink->fs_count + 1 < sink->fs_size) {
        room = sink->fs_size - 1 - sink->fs_count;
        _Str_CopyBytes(sink->fs_str + sink->fs_count, data, len < room ? len : room);
    }
    sink->fs_count += len;
}

// Emit len copies of the character ch.
void _Fmt_EmitPad(_Fmt_Sink *sink, int ch, unsigned long len)
{
    char chunk[_FMT_PAD_CHUNK];

    _Str_FillBytes(chunk, ch, sizeof(chunk));
    while (len > 0 && ! sink->fs_failed) {
        unsigned long part = len < sizeof(chunk) ? len : sizeof(chunk);

        _Fmt_EmitText(sink, chunk, part);
        len -= part;
    }
}

// Emit len characters of data in a width.
void _Fmt_EmitField(_Fmt_Sink *sink, int flags, int width, const char *data, unsigned long len)
{
    unsigned long pad = (unsigned long) width > len ? (unsigned long) width - len : 0;

    if (! (flags & _FMT_FLAG_LEFT)) {
        _Fmt_EmitPad(sink, ' ', pad);
    }
    _Fmt_EmitText(sink, data, len);
    if (flags & _FMT_FLAG_LEFT) {
        _Fmt_EmitPad(sink, ' ', pad);
    }
}

// Emit what leads a number, and return the spaces to follow it.
unsigned long _Fmt_EmitLead(_Fmt_Sink *sink, int flags, int width, _Fmt_Sign sign, const char *prefix, unsigned long len)
{
    unsigned long pad;
    unsigned long lead;
    const char *text = "";

    if (sign == _FMT_SIGN_MINUS) {
        text = "-";
    } else if (flags & _FMT_FLAG_SIGN) {
        text = "+";
    } else if (flags & _FMT_FLAG_SPACE) {
        text = " ";
    }
    if (flags & _FMT_FLAG_LEFT) {
        flags &= ~_FMT_FLAG_ZERO;
    }
    lead = _Str_GetLength(text) + _Str_GetLength(prefix);
    pad = (unsigned long) width > lead + len ? (unsigned long) width - lead - len : 0;
    if (! (flags & (_FMT_FLAG_LEFT | _FMT_FLAG_ZERO))) {
        _Fmt_EmitPad(sink, ' ', pad);
    }
    _Fmt_EmitText(sink, text, _Str_GetLength(text));
    _Fmt_EmitText(sink, prefix, _Str_GetLength(prefix));
    if (flags & _FMT_FLAG_ZERO) {
        _Fmt_EmitPad(sink, '0', pad);
    }
    return flags & _FMT_FLAG_LEFT ? pad : 0;
}

// Emit an integer's digits in a width, its precision the fewest.
void _Fmt_EmitInteger(_Fmt_Sink *sink, int flags, int width, int precision, _Fmt_Sign sign, const char *prefix, const char *digits, unsigned long len)
{
    unsigned long rest;
    unsigned long zeros;

    if (precision == _FMT_PRECISION_NONE) {
        precision = _FMT_INTEGER_PRECISION;
    } else {
        flags &= ~_FMT_FLAG_ZERO;
    }
    zeros = (unsigned long) precision > len ? (unsigned long) precision - len : 0;
    rest = _Fmt_EmitLead(sink, flags, width, sign, prefix, zeros + len);
    _Fmt_EmitPad(sink, '0', zeros);
    _Fmt_EmitText(sink, digits, len);
    _Fmt_EmitPad(sink, ' ', rest);
}

// Emit count digits of num from the one worth 10^high, zeros for none.
void _Fmt_EmitSpan(_Fmt_Sink *sink, const _Fmt_Numeral *num, long high, unsigned long count)
{
    unsigned long real;
    unsigned long lead = high > num->fn_exp ? (unsigned long) (high - num->fn_exp) : 0;
    unsigned long skip = high < num->fn_exp ? (unsigned long) (num->fn_exp - high) : 0;

    if (lead > count) {
        lead = count;
    }
    count -= lead;
    real = skip < num->fn_len ? num->fn_len - skip : 0;
    if (real > count) {
        real = count;
    }
    _Fmt_EmitPad(sink, '0', lead);
    _Fmt_EmitText(sink, num->fn_digit + skip, real);
    _Fmt_EmitPad(sink, '0', count - real);
}

// Emit a rounded numeral in a width, frac digits after its point.
void _Fmt_EmitNumeral(_Fmt_Sink *sink, int flags, int width, const _Fmt_Numeral *num, unsigned long frac, const char *mark)
{
    unsigned long rest;
    char point = _FMT_POINT;
    char text[_FMT_DIGITS_MAX];
    _Bool powered = mark[0] != '\0';
    long high = powered || num->fn_exp > 0 ? num->fn_exp : 0;
    unsigned long whole = powered ? 1 : (unsigned long) high + 1;
    unsigned long dot = frac > 0 || (flags & _FMT_FLAG_ALT) ? 1 : 0;
    _Fmt_UintmaxType power = (_Fmt_UintmaxType) (num->fn_exp < 0 ? -num->fn_exp : num->fn_exp);
    unsigned long count = _Fmt_WriteDigits(text + sizeof(text), power, _FMT_BASE_DECIMAL, _FMT_DIGITS_LOWER);
    unsigned long zeros = count < _FMT_EXPONENT_MIN ? _FMT_EXPONENT_MIN - count : 0;
    unsigned long tail = powered ? _Str_GetLength(mark) + 1 + zeros + count : 0;

    rest = _Fmt_EmitLead(sink, flags, width, num->fn_sign, "", whole + dot + frac + tail);
    _Fmt_EmitSpan(sink, num, high, whole);
    _Fmt_EmitText(sink, &point, dot);
    _Fmt_EmitSpan(sink, num, high - (long) whole, frac);
    if (powered) {
        _Fmt_EmitText(sink, mark, _Str_GetLength(mark));
        _Fmt_EmitText(sink, num->fn_exp < 0 ? "-" : "+", 1);
        _Fmt_EmitPad(sink, '0', zeros);
        _Fmt_EmitText(sink, text + sizeof(text) - count, count);
    }
    _Fmt_EmitPad(sink, ' ', rest);
}

// Emit an infinity or a NaN in a width, a sign and a text, no zeros.
void _Fmt_EmitSpecial(_Fmt_Sink *sink, int flags, int width, _Fmt_Sign sign, _Fmt_Real kind, _Fmt_Case letters)
{
    unsigned long rest;
    const char *inf = letters == _FMT_CASE_UPPER ? _ERR_FMT_INF_UPPER : _ERR_FMT_INF;
    const char *nan = letters == _FMT_CASE_UPPER ? _ERR_FMT_NAN_UPPER : _ERR_FMT_NAN;
    const char *text = kind == _FMT_REAL_NAN ? nan : inf;

    flags &= ~_FMT_FLAG_ZERO;
    rest = _Fmt_EmitLead(sink, flags, width, sign, "", _Str_GetLength(text));
    _Fmt_EmitText(sink, text, _Str_GetLength(text));
    _Fmt_EmitPad(sink, ' ', rest);
}

// Emit a mantissa in hex, its top four bits first, by a power of two.
void _Fmt_EmitMantissa(_Fmt_Sink *sink, int flags, int width, int precision, _Fmt_Sign sign, unsigned long long mant, int exp, _Fmt_Case letters)
{
    unsigned long dot;
    unsigned long kept;
    unsigned long rest;
    unsigned long zeros;
    unsigned long count;
    unsigned long long unit;
    unsigned long long below;
    char point = _FMT_POINT;
    char text[_FMT_DIGITS_MAX];
    char digits[_FMT_HEX_DIGITS];
    int last = _FMT_HEX_DIGITS - 1;
    _Bool upper = letters == _FMT_CASE_UPPER;

    // Phase: round to the precision, or find the least that is exact
    if (precision == _FMT_PRECISION_NONE) {
        for (precision = last; precision > 0 && (mant >> (_FMT_HEX_BITS * (last - precision))) % _FMT_BASE_HEX == 0; precision--) {
            // empty
        }
    }
    if (precision < last) {
        unit = 1ULL << (_FMT_HEX_BITS * (last - precision));
        below = mant & (unit - 1);
        mant -= below;
        if (below > unit / 2 || (below == unit / 2 && (mant & unit) != 0)) {
            mant += unit;
            if (mant < unit) {
                mant = 1ULL << (_FMT_HEX_BITS * last);
                exp += _FMT_HEX_BITS;
            }
        }
    }

    // Phase: write its digits and its power
    dot = precision > 0 || (flags & _FMT_FLAG_ALT) ? 1 : 0;
    _Str_FillBytes(digits, '0', sizeof(digits));
    _Fmt_WriteDigits(digits + sizeof(digits), mant, _FMT_BASE_HEX, upper ? _FMT_DIGITS_UPPER : _FMT_DIGITS_LOWER);
    count = _Fmt_WriteDigits(text + sizeof(text), (_Fmt_UintmaxType) (exp < 0 ? -exp : exp), _FMT_BASE_DECIMAL, _FMT_DIGITS_LOWER);
    zeros = count < _FMT_POWER_MIN ? _FMT_POWER_MIN - count : 0;
    kept = precision < last ? (unsigned long) precision : (unsigned long) last;
    rest = _Fmt_EmitLead(sink, flags, width, sign, upper ? "0X" : "0x", 1 + dot + (unsigned long) precision + 2 + zeros + count);
    _Fmt_EmitText(sink, digits, 1);
    _Fmt_EmitText(sink, &point, dot);
    _Fmt_EmitText(sink, digits + 1, kept);
    _Fmt_EmitPad(sink, '0', (unsigned long) precision - kept);
    _Fmt_EmitText(sink, upper ? "P" : "p", 1);
    _Fmt_EmitText(sink, exp < 0 ? "-" : "+", 1);
    _Fmt_EmitPad(sink, '0', zeros);
    _Fmt_EmitText(sink, text + sizeof(text) - count, count);
    _Fmt_EmitPad(sink, ' ', rest);
}

// Emit a character as %c converts it.
void _Fmt_EmitChar(_Fmt_Sink *sink, int flags, int width, char ch)
{
    _Fmt_EmitField(sink, flags, width, &ch, 1);
}

// Emit a string as %s converts it, a null one as glibc does.
void _Fmt_EmitString(_Fmt_Sink *sink, int flags, int width, int precision, const char *str)
{
    const char *end;
    unsigned long len;

    if (str == _FMT_NULL) {
        str = precision == _FMT_PRECISION_NONE || precision >= (int) sizeof(_ERR_FMT_NULL) - 1 ? _ERR_FMT_NULL : "";
    }
    if (precision == _FMT_PRECISION_NONE) {
        len = _Str_GetLength(str);
    } else {
        end = _Str_FindByte(str, '\0', (unsigned long) precision);
        len = end != _FMT_NULL ? (unsigned long) (end - str) : (unsigned long) precision;
    }
    _Fmt_EmitField(sink, flags, width, str, len);
}

// Emit an integer as %d and %i convert it.
void _Fmt_EmitDecimal(_Fmt_Sink *sink, int flags, int width, int precision, _Fmt_IntmaxType value)
{
    char text[_FMT_DIGITS_MAX];
    _Fmt_UintmaxType magnitude = value < 0 ? -(_Fmt_UintmaxType) value : (_Fmt_UintmaxType) value;
    unsigned long len = _Fmt_WriteDigits(text + sizeof(text), magnitude, _FMT_BASE_DECIMAL, _FMT_DIGITS_LOWER);

    _Fmt_EmitInteger(sink, flags, width, precision, value < 0 ? _FMT_SIGN_MINUS : _FMT_SIGN_PLUS, "", text + sizeof(text) - len, len);
}

// Emit an unsigned integer as %u converts it.
void _Fmt_EmitUnsigned(_Fmt_Sink *sink, int flags, int width, int precision, _Fmt_UintmaxType value)
{
    char text[_FMT_DIGITS_MAX];
    unsigned long len = _Fmt_WriteDigits(text + sizeof(text), value, _FMT_BASE_DECIMAL, _FMT_DIGITS_LOWER);

    flags &= ~(_FMT_FLAG_SIGN | _FMT_FLAG_SPACE);
    _Fmt_EmitInteger(sink, flags, width, precision, _FMT_SIGN_PLUS, "", text + sizeof(text) - len, len);
}

// Emit an unsigned integer as %o converts it.
void _Fmt_EmitOctal(_Fmt_Sink *sink, int flags, int width, int precision, _Fmt_UintmaxType value)
{
    char text[_FMT_DIGITS_MAX];
    const char *prefix = "";
    unsigned long least = precision == _FMT_PRECISION_NONE ? _FMT_INTEGER_PRECISION : (unsigned long) precision;
    unsigned long len = _Fmt_WriteDigits(text + sizeof(text), value, _FMT_BASE_OCTAL, _FMT_DIGITS_LOWER);

    if ((flags & _FMT_FLAG_ALT) && len >= least) {
        prefix = "0";
    }
    flags &= ~(_FMT_FLAG_SIGN | _FMT_FLAG_SPACE);
    _Fmt_EmitInteger(sink, flags, width, precision, _FMT_SIGN_PLUS, prefix, text + sizeof(text) - len, len);
}

// Emit an unsigned integer as %x and %X convert it.
void _Fmt_EmitHex(_Fmt_Sink *sink, int flags, int width, int precision, _Fmt_UintmaxType value, _Fmt_Case letters)
{
    char text[_FMT_DIGITS_MAX];
    const char *prefix = "";
    const char *digits = letters == _FMT_CASE_UPPER ? _FMT_DIGITS_UPPER : _FMT_DIGITS_LOWER;
    unsigned long len = _Fmt_WriteDigits(text + sizeof(text), value, _FMT_BASE_HEX, digits);

    if ((flags & _FMT_FLAG_ALT) && value != 0) {
        prefix = letters == _FMT_CASE_UPPER ? "0X" : "0x";
    }
    flags &= ~(_FMT_FLAG_SIGN | _FMT_FLAG_SPACE);
    _Fmt_EmitInteger(sink, flags, width, precision, _FMT_SIGN_PLUS, prefix, text + sizeof(text) - len, len);
}

// Emit a pointer as glibc does: a null one a text, another by %#lx.
void _Fmt_EmitPointer(_Fmt_Sink *sink, int flags, int width, int precision, const void *ptr)
{
    char text[_FMT_DIGITS_MAX];
    unsigned long len = _Fmt_WriteDigits(text + sizeof(text), (unsigned long) ptr, _FMT_BASE_HEX, _FMT_DIGITS_LOWER);

    if (ptr == _FMT_NULL) {
        _Fmt_EmitField(sink, flags, width, _ERR_FMT_NIL, sizeof(_ERR_FMT_NIL) - 1);
        return;
    }
    _Fmt_EmitInteger(sink, flags, width, precision, _FMT_SIGN_PLUS, "0x", text + sizeof(text) - len, len);
}

// Emit a floating value as %f and %F convert it.
void _Fmt_EmitFixed(_Fmt_Sink *sink, int flags, int width, int precision, long double value, _Fmt_Case letters)
{
    _Fmt_Numeral num;

    _Fmt_WriteNumeral(value, &num);
    if (num.fn_kind != _FMT_REAL_FINITE) {
        _Fmt_EmitSpecial(sink, flags, width, num.fn_sign, num.fn_kind, letters);
        return;
    }
    if (precision == _FMT_PRECISION_NONE) {
        precision = _FMT_REAL_PRECISION;
    }
    _Fmt_RoundNumeral(&num, (long) num.fn_exp + 1 + precision);
    _Fmt_EmitNumeral(sink, flags, width, &num, (unsigned long) precision, "");
}

// Emit a floating value as %e and %E convert it.
void _Fmt_EmitExponential(_Fmt_Sink *sink, int flags, int width, int precision, long double value, _Fmt_Case letters)
{
    _Fmt_Numeral num;

    _Fmt_WriteNumeral(value, &num);
    if (num.fn_kind != _FMT_REAL_FINITE) {
        _Fmt_EmitSpecial(sink, flags, width, num.fn_sign, num.fn_kind, letters);
        return;
    }
    if (precision == _FMT_PRECISION_NONE) {
        precision = _FMT_REAL_PRECISION;
    }
    _Fmt_RoundNumeral(&num, (long) precision + 1);
    _Fmt_EmitNumeral(sink, flags, width, &num, (unsigned long) precision, letters == _FMT_CASE_UPPER ? "E" : "e");
}

// Emit a floating value as %g and %G convert it.
void _Fmt_EmitGeneral(_Fmt_Sink *sink, int flags, int width, int precision, long double value, _Fmt_Case letters)
{
    long frac;
    _Bool powered;
    _Fmt_Numeral num;
    const char *mark = "";

    _Fmt_WriteNumeral(value, &num);
    if (num.fn_kind != _FMT_REAL_FINITE) {
        _Fmt_EmitSpecial(sink, flags, width, num.fn_sign, num.fn_kind, letters);
        return;
    }
    if (precision == _FMT_PRECISION_NONE) {
        precision = _FMT_REAL_PRECISION;
    } else if (precision == 0) {
        precision = _FMT_REAL_LEAST;
    }
    _Fmt_RoundNumeral(&num, precision);
    powered = num.fn_exp < _FMT_GENERAL_MIN || num.fn_exp >= precision;
    frac = (flags & _FMT_FLAG_ALT ? precision : (long) num.fn_len) - 1;
    if (powered) {
        mark = letters == _FMT_CASE_UPPER ? "E" : "e";
    } else {
        frac -= num.fn_exp;
    }
    _Fmt_EmitNumeral(sink, flags, width, &num, frac > 0 ? (unsigned long) frac : 0, mark);
}

// Emit a double as %a and %A convert it, its first digit a 1 or a 0.
void _Fmt_EmitHexReal(_Fmt_Sink *sink, int flags, int width, int precision, double value, _Fmt_Case letters)
{
    int exp;
    int under;
    _Fmt_Sign sign;
    unsigned long long mant;
    _Fmt_Real kind = _Fmt_UnpackReal(value, &sign, &mant, &exp);

    if (kind != _FMT_REAL_FINITE) {
        _Fmt_EmitSpecial(sink, flags, width, sign, kind, letters);
        return;
    }
    exp = mant != 0 ? exp + _FMT_LDOUBLE_MANT - 1 : 0;
    under = exp < _FMT_DOUBLE_MIN - 1 ? _FMT_DOUBLE_MIN - 1 - exp : 0;
    _Fmt_EmitMantissa(sink, flags, width, precision, sign, mant >> (_FMT_HEX_BITS - 1) >> under, exp + under, letters);
}

// Emit a long double as %La and %LA convert it, its top bits first.
void _Fmt_EmitHexLongReal(_Fmt_Sink *sink, int flags, int width, int precision, long double value, _Fmt_Case letters)
{
    int exp;
    _Fmt_Sign sign;
    unsigned long long mant;
    _Fmt_Real kind = _Fmt_UnpackReal(value, &sign, &mant, &exp);

    if (kind != _FMT_REAL_FINITE) {
        _Fmt_EmitSpecial(sink, flags, width, sign, kind, letters);
        return;
    }
    exp = mant != 0 ? exp + _FMT_LDOUBLE_MANT - _FMT_HEX_BITS : 0;
    _Fmt_EmitMantissa(sink, flags, width, precision, sign, mant, exp, letters);
}

// Emit a wide character as %lc converts it, a wide string of it alone.
void _Fmt_EmitWideChar(_Fmt_Sink *sink, int flags, int width, _Fmt_WcharType wc)
{
    _Fmt_WcharType pair[2] = {wc, L'\0'};

    _Fmt_EmitWideString(sink, flags, width, _FMT_PRECISION_NONE, pair);
}

// Emit a wide string as %ls converts it, in the locale's bytes.
void _Fmt_EmitWideString(_Fmt_Sink *sink, int flags, int width, int precision, const _Fmt_WcharType *wstr)
{
    int n;
    unsigned long i;
    unsigned long pad;
    unsigned long len = 0;
    unsigned long count = 0;
    char bytes[_FMT_BYTES_MAX];
    unsigned long max = precision == _FMT_PRECISION_NONE ? _FMT_SIZE_MAX : (unsigned long) precision;

    if (wstr == _FMT_NULL) {
        _Fmt_EmitString(sink, flags, width, precision, _FMT_NULL);
        return;
    }

    // Phase: count the characters whose bytes the precision holds
    for (; len < max && wstr[count] != L'\0'; count++) {
        n = _Utf8_EncodeChar(wstr[count], bytes);
        if (n < 0) {
            _Err_Errno = _SYS_EILSEQ;
            sink->fs_failed = 1;
            return;
        }
        if ((unsigned long) n > max - len) {
            break;
        }
        len += (unsigned long) n;
    }

    // Phase: write them in the width
    pad = (unsigned long) width > len ? (unsigned long) width - len : 0;
    if (! (flags & _FMT_FLAG_LEFT)) {
        _Fmt_EmitPad(sink, ' ', pad);
    }
    for (i = 0; i < count; i++) {
        n = _Utf8_EncodeChar(wstr[i], bytes);
        _Fmt_EmitText(sink, bytes, (unsigned long) n);
    }
    if (flags & _FMT_FLAG_LEFT) {
        _Fmt_EmitPad(sink, ' ', pad);
    }
}

// Emit the format's text, each directive converting its arguments.
int _Fmt_EmitFormat(_Fmt_Sink *sink, const char *format, _Fmt_VaList args)
{
    char ch;
    int flags;
    int width;
    int length;
    void *dest;
    double dval;
    int precision;
    const char *str;
    const void *ptr;
    long double fval;
    _Fmt_VaList list;
    _Fmt_Case letters;
    unsigned long len;
    _Fmt_WcharType wc;
    _Fmt_IntmaxType sval;
    _Fmt_UintmaxType uval;
    const _Fmt_WcharType *wstr;

    __builtin_va_copy(list, args);
    for (unsigned long i = 0; format[i] != '\0'; i += len) {
        if (_Fmt_MatchPrintText(format + i, &len)) {
            _Fmt_EmitText(sink, format + i, len);
        } else if (_Fmt_MatchPrintPercent(format + i, &list, &len)) {
            _Fmt_EmitText(sink, "%", 1);
        } else if (_Fmt_MatchPrintChar(format + i, &list, &len, &flags, &width, &ch)) {
            _Fmt_EmitChar(sink, flags, width, ch);
        } else if (_Fmt_MatchPrintString(format + i, &list, &len, &flags, &width, &precision, &str)) {
            _Fmt_EmitString(sink, flags, width, precision, str);
        } else if (_Fmt_MatchPrintDecimal(format + i, &list, &len, &flags, &width, &precision, &sval)) {
            _Fmt_EmitDecimal(sink, flags, width, precision, sval);
        } else if (_Fmt_MatchPrintUnsigned(format + i, &list, &len, &flags, &width, &precision, &uval)) {
            _Fmt_EmitUnsigned(sink, flags, width, precision, uval);
        } else if (_Fmt_MatchPrintOctal(format + i, &list, &len, &flags, &width, &precision, &uval)) {
            _Fmt_EmitOctal(sink, flags, width, precision, uval);
        } else if (_Fmt_MatchPrintHex(format + i, &list, &len, &flags, &width, &precision, &uval, &letters)) {
            _Fmt_EmitHex(sink, flags, width, precision, uval, letters);
        } else if (_Fmt_MatchPrintPointer(format + i, &list, &len, &flags, &width, &precision, &ptr)) {
            _Fmt_EmitPointer(sink, flags, width, precision, ptr);
        } else if (_Fmt_MatchPrintCount(format + i, &list, &len, &length, &dest)) {
            _Fmt_StoreCount(dest, length, sink->fs_count);
        } else if (_Fmt_MatchPrintFixed(format + i, &list, &len, &flags, &width, &precision, &fval, &letters)) {
            _Fmt_EmitFixed(sink, flags, width, precision, fval, letters);
        } else if (_Fmt_MatchPrintExponential(format + i, &list, &len, &flags, &width, &precision, &fval, &letters)) {
            _Fmt_EmitExponential(sink, flags, width, precision, fval, letters);
        } else if (_Fmt_MatchPrintGeneral(format + i, &list, &len, &flags, &width, &precision, &fval, &letters)) {
            _Fmt_EmitGeneral(sink, flags, width, precision, fval, letters);
        } else if (_Fmt_MatchPrintHexReal(format + i, &list, &len, &flags, &width, &precision, &dval, &letters)) {
            _Fmt_EmitHexReal(sink, flags, width, precision, dval, letters);
        } else if (_Fmt_MatchPrintHexLongReal(format + i, &list, &len, &flags, &width, &precision, &fval, &letters)) {
            _Fmt_EmitHexLongReal(sink, flags, width, precision, fval, letters);
        } else if (_Fmt_MatchPrintWideChar(format + i, &list, &len, &flags, &width, &wc)) {
            _Fmt_EmitWideChar(sink, flags, width, wc);
        } else if (_Fmt_MatchPrintWideString(format + i, &list, &len, &flags, &width, &precision, &wstr)) {
            _Fmt_EmitWideString(sink, flags, width, precision, wstr);
        } else {
            len = 1;
            _Fmt_EmitText(sink, format + i, len);
        }
    }
    __builtin_va_end(list);
    return _Fmt_EndSink(sink);
}
