/*
 * C source file for formatted output.
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
#include <_print.h>

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

// The conversions, a letter that means two things by its length being two.
const _Print_Rule _Print_Rules[_PRINT_RULES] = {
    {'d', _PRINT_LENGTHS_INTEGER, _PRINT_KIND_INTEGER,     _PRINT_ARG_SIGNED,      _PRINT_BASE_DECIMAL, _PRINT_STYLE_NONE,        _PRINT_CASE_LOWER},
    {'i', _PRINT_LENGTHS_INTEGER, _PRINT_KIND_INTEGER,     _PRINT_ARG_SIGNED,      _PRINT_BASE_DECIMAL, _PRINT_STYLE_NONE,        _PRINT_CASE_LOWER},
    {'o', _PRINT_LENGTHS_INTEGER, _PRINT_KIND_INTEGER,     _PRINT_ARG_UNSIGNED,    _PRINT_BASE_OCTAL,   _PRINT_STYLE_NONE,        _PRINT_CASE_LOWER},
    {'u', _PRINT_LENGTHS_INTEGER, _PRINT_KIND_INTEGER,     _PRINT_ARG_UNSIGNED,    _PRINT_BASE_DECIMAL, _PRINT_STYLE_NONE,        _PRINT_CASE_LOWER},
    {'x', _PRINT_LENGTHS_INTEGER, _PRINT_KIND_INTEGER,     _PRINT_ARG_UNSIGNED,    _PRINT_BASE_HEX,     _PRINT_STYLE_NONE,        _PRINT_CASE_LOWER},
    {'X', _PRINT_LENGTHS_INTEGER, _PRINT_KIND_INTEGER,     _PRINT_ARG_UNSIGNED,    _PRINT_BASE_HEX,     _PRINT_STYLE_NONE,        _PRINT_CASE_UPPER},
    {'f', _PRINT_LENGTHS_REAL,    _PRINT_KIND_REAL,        _PRINT_ARG_REAL,        _PRINT_BASE_NONE,    _PRINT_STYLE_FIXED,       _PRINT_CASE_LOWER},
    {'F', _PRINT_LENGTHS_REAL,    _PRINT_KIND_REAL,        _PRINT_ARG_REAL,        _PRINT_BASE_NONE,    _PRINT_STYLE_FIXED,       _PRINT_CASE_UPPER},
    {'e', _PRINT_LENGTHS_REAL,    _PRINT_KIND_REAL,        _PRINT_ARG_REAL,        _PRINT_BASE_NONE,    _PRINT_STYLE_EXPONENTIAL, _PRINT_CASE_LOWER},
    {'E', _PRINT_LENGTHS_REAL,    _PRINT_KIND_REAL,        _PRINT_ARG_REAL,        _PRINT_BASE_NONE,    _PRINT_STYLE_EXPONENTIAL, _PRINT_CASE_UPPER},
    {'g', _PRINT_LENGTHS_REAL,    _PRINT_KIND_REAL,        _PRINT_ARG_REAL,        _PRINT_BASE_NONE,    _PRINT_STYLE_GENERAL,     _PRINT_CASE_LOWER},
    {'G', _PRINT_LENGTHS_REAL,    _PRINT_KIND_REAL,        _PRINT_ARG_REAL,        _PRINT_BASE_NONE,    _PRINT_STYLE_GENERAL,     _PRINT_CASE_UPPER},
    {'a', _PRINT_LENGTHS_REAL,    _PRINT_KIND_HEX_REAL,    _PRINT_ARG_REAL,        _PRINT_BASE_NONE,    _PRINT_STYLE_NONE,        _PRINT_CASE_LOWER},
    {'A', _PRINT_LENGTHS_REAL,    _PRINT_KIND_HEX_REAL,    _PRINT_ARG_REAL,        _PRINT_BASE_NONE,    _PRINT_STYLE_NONE,        _PRINT_CASE_UPPER},
    {'c', _PRINT_LENGTHS_NONE,    _PRINT_KIND_CHAR,        _PRINT_ARG_CHAR,        _PRINT_BASE_NONE,    _PRINT_STYLE_NONE,        _PRINT_CASE_LOWER},
    {'c', _PRINT_LENGTHS_WIDE,    _PRINT_KIND_WIDE_CHAR,   _PRINT_ARG_WIDE_CHAR,   _PRINT_BASE_NONE,    _PRINT_STYLE_NONE,        _PRINT_CASE_LOWER},
    {'s', _PRINT_LENGTHS_NONE,    _PRINT_KIND_STRING,      _PRINT_ARG_STRING,      _PRINT_BASE_NONE,    _PRINT_STYLE_NONE,        _PRINT_CASE_LOWER},
    {'s', _PRINT_LENGTHS_WIDE,    _PRINT_KIND_WIDE_STRING, _PRINT_ARG_WIDE_STRING, _PRINT_BASE_NONE,    _PRINT_STYLE_NONE,        _PRINT_CASE_LOWER},
    {'p', _PRINT_LENGTHS_NONE,    _PRINT_KIND_POINTER,     _PRINT_ARG_POINTER,     _PRINT_BASE_HEX,     _PRINT_STYLE_NONE,        _PRINT_CASE_LOWER},
    {'n', _PRINT_LENGTHS_INTEGER, _PRINT_KIND_COUNT,       _PRINT_ARG_DEST,        _PRINT_BASE_NONE,    _PRINT_STYLE_NONE,        _PRINT_CASE_LOWER},
    {'%', _PRINT_LENGTHS_NONE,    _PRINT_KIND_PERCENT,     _PRINT_ARG_NONE,        _PRINT_BASE_NONE,    _PRINT_STYLE_NONE,        _PRINT_CASE_LOWER}
};

// Put len characters of data, counted though the sink's string be full.
void _Print_PutText(_Print_Sink *sink, const char *data, unsigned long len)
{
    unsigned long room;

    if (sink->ps_status == _PRINT_STATUS_FAILED) {
        return;
    }
    if (len > (unsigned long) _PRINT_INT_MAX - sink->ps_count) {
        _Err_Errno = _SYS_EOVERFLOW;
        sink->ps_status = _PRINT_STATUS_FAILED;
        return;
    }
    if (sink->ps_stream != _PRINT_NULL && _File_PutBytes(sink->ps_stream, data, len) != len) {
        sink->ps_status = _PRINT_STATUS_FAILED;
    } else if (sink->ps_stream == _PRINT_NULL && sink->ps_count + 1 < sink->ps_size) {
        room = sink->ps_size - 1 - sink->ps_count;
        _Str_CopyBytes(sink->ps_str + sink->ps_count, data, len < room ? len : room);
    }
    sink->ps_count += len;
}

// Put len copies of the character ch.
void _Print_PutPad(_Print_Sink *sink, int ch, unsigned long len)
{
    char chunk[_PRINT_PAD_CHUNK];

    _Str_FillBytes(chunk, ch, sizeof(chunk));
    while (len > 0 && sink->ps_status == _PRINT_STATUS_GOOD) {
        unsigned long part = len < sizeof(chunk) ? len : sizeof(chunk);

        _Print_PutText(sink, chunk, part);
        len -= part;
    }
}

// End the sink's string, and return its count, or _FILE_END if it failed.
int _Print_EndSink(_Print_Sink *sink)
{
    if (sink->ps_stream == _PRINT_NULL && sink->ps_size > 0) {
        unsigned long last = sink->ps_size - 1;

        sink->ps_str[sink->ps_count < last ? sink->ps_count : last] = '\0';
    }
    return sink->ps_status == _PRINT_STATUS_FAILED ? _FILE_END : (int) sink->ps_count;
}

// Match the text before a directive, and return its length, 0 for none.
unsigned long _Print_MatchText(const char *format)
{
    return _Str_CountUnlisted(format, "%");
}

// Match a directive's percent sign, flags, width, precision and length.
unsigned long _Print_MatchHead(const char *format, _Print_Directive *dir)
{
    int digit;
    unsigned long i = 0;

    if (format[i] != '%') {
        return 0;
    }

    // Phase: match the flags
    dir->pd_flags = 0;
    for (i++; ; i++) {
        if (format[i] == '-') {
            dir->pd_flags |= _PRINT_FLAG_LEFT;
        } else if (format[i] == '+') {
            dir->pd_flags |= _PRINT_FLAG_SIGN;
        } else if (format[i] == ' ') {
            dir->pd_flags |= _PRINT_FLAG_SPACE;
        } else if (format[i] == '#') {
            dir->pd_flags |= _PRINT_FLAG_ALT;
        } else if (format[i] == '0') {
            dir->pd_flags |= _PRINT_FLAG_ZERO;
        } else {
            break;
        }
    }

    // Phase: match the width
    dir->pd_width = 0;
    if (format[i] == '*') {
        dir->pd_width = _PRINT_COUNT_ARGUMENT;
        i++;
    } else {
        for (; format[i] >= '0' && format[i] <= '9'; i++) {
            digit = format[i] - '0';
            dir->pd_width = dir->pd_width > (_PRINT_INT_MAX - digit) / _PRINT_COUNT_BASE ? _PRINT_INT_MAX : dir->pd_width * _PRINT_COUNT_BASE + digit;
        }
    }

    // Phase: match the precision
    dir->pd_precision = _PRINT_PRECISION_NONE;
    if (format[i] == '.' && format[i + 1] == '*') {
        dir->pd_precision = _PRINT_COUNT_ARGUMENT;
        i += 2;
    } else if (format[i] == '.') {
        dir->pd_precision = 0;
        for (i++; format[i] >= '0' && format[i] <= '9'; i++) {
            digit = format[i] - '0';
            dir->pd_precision = dir->pd_precision > (_PRINT_INT_MAX - digit) / _PRINT_COUNT_BASE ? _PRINT_INT_MAX : dir->pd_precision * _PRINT_COUNT_BASE + digit;
        }
    }

    // Phase: match the length
    dir->pd_length = _PRINT_LENGTH_NONE;
    if (format[i] == 'h' && format[i + 1] == 'h') {
        dir->pd_length = _PRINT_LENGTH_CHAR;
        i += 2;
    } else if (format[i] == 'h') {
        dir->pd_length = _PRINT_LENGTH_SHORT;
        i++;
    } else if (format[i] == 'l' && format[i + 1] == 'l') {
        dir->pd_length = _PRINT_LENGTH_LLONG;
        i += 2;
    } else if (format[i] == 'l') {
        dir->pd_length = _PRINT_LENGTH_LONG;
        i++;
    } else if (format[i] == 'j') {
        dir->pd_length = _PRINT_LENGTH_INTMAX;
        i++;
    } else if (format[i] == 'z') {
        dir->pd_length = _PRINT_LENGTH_SIZE;
        i++;
    } else if (format[i] == 't') {
        dir->pd_length = _PRINT_LENGTH_PTRDIFF;
        i++;
    } else if (format[i] == 'L') {
        dir->pd_length = _PRINT_LENGTH_LDOUBLE;
        i++;
    }
    return i;
}

// Find the conversion a letter names with a length, or null if none does.
const _Print_Rule *_Print_FindRule(char letter, _Print_Length length)
{
    for (unsigned long i = 0; i < _PRINT_RULES; i++) {
        if (_Print_Rules[i].pr_letter == letter && (_Print_Rules[i].pr_lengths & (1 << length))) {
            return &_Print_Rules[i];
        }
    }
    return _PRINT_NULL;
}

// Take the width and precision a directive left to its arguments.
void _Print_TakeCounts(_Print_VaList *list, _Print_Directive *dir)
{
    if (dir->pd_width == _PRINT_COUNT_ARGUMENT) {
        dir->pd_width = __builtin_va_arg(*list, int);
    }
    if (dir->pd_width < 0) {
        dir->pd_flags |= _PRINT_FLAG_LEFT;
        dir->pd_width = dir->pd_width == _PRINT_INT_MIN ? _PRINT_INT_MAX : -dir->pd_width;
    }
    if (dir->pd_precision == _PRINT_COUNT_ARGUMENT) {
        dir->pd_precision = __builtin_va_arg(*list, int);
    }
    if (dir->pd_precision < 0) {
        dir->pd_precision = _PRINT_PRECISION_NONE;
    }
}

// Take a directive's argument, and settle the flags its conversion ignores.
void _Print_TakeArgument(_Print_VaList *list, _Print_Arg arg, _Print_Directive *dir)
{
    _Print_IntmaxType value = 0;

    // Phase: clear what the argument does not set
    dir->pd_sign = _PRINT_SIGN_PLUS;
    dir->pd_uval = 0;
    dir->pd_fval = 0;
    dir->pd_str = _PRINT_NULL;
    dir->pd_wstr = _PRINT_NULL;
    dir->pd_dest = _PRINT_NULL;

    // Phase: take a signed integer of the length's type
    if (arg == _PRINT_ARG_SIGNED) {
        switch (dir->pd_length) {
            case _PRINT_LENGTH_CHAR: {
                value = (signed char) __builtin_va_arg(*list, int);
            } break;
            case _PRINT_LENGTH_SHORT: {
                value = (short) __builtin_va_arg(*list, int);
            } break;
            case _PRINT_LENGTH_LLONG: {
                value = __builtin_va_arg(*list, long long);
            } break;
            case _PRINT_LENGTH_INTMAX: {
                value = __builtin_va_arg(*list, _Print_IntmaxType);
            } break;
            case _PRINT_LENGTH_LONG:
            case _PRINT_LENGTH_SIZE:
            case _PRINT_LENGTH_PTRDIFF: {
                value = __builtin_va_arg(*list, long);
            } break;
            default: {
                value = __builtin_va_arg(*list, int);
            } break;
        }
        dir->pd_sign = value < 0 ? _PRINT_SIGN_MINUS : _PRINT_SIGN_PLUS;
        dir->pd_uval = value < 0 ? -(_Print_UintmaxType) value : (_Print_UintmaxType) value;
    }

    // Phase: take an unsigned integer of the length's type, which has no sign
    if (arg == _PRINT_ARG_UNSIGNED) {
        switch (dir->pd_length) {
            case _PRINT_LENGTH_CHAR: {
                dir->pd_uval = (unsigned char) __builtin_va_arg(*list, unsigned);
            } break;
            case _PRINT_LENGTH_SHORT: {
                dir->pd_uval = (unsigned short) __builtin_va_arg(*list, unsigned);
            } break;
            case _PRINT_LENGTH_LLONG: {
                dir->pd_uval = __builtin_va_arg(*list, unsigned long long);
            } break;
            case _PRINT_LENGTH_INTMAX: {
                dir->pd_uval = __builtin_va_arg(*list, _Print_UintmaxType);
            } break;
            case _PRINT_LENGTH_LONG:
            case _PRINT_LENGTH_SIZE:
            case _PRINT_LENGTH_PTRDIFF: {
                dir->pd_uval = __builtin_va_arg(*list, unsigned long);
            } break;
            default: {
                dir->pd_uval = __builtin_va_arg(*list, unsigned);
            } break;
        }
        dir->pd_flags &= ~(_PRINT_FLAG_SIGN | _PRINT_FLAG_SPACE);
    }

    // Phase: take an argument of one type, a pointer written as by %#lx
    switch (arg) {
        case _PRINT_ARG_CHAR: {
            dir->pd_uval = (unsigned char) __builtin_va_arg(*list, int);
        } break;
        case _PRINT_ARG_STRING: {
            dir->pd_str = __builtin_va_arg(*list, const char *);
        } break;
        case _PRINT_ARG_POINTER: {
            dir->pd_uval = (unsigned long) __builtin_va_arg(*list, const void *);
            dir->pd_flags |= _PRINT_FLAG_ALT;
        } break;
        case _PRINT_ARG_DEST: {
            dir->pd_dest = __builtin_va_arg(*list, void *);
        } break;
        case _PRINT_ARG_REAL: {
            dir->pd_fval = dir->pd_length == _PRINT_LENGTH_LDOUBLE ? __builtin_va_arg(*list, long double) : __builtin_va_arg(*list, double);
        } break;
        case _PRINT_ARG_WIDE_CHAR: {
            dir->pd_uval = (_Print_UintmaxType) (_Print_WcharType) __builtin_va_arg(*list, _Print_WintType);
        } break;
        case _PRINT_ARG_WIDE_STRING: {
            dir->pd_wstr = __builtin_va_arg(*list, const _Print_WcharType *);
        } break;
        default: {
            // empty
        } break;
    }
}

// Match a directive with its arguments, and return its length, 0 for none.
unsigned long _Print_MatchDirective(const char *format, _Print_VaList *list, _Print_Directive *dir)
{
    const _Print_Rule *rule;
    unsigned long at = _Print_MatchHead(format, dir);

    // Phase: find its conversion, before any argument is taken
    if (at == 0) {
        return 0;
    }
    rule = _Print_FindRule(format[at], dir->pd_length);
    if (rule == _PRINT_NULL) {
        return 0;
    }
    dir->pd_kind = rule->pr_kind;
    dir->pd_base = rule->pr_base;
    dir->pd_style = rule->pr_style;
    dir->pd_case = rule->pr_case;

    // Phase: take its arguments, in the format's order
    _Print_TakeCounts(list, dir);
    _Print_TakeArgument(list, rule->pr_arg, dir);
    return at + 1;
}

// Write a value's digits in a base to end at end, none for zero.
unsigned long _Print_ToDigits(char *end, _Print_UintmaxType value, int base, const char *digits)
{
    char *ptr = end;

    while (value != 0) {
        ptr--;
        ptr[0] = digits[value % (_Print_UintmaxType) base];
        value /= (_Print_UintmaxType) base;
    }
    return (unsigned long) (end - ptr);
}

// Unpack a floating value into a sign and its bits by a power of two.
_Print_Real _Print_ToParts(long double value, _Print_Sign *sign, unsigned long long *mant, int *exp)
{
    unsigned short top;
    unsigned int biased;
    _Print_Real kind = _PRINT_REAL_FINITE;
    unsigned int ones = (unsigned int) _PRINT_LDOUBLE_EXP * 2 - 1;
    unsigned long long lead = 1ULL << (_PRINT_LDOUBLE_MANT - 1);

    _Str_CopyBytes(mant, &value, sizeof(*mant));
    _Str_CopyBytes(&top, (char *) &value + sizeof(*mant), sizeof(top));
    biased = top & ones;
    *exp = (int) (biased != 0 ? biased : 1) - (_PRINT_LDOUBLE_EXP - 1) - (_PRINT_LDOUBLE_MANT - 1);
    *sign = top > ones ? _PRINT_SIGN_MINUS : _PRINT_SIGN_PLUS;
    if (biased == ones) {
        kind = (*mant & (lead - 1)) != 0 ? _PRINT_REAL_NAN : _PRINT_REAL_INFINITE;
    }
    if (biased != 0 && ! (*mant & lead)) {
        kind = _PRINT_REAL_NAN;
    }
    return kind;
}

// Write a floating value's sign, what it is and its every decimal digit.
void _Print_ToNumeral(long double value, _Print_Numeral *num)
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
    char *end = num->pn_digit + sizeof(num->pn_digit);
    char *ptr = end;

    // Phase: take the value apart
    num->pn_kind = _Print_ToParts(value, &num->pn_sign, &mant, &exp);
    num->pn_len = 0;
    num->pn_exp = 0;
    if (num->pn_kind != _PRINT_REAL_FINITE) {
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
            ptr[0] = (char) ('0' + chunk % _PRINT_BASE_DECIMAL);
            chunk /= _PRINT_BASE_DECIMAL;
        }
    }
    while (ptr < end && ptr[0] == '0') {
        ptr++;
    }
    num->pn_len = (unsigned long) (end - ptr);
    num->pn_exp = num->pn_len > 0 ? (int) num->pn_len - 1 : 0;
    _Str_MoveBytes(num->pn_digit, ptr, num->pn_len);

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
            run[i] = (char) ('0' + part % _PRINT_BASE_DECIMAL);
            part /= _PRINT_BASE_DECIMAL;
        }
        for (i = 0; i < step; i++) {
            place++;
            if (num->pn_len == 0 && run[i] != '0') {
                num->pn_exp = -place;
            }
            if (num->pn_len > 0 || run[i] != '0') {
                num->pn_digit[num->pn_len++] = run[i];
            }
        }
    }

    // Phase: drop the zeros at the end
    while (num->pn_len > 0 && num->pn_digit[num->pn_len - 1] == '0') {
        num->pn_len--;
    }
}

// Round num to its first keep digits, to nearest and a tie to even.
void _Print_RoundNumeral(_Print_Numeral *num, long keep)
{
    _Bool up;
    unsigned long len;
    char *digit = num->pn_digit;

    if (keep >= (long) num->pn_len) {
        return;
    }
    len = keep > 0 ? (unsigned long) keep : 0;
    up = keep >= 0 && digit[len] >= '5';
    if (up && digit[len] == '5' && len + 1 == num->pn_len) {
        up = len > 0 && (digit[len - 1] - '0') % 2 != 0;
    }
    while (up && len > 0 && digit[len - 1] == '9') {
        len--;
    }
    if (up && len == 0) {
        digit[0] = '1';
        len = 1;
        num->pn_exp++;
    } else if (up) {
        digit[len - 1]++;
    }
    while (len > 0 && digit[len - 1] == '0') {
        len--;
    }
    num->pn_len = len;
    if (len == 0) {
        num->pn_exp = 0;
    }
}

// Store a count at dest as an integer of the type a length names.
void _Print_StoreCount(void *dest, _Print_Length length, unsigned long count)
{
    switch (length) {
        case _PRINT_LENGTH_CHAR: {
            *(signed char *) dest = (signed char) count;
        } break;
        case _PRINT_LENGTH_SHORT: {
            *(short *) dest = (short) count;
        } break;
        case _PRINT_LENGTH_LLONG: {
            *(long long *) dest = (long long) count;
        } break;
        case _PRINT_LENGTH_INTMAX: {
            *(_Print_IntmaxType *) dest = (_Print_IntmaxType) count;
        } break;
        case _PRINT_LENGTH_SIZE: {
            *(unsigned long *) dest = count;
        } break;
        case _PRINT_LENGTH_LONG:
        case _PRINT_LENGTH_PTRDIFF: {
            *(long *) dest = (long) count;
        } break;
        default: {
            *(int *) dest = (int) count;
        } break;
    }
}

// Write len characters of data in a width.
void _Print_WriteField(_Print_Sink *sink, int flags, int width, const char *data, unsigned long len)
{
    unsigned long pad = (unsigned long) width > len ? (unsigned long) width - len : 0;

    if (! (flags & _PRINT_FLAG_LEFT)) {
        _Print_PutPad(sink, ' ', pad);
    }
    _Print_PutText(sink, data, len);
    if (flags & _PRINT_FLAG_LEFT) {
        _Print_PutPad(sink, ' ', pad);
    }
}

// Write what leads a number, and return the spaces to follow it.
unsigned long _Print_WriteLead(_Print_Sink *sink, int flags, int width, _Print_Sign sign, const char *prefix, unsigned long len)
{
    unsigned long pad;
    unsigned long lead;
    const char *text = "";

    if (sign == _PRINT_SIGN_MINUS) {
        text = "-";
    } else if (flags & _PRINT_FLAG_SIGN) {
        text = "+";
    } else if (flags & _PRINT_FLAG_SPACE) {
        text = " ";
    }
    if (flags & _PRINT_FLAG_LEFT) {
        flags &= ~_PRINT_FLAG_ZERO;
    }
    lead = _Str_GetLength(text) + _Str_GetLength(prefix);
    pad = (unsigned long) width > lead + len ? (unsigned long) width - lead - len : 0;
    if (! (flags & (_PRINT_FLAG_LEFT | _PRINT_FLAG_ZERO))) {
        _Print_PutPad(sink, ' ', pad);
    }
    _Print_PutText(sink, text, _Str_GetLength(text));
    _Print_PutText(sink, prefix, _Str_GetLength(prefix));
    if (flags & _PRINT_FLAG_ZERO) {
        _Print_PutPad(sink, '0', pad);
    }
    return flags & _PRINT_FLAG_LEFT ? pad : 0;
}

// Write count digits of num from the one worth 10^high, zeros for none.
void _Print_WriteSpan(_Print_Sink *sink, const _Print_Numeral *num, long high, unsigned long count)
{
    unsigned long real;
    unsigned long lead = high > num->pn_exp ? (unsigned long) (high - num->pn_exp) : 0;
    unsigned long skip = high < num->pn_exp ? (unsigned long) (num->pn_exp - high) : 0;

    if (lead > count) {
        lead = count;
    }
    count -= lead;
    real = skip < num->pn_len ? num->pn_len - skip : 0;
    if (real > count) {
        real = count;
    }
    _Print_PutPad(sink, '0', lead);
    _Print_PutText(sink, num->pn_digit + skip, real);
    _Print_PutPad(sink, '0', count - real);
}

// Write a rounded numeral in a width, frac digits after its point.
void _Print_WriteNumeral(_Print_Sink *sink, int flags, int width, const _Print_Numeral *num, unsigned long frac, const char *mark)
{
    unsigned long rest;
    char point = _PRINT_POINT;
    char text[_PRINT_DIGITS_MAX];
    _Bool powered = mark[0] != '\0';
    long high = powered || num->pn_exp > 0 ? num->pn_exp : 0;
    unsigned long whole = powered ? 1 : (unsigned long) high + 1;
    unsigned long dot = frac > 0 || (flags & _PRINT_FLAG_ALT) ? 1 : 0;
    _Print_UintmaxType power = (_Print_UintmaxType) (num->pn_exp < 0 ? -num->pn_exp : num->pn_exp);
    unsigned long count = _Print_ToDigits(text + sizeof(text), power, _PRINT_BASE_DECIMAL, _PRINT_DIGITS_LOWER);
    unsigned long zeros = count < _PRINT_EXPONENT_MIN ? _PRINT_EXPONENT_MIN - count : 0;
    unsigned long tail = powered ? _Str_GetLength(mark) + 1 + zeros + count : 0;

    rest = _Print_WriteLead(sink, flags, width, num->pn_sign, "", whole + dot + frac + tail);
    _Print_WriteSpan(sink, num, high, whole);
    _Print_PutText(sink, &point, dot);
    _Print_WriteSpan(sink, num, high - (long) whole, frac);
    if (powered) {
        _Print_PutText(sink, mark, _Str_GetLength(mark));
        _Print_PutText(sink, num->pn_exp < 0 ? "-" : "+", 1);
        _Print_PutPad(sink, '0', zeros);
        _Print_PutText(sink, text + sizeof(text) - count, count);
    }
    _Print_PutPad(sink, ' ', rest);
}

// Write an infinity or a NaN in a width, a sign and a text, no zeros.
void _Print_WriteSpecial(_Print_Sink *sink, const _Print_Directive *dir, _Print_Sign sign, _Print_Real kind)
{
    unsigned long rest;
    const char *inf = dir->pd_case == _PRINT_CASE_UPPER ? _ERR_PRINT_INF_UPPER : _ERR_PRINT_INF;
    const char *nan = dir->pd_case == _PRINT_CASE_UPPER ? _ERR_PRINT_NAN_UPPER : _ERR_PRINT_NAN;
    const char *text = kind == _PRINT_REAL_NAN ? nan : inf;

    rest = _Print_WriteLead(sink, dir->pd_flags & ~_PRINT_FLAG_ZERO, dir->pd_width, sign, "", _Str_GetLength(text));
    _Print_PutText(sink, text, _Str_GetLength(text));
    _Print_PutPad(sink, ' ', rest);
}

// Write a mantissa in hex, its top four bits first, by a power of two.
void _Print_WriteMantissa(_Print_Sink *sink, const _Print_Directive *dir, _Print_Sign sign, unsigned long long mant, int exp)
{
    unsigned long dot;
    unsigned long kept;
    unsigned long rest;
    unsigned long zeros;
    unsigned long count;
    unsigned long long unit;
    unsigned long long below;
    char point = _PRINT_POINT;
    char text[_PRINT_DIGITS_MAX];
    int precision = dir->pd_precision;
    char digits[_PRINT_HEX_DIGITS];
    int last = _PRINT_HEX_DIGITS - 1;
    _Bool upper = dir->pd_case == _PRINT_CASE_UPPER;

    // Phase: round to the precision, or find the least that is exact
    if (precision == _PRINT_PRECISION_NONE) {
        for (precision = last; precision > 0 && (mant >> (_PRINT_HEX_BITS * (last - precision))) % _PRINT_BASE_HEX == 0; precision--) {
            // empty
        }
    }
    if (precision < last) {
        unit = 1ULL << (_PRINT_HEX_BITS * (last - precision));
        below = mant & (unit - 1);
        mant -= below;
        if (below > unit / 2 || (below == unit / 2 && (mant & unit) != 0)) {
            mant += unit;
            if (mant < unit) {
                mant = 1ULL << (_PRINT_HEX_BITS * last);
                exp += _PRINT_HEX_BITS;
            }
        }
    }

    // Phase: write its digits and its power
    dot = precision > 0 || (dir->pd_flags & _PRINT_FLAG_ALT) ? 1 : 0;
    _Str_FillBytes(digits, '0', sizeof(digits));
    _Print_ToDigits(digits + sizeof(digits), mant, _PRINT_BASE_HEX, upper ? _PRINT_DIGITS_UPPER : _PRINT_DIGITS_LOWER);
    count = _Print_ToDigits(text + sizeof(text), (_Print_UintmaxType) (exp < 0 ? -exp : exp), _PRINT_BASE_DECIMAL, _PRINT_DIGITS_LOWER);
    zeros = count < _PRINT_POWER_MIN ? _PRINT_POWER_MIN - count : 0;
    kept = precision < last ? (unsigned long) precision : (unsigned long) last;
    rest = _Print_WriteLead(sink, dir->pd_flags, dir->pd_width, sign, upper ? _PRINT_PREFIX_UPPER : _PRINT_PREFIX_LOWER, 1 + dot + (unsigned long) precision + 2 + zeros + count);
    _Print_PutText(sink, digits, 1);
    _Print_PutText(sink, &point, dot);
    _Print_PutText(sink, digits + 1, kept);
    _Print_PutPad(sink, '0', (unsigned long) precision - kept);
    _Print_PutText(sink, upper ? "P" : "p", 1);
    _Print_PutText(sink, exp < 0 ? "-" : "+", 1);
    _Print_PutPad(sink, '0', zeros);
    _Print_PutText(sink, text + sizeof(text) - count, count);
    _Print_PutPad(sink, ' ', rest);
}

// Write a string as %s converts it, a null one as glibc does.
void _Print_WriteString(_Print_Sink *sink, const _Print_Directive *dir)
{
    const char *end;
    unsigned long len;
    const char *str = dir->pd_str;
    int precision = dir->pd_precision;

    if (str == _PRINT_NULL) {
        str = precision == _PRINT_PRECISION_NONE || precision >= (int) sizeof(_ERR_PRINT_NULL) - 1 ? _ERR_PRINT_NULL : "";
    }
    if (precision == _PRINT_PRECISION_NONE) {
        len = _Str_GetLength(str);
    } else {
        end = _Str_FindByte(str, '\0', (unsigned long) precision);
        len = end != _PRINT_NULL ? (unsigned long) (end - str) : (unsigned long) precision;
    }
    _Print_WriteField(sink, dir->pd_flags, dir->pd_width, str, len);
}

// Write an integer in its base, its precision the fewest digits.
void _Print_WriteInteger(_Print_Sink *sink, const _Print_Directive *dir)
{
    unsigned long rest;
    unsigned long zeros;
    const char *prefix = "";
    int flags = dir->pd_flags;
    char text[_PRINT_DIGITS_MAX];
    int precision = dir->pd_precision;
    _Bool upper = dir->pd_case == _PRINT_CASE_UPPER;
    const char *digits = upper ? _PRINT_DIGITS_UPPER : _PRINT_DIGITS_LOWER;
    unsigned long len = _Print_ToDigits(text + sizeof(text), dir->pd_uval, dir->pd_base, digits);

    // Phase: settle the precision, which a zero flag gives way to
    if (precision == _PRINT_PRECISION_NONE) {
        precision = _PRINT_INTEGER_PRECISION;
    } else {
        flags &= ~_PRINT_FLAG_ZERO;
    }
    zeros = (unsigned long) precision > len ? (unsigned long) precision - len : 0;

    // Phase: pick what # leads an octal and a hex number with
    if ((flags & _PRINT_FLAG_ALT) && dir->pd_base == _PRINT_BASE_OCTAL && zeros == 0) {
        prefix = _PRINT_PREFIX_OCTAL;
    } else if ((flags & _PRINT_FLAG_ALT) && dir->pd_base == _PRINT_BASE_HEX && dir->pd_uval != 0) {
        prefix = upper ? _PRINT_PREFIX_UPPER : _PRINT_PREFIX_LOWER;
    }

    // Phase: write it in the width
    rest = _Print_WriteLead(sink, flags, dir->pd_width, dir->pd_sign, prefix, zeros + len);
    _Print_PutPad(sink, '0', zeros);
    _Print_PutText(sink, text + sizeof(text) - len, len);
    _Print_PutPad(sink, ' ', rest);
}

// Write a floating value in decimal, as %f, %e or %g converts it.
void _Print_WriteReal(_Print_Sink *sink, const _Print_Directive *dir)
{
    _Bool powered;
    long frac = 0;
    _Print_Numeral num;
    const char *mark = "";
    int precision = dir->pd_precision;
    const char *exponent = dir->pd_case == _PRINT_CASE_UPPER ? "E" : "e";

    _Print_ToNumeral(dir->pd_fval, &num);
    if (num.pn_kind != _PRINT_REAL_FINITE) {
        _Print_WriteSpecial(sink, dir, num.pn_sign, num.pn_kind);
        return;
    }
    if (precision == _PRINT_PRECISION_NONE) {
        precision = _PRINT_REAL_PRECISION;
    }
    switch (dir->pd_style) {
        case _PRINT_STYLE_FIXED: {
            _Print_RoundNumeral(&num, (long) num.pn_exp + 1 + precision);
            frac = precision;
        } break;
        case _PRINT_STYLE_EXPONENTIAL: {
            _Print_RoundNumeral(&num, (long) precision + 1);
            frac = precision;
            mark = exponent;
        } break;
        default: {
            if (precision == 0) {
                precision = _PRINT_REAL_LEAST;
            }
            _Print_RoundNumeral(&num, precision);
            powered = num.pn_exp < _PRINT_GENERAL_MIN || num.pn_exp >= precision;
            frac = (dir->pd_flags & _PRINT_FLAG_ALT ? precision : (long) num.pn_len) - 1;
            if (powered) {
                mark = exponent;
            } else {
                frac -= num.pn_exp;
            }
        } break;
    }
    _Print_WriteNumeral(sink, dir->pd_flags, dir->pd_width, &num, frac > 0 ? (unsigned long) frac : 0, mark);
}

// Write a floating value in hex, a double's first digit a 1 or a 0.
void _Print_WriteHexReal(_Print_Sink *sink, const _Print_Directive *dir)
{
    int exp;
    int under;
    _Print_Sign sign;
    unsigned long long mant;
    _Print_Real kind = _Print_ToParts(dir->pd_fval, &sign, &mant, &exp);

    if (kind != _PRINT_REAL_FINITE) {
        _Print_WriteSpecial(sink, dir, sign, kind);
        return;
    }
    if (dir->pd_length == _PRINT_LENGTH_LDOUBLE) {
        exp = mant != 0 ? exp + _PRINT_LDOUBLE_MANT - _PRINT_HEX_BITS : 0;
    } else {
        exp = mant != 0 ? exp + _PRINT_LDOUBLE_MANT - 1 : 0;
        under = exp < _PRINT_DOUBLE_MIN - 1 ? _PRINT_DOUBLE_MIN - 1 - exp : 0;
        mant = mant >> (_PRINT_HEX_BITS - 1) >> under;
        exp += under;
    }
    _Print_WriteMantissa(sink, dir, sign, mant, exp);
}

// Write a wide string as %ls converts it, in the locale's bytes.
void _Print_WriteWideString(_Print_Sink *sink, const _Print_Directive *dir)
{
    int n;
    unsigned long i;
    unsigned long pad;
    unsigned long len = 0;
    unsigned long count = 0;
    char bytes[_PRINT_BYTES_MAX];
    const _Print_WcharType *wstr = dir->pd_wstr;
    unsigned long max = dir->pd_precision == _PRINT_PRECISION_NONE ? _PRINT_SIZE_MAX : (unsigned long) dir->pd_precision;

    if (wstr == _PRINT_NULL) {
        _Print_WriteString(sink, dir);
        return;
    }

    // Phase: count the characters whose bytes the precision holds
    for (; len < max && wstr[count] != L'\0'; count++) {
        n = _Utf8_EncodeChar(wstr[count], bytes);
        if (n < 0) {
            _Err_Errno = _SYS_EILSEQ;
            sink->ps_status = _PRINT_STATUS_FAILED;
            return;
        }
        if ((unsigned long) n > max - len) {
            break;
        }
        len += (unsigned long) n;
    }

    // Phase: write them in the width
    pad = (unsigned long) dir->pd_width > len ? (unsigned long) dir->pd_width - len : 0;
    if (! (dir->pd_flags & _PRINT_FLAG_LEFT)) {
        _Print_PutPad(sink, ' ', pad);
    }
    for (i = 0; i < count; i++) {
        n = _Utf8_EncodeChar(wstr[i], bytes);
        _Print_PutText(sink, bytes, (unsigned long) n);
    }
    if (dir->pd_flags & _PRINT_FLAG_LEFT) {
        _Print_PutPad(sink, ' ', pad);
    }
}

// Write what a directive converts, by its kind.
void _Print_WriteDirective(_Print_Sink *sink, const _Print_Directive *dir)
{
    switch (dir->pd_kind) {
        case _PRINT_KIND_PERCENT: {
            _Print_PutText(sink, "%", 1);
        } break;
        case _PRINT_KIND_CHAR: {
            char ch = (char) dir->pd_uval;
            _Print_WriteField(sink, dir->pd_flags, dir->pd_width, &ch, 1);
        } break;
        case _PRINT_KIND_STRING: {
            _Print_WriteString(sink, dir);
        } break;
        case _PRINT_KIND_INTEGER: {
            _Print_WriteInteger(sink, dir);
        } break;
        case _PRINT_KIND_POINTER: {
            if (dir->pd_uval == 0) {
                _Print_WriteField(sink, dir->pd_flags, dir->pd_width, _ERR_PRINT_NIL, sizeof(_ERR_PRINT_NIL) - 1);
            } else {
                _Print_WriteInteger(sink, dir);
            }
        } break;
        case _PRINT_KIND_COUNT: {
            _Print_StoreCount(dir->pd_dest, dir->pd_length, sink->ps_count);
        } break;
        case _PRINT_KIND_REAL: {
            _Print_WriteReal(sink, dir);
        } break;
        case _PRINT_KIND_HEX_REAL: {
            _Print_WriteHexReal(sink, dir);
        } break;
        case _PRINT_KIND_WIDE_CHAR: {
            _Print_Directive one = *dir;
            _Print_WcharType pair[2] = {(_Print_WcharType) dir->pd_uval, L'\0'};
            one.pd_wstr = pair;
            one.pd_precision = _PRINT_PRECISION_NONE;
            _Print_WriteWideString(sink, &one);
        } break;
        case _PRINT_KIND_WIDE_STRING: {
            _Print_WriteWideString(sink, dir);
        } break;
    }
}

// Write the format's text, each directive converting its arguments.
int _Print_WriteFormat(_Print_Sink *sink, const char *format, _Print_VaList args)
{
    unsigned long len;
    _Print_VaList list;
    _Print_Directive dir;

    __builtin_va_copy(list, args);
    for (unsigned long i = 0; format[i] != '\0'; i += len) {
        if ((len = _Print_MatchText(format + i)) > 0) {
            _Print_PutText(sink, format + i, len);
        } else if ((len = _Print_MatchDirective(format + i, &list, &dir)) > 0) {
            _Print_WriteDirective(sink, &dir);
        } else {
            len = 1;
            _Print_PutText(sink, format + i, len);
        }
    }
    __builtin_va_end(list);
    return _Print_EndSink(sink);
}
