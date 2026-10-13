/*
 * C source file for formatted input.
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
#include <_scan.h>

// The error a character without all its bytes reports.
#include <_sys.h>

// The error number.
#include <_err.h>

// Finding a character among those an item may have.
#include <_str.h>

// The character some bytes are, and the most bytes one has.
#include <_utf8.h>

// The array an input item of any length grows in.
#include <_mem.h>

// The integer an input item is.
#include <_int.h>

// The floating number an input item is.
#include <_fp.h>

// The conversions, a letter that means two things by its length being two.
const _Scan_Rule _Scan_Rules[_SCAN_RULES] = {
    {'d', _SCAN_LENGTHS_INTEGER, _SCAN_KIND_INTEGER, _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_DECIMAL},
    {'i', _SCAN_LENGTHS_INTEGER, _SCAN_KIND_INTEGER, _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'o', _SCAN_LENGTHS_INTEGER, _SCAN_KIND_INTEGER, _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_UNSIGNED, _SCAN_BASE_OCTAL},
    {'u', _SCAN_LENGTHS_INTEGER, _SCAN_KIND_INTEGER, _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_UNSIGNED, _SCAN_BASE_DECIMAL},
    {'x', _SCAN_LENGTHS_INTEGER, _SCAN_KIND_INTEGER, _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_UNSIGNED, _SCAN_BASE_HEX},
    {'X', _SCAN_LENGTHS_INTEGER, _SCAN_KIND_INTEGER, _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_UNSIGNED, _SCAN_BASE_HEX},
    {'a', _SCAN_LENGTHS_REAL,    _SCAN_KIND_REAL,    _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'A', _SCAN_LENGTHS_REAL,    _SCAN_KIND_REAL,    _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'e', _SCAN_LENGTHS_REAL,    _SCAN_KIND_REAL,    _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'E', _SCAN_LENGTHS_REAL,    _SCAN_KIND_REAL,    _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'f', _SCAN_LENGTHS_REAL,    _SCAN_KIND_REAL,    _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'F', _SCAN_LENGTHS_REAL,    _SCAN_KIND_REAL,    _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'g', _SCAN_LENGTHS_REAL,    _SCAN_KIND_REAL,    _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'G', _SCAN_LENGTHS_REAL,    _SCAN_KIND_REAL,    _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'c', _SCAN_LENGTHS_NONE,    _SCAN_KIND_CHARS,   _SCAN_CHARS_NARROW, _SCAN_SPACE_KEEP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'c', _SCAN_LENGTHS_WIDE,    _SCAN_KIND_CHARS,   _SCAN_CHARS_WIDE,   _SCAN_SPACE_KEEP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'s', _SCAN_LENGTHS_NONE,    _SCAN_KIND_STRING,  _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'s', _SCAN_LENGTHS_WIDE,    _SCAN_KIND_STRING,  _SCAN_CHARS_WIDE,   _SCAN_SPACE_SKIP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'[', _SCAN_LENGTHS_NONE,    _SCAN_KIND_SET,     _SCAN_CHARS_NARROW, _SCAN_SPACE_KEEP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'[', _SCAN_LENGTHS_WIDE,    _SCAN_KIND_SET,     _SCAN_CHARS_WIDE,   _SCAN_SPACE_KEEP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'p', _SCAN_LENGTHS_NONE,    _SCAN_KIND_POINTER, _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_UNSIGNED, _SCAN_BASE_HEX},
    {'n', _SCAN_LENGTHS_INTEGER, _SCAN_KIND_COUNT,   _SCAN_CHARS_NARROW, _SCAN_SPACE_KEEP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX},
    {'%', _SCAN_LENGTHS_NONE,    _SCAN_KIND_PERCENT, _SCAN_CHARS_NARROW, _SCAN_SPACE_SKIP, _SCAN_SIGN_SIGNED,   _SCAN_BASE_PREFIX}
};

// Check whether ch is white space in the "C" locale, which no end of input is.
_Bool _Scan_IsSpace(int ch)
{
    return ch == ' ' || (ch >= '\t' && ch <= '\r');
}

// Check whether ch is a decimal digit.
_Bool _Scan_IsDigit(int ch)
{
    return ch >= '0' && ch <= '9';
}

// Check whether ch is a hex digit.
_Bool _Scan_IsHexDigit(int ch)
{
    return _Scan_IsDigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

// Check whether ch is in a scanset's count characters, a-z being a range.
_Bool _Scan_IsInSet(const char *set, unsigned long count, int ch)
{
    unsigned long i;
    const unsigned char *list = (const unsigned char *) set;

    for (i = 0; i < count; i++) {
        if (i > 0 && i + 1 < count && list[i] == '-' && list[i - 1] <= list[i + 1]) {
            if (ch >= list[i - 1] && ch < list[i + 1]) {
                break;
            }
        } else if (ch == list[i]) {
            break;
        }
    }
    return i < count;
}

// Check whether a directive of characters takes ch: %c any, %s none of space.
_Bool _Scan_IsTaken(const _Scan_Directive *dir, int ch)
{
    _Bool space = _Scan_IsSpace(ch);
    _Bool listed = _Scan_IsInSet(dir->sd_set, dir->sd_count, ch);

    if (dir->sd_kind == _SCAN_KIND_STRING) {
        return ! space;
    }
    return dir->sd_kind != _SCAN_KIND_SET || listed == (dir->sd_accept == _SCAN_ACCEPT_LISTED);
}

// Convert a capital to its small letter, any other character to itself.
int _Scan_ToLower(int ch)
{
    return ch >= 'A' && ch <= 'Z' ? ch - 'A' + 'a' : ch;
}

// Convert the integer str begins with, in a base, to a signed value.
long _Scan_ToSigned(const char *str, int base)
{
    _Int_Sign sign;
    _Int_Range range;
    unsigned long end;
    unsigned long value = _Int_ToNumber(str, _INT_TEXT_NARROW, &end, base, &sign, &range);

    return _Int_MakeSigned(value, sign, range);
}

// Convert the integer str begins with, in a base, to an unsigned value.
unsigned long _Scan_ToUnsigned(const char *str, int base)
{
    _Int_Sign sign;
    _Int_Range range;
    unsigned long end;
    unsigned long value = _Int_ToNumber(str, _INT_TEXT_NARROW, &end, base, &sign, &range);

    return _Int_MakeUnsigned(value, sign, range);
}

// Convert all of str to a length's floating type, stored at dest if not null.
_Bool _Scan_ToReal(const char *str, _Scan_Length length, void *dest)
{
    _Bool whole;
    float single = 0;
    double real = 0;
    unsigned long end;
    long double extended = 0;

    if (length == _SCAN_LENGTH_LONG) {
        real = _Fp_ToDouble(str, _FP_TEXT_NARROW, &end);
    } else if (length == _SCAN_LENGTH_LDOUBLE) {
        extended = _Fp_ToLongDouble(str, _FP_TEXT_NARROW, &end);
    } else {
        single = _Fp_ToFloat(str, _FP_TEXT_NARROW, &end);
    }
    whole = end > 0 && str[end] == '\0';
    if (whole && dest != _SCAN_NULL && length == _SCAN_LENGTH_LONG) {
        *(double *) dest = real;
    } else if (whole && dest != _SCAN_NULL && length == _SCAN_LENGTH_LDOUBLE) {
        *(long double *) dest = extended;
    } else if (whole && dest != _SCAN_NULL) {
        *(float *) dest = single;
    }
    return whole;
}

// Peek at the source's next character, left unread, or _FILE_END for none.
int _Scan_PeekSource(_Scan_Source *source)
{
    char ch;

    if (source->ss_stream != _SCAN_NULL) {
        return _File_PeekChar(source->ss_stream);
    }
    ch = source->ss_str[source->ss_count];
    return ch != '\0' ? (unsigned char) ch : _FILE_END;
}

// Advance the source past the character peeked at, which counts as read.
void _Scan_AdvanceSource(_Scan_Source *source)
{
    if (source->ss_stream != _SCAN_NULL) {
        _File_SkipChar(source->ss_stream);
    }
    source->ss_count++;
}

// Fail the source at ch, which is its end or a character that does not fit.
void _Scan_FailSource(_Scan_Source *source, int ch)
{
    source->ss_failure = ch == _FILE_END ? _SCAN_FAILURE_INPUT : _SCAN_FAILURE_MATCHING;
}

// End the source's input, and return its assignments, or _FILE_END for none.
int _Scan_EndSource(_Scan_Source *source)
{
    if (source->ss_failure == _SCAN_FAILURE_INPUT && source->ss_stored == 0) {
        return _FILE_END;
    }
    return source->ss_stored;
}

// Match a directive of white space, and return its length, 0 for none.
unsigned long _Scan_MatchSpace(const char *format)
{
    unsigned long i = 0;

    while (_Scan_IsSpace((unsigned char) format[i])) {
        i++;
    }
    return i;
}

// Match the ordinary characters up to white space or a directive.
unsigned long _Scan_MatchText(const char *format)
{
    unsigned long i = 0;

    while (format[i] != '\0' && format[i] != '%' && ! _Scan_IsSpace((unsigned char) format[i])) {
        i++;
    }
    return i;
}

// Match a directive's percent sign, asterisk, width and length.
unsigned long _Scan_MatchHead(const char *format, _Scan_Assign *assign, _Scan_Directive *dir)
{
    int digit;
    unsigned long from;
    unsigned long i = 0;

    if (format[i] != '%') {
        return 0;
    }
    i++;

    // Phase: match the asterisk
    *assign = _SCAN_ASSIGN_STORE;
    if (format[i] == '*') {
        *assign = _SCAN_ASSIGN_SUPPRESS;
        i++;
    }

    // Phase: match the width, which may not be zero
    dir->sd_width = _SCAN_WIDTH_NONE;
    for (from = i; format[i] >= '0' && format[i] <= '9'; i++) {
        digit = format[i] - '0';
        dir->sd_width = dir->sd_width > (_SCAN_INT_MAX - digit) / _SCAN_COUNT_BASE ? _SCAN_INT_MAX : dir->sd_width * _SCAN_COUNT_BASE + digit;
    }
    if (i > from && dir->sd_width == 0) {
        return 0;
    }

    // Phase: match the length
    dir->sd_length = _SCAN_LENGTH_NONE;
    if (format[i] == 'h' && format[i + 1] == 'h') {
        dir->sd_length = _SCAN_LENGTH_CHAR;
        i += 2;
    } else if (format[i] == 'h') {
        dir->sd_length = _SCAN_LENGTH_SHORT;
        i++;
    } else if (format[i] == 'l' && format[i + 1] == 'l') {
        dir->sd_length = _SCAN_LENGTH_LLONG;
        i += 2;
    } else if (format[i] == 'l') {
        dir->sd_length = _SCAN_LENGTH_LONG;
        i++;
    } else if (format[i] == 'j') {
        dir->sd_length = _SCAN_LENGTH_INTMAX;
        i++;
    } else if (format[i] == 'z') {
        dir->sd_length = _SCAN_LENGTH_SIZE;
        i++;
    } else if (format[i] == 't') {
        dir->sd_length = _SCAN_LENGTH_PTRDIFF;
        i++;
    } else if (format[i] == 'L') {
        dir->sd_length = _SCAN_LENGTH_LDOUBLE;
        i++;
    }
    return i;
}

// Match a scanset after its left bracket, a right one first being one of it.
unsigned long _Scan_MatchSet(const char *format, _Scan_Directive *dir)
{
    unsigned long end;
    unsigned long from = 0;

    dir->sd_accept = _SCAN_ACCEPT_LISTED;
    if (format[from] == '^') {
        dir->sd_accept = _SCAN_ACCEPT_UNLISTED;
        from++;
    }
    end = from;
    if (format[end] == ']') {
        end++;
    }
    while (format[end] != '\0' && format[end] != ']') {
        end++;
    }
    if (format[end] != ']') {
        return 0;
    }
    dir->sd_set = format + from;
    dir->sd_count = end - from;
    return end + 1;
}

// Find the conversion a letter names with a length, or null if none does.
const _Scan_Rule *_Scan_FindRule(char letter, _Scan_Length length)
{
    for (unsigned long i = 0; i < _SCAN_RULES; i++) {
        if (_Scan_Rules[i].sr_letter == letter && (_Scan_Rules[i].sr_lengths & (1 << length))) {
            return &_Scan_Rules[i];
        }
    }
    return _SCAN_NULL;
}

// Match a directive with where it stores, and return its length, 0 for none.
unsigned long _Scan_MatchDirective(const char *format, _Scan_VaList *list, _Scan_Directive *dir)
{
    _Scan_Assign assign;
    unsigned long set = 0;
    const _Scan_Rule *rule;
    unsigned long at = _Scan_MatchHead(format, &assign, dir);

    // Phase: find its conversion, before any argument is taken
    if (at == 0) {
        return 0;
    }
    rule = _Scan_FindRule(format[at], dir->sd_length);
    if (rule == _SCAN_NULL) {
        return 0;
    }
    dir->sd_kind = rule->sr_kind;
    dir->sd_chars = rule->sr_chars;
    dir->sd_space = rule->sr_space;
    dir->sd_sign = rule->sr_sign;
    dir->sd_base = rule->sr_base;
    dir->sd_set = _SCAN_NULL;
    dir->sd_count = 0;
    dir->sd_accept = _SCAN_ACCEPT_LISTED;
    dir->sd_dest = _SCAN_NULL;

    // Phase: refuse what its conversion may not have, and match a scanset
    if (dir->sd_kind == _SCAN_KIND_PERCENT && at != 1) {
        return 0;
    }
    if (dir->sd_kind == _SCAN_KIND_COUNT && (assign != _SCAN_ASSIGN_STORE || dir->sd_width != _SCAN_WIDTH_NONE)) {
        return 0;
    }
    if (dir->sd_kind == _SCAN_KIND_SET) {
        set = _Scan_MatchSet(format + at + 1, dir);
        if (set == 0) {
            return 0;
        }
    }

    // Phase: take where it stores, unless it stores nothing
    if (dir->sd_kind != _SCAN_KIND_PERCENT && assign == _SCAN_ASSIGN_STORE && dir->sd_chars == _SCAN_CHARS_WIDE) {
        dir->sd_dest = __builtin_va_arg(*list, _Scan_WcharType *);
    } else if (dir->sd_kind != _SCAN_KIND_PERCENT && assign == _SCAN_ASSIGN_STORE) {
        dir->sd_dest = __builtin_va_arg(*list, void *);
    }
    return at + 1 + set;
}

// Take the longest input that is or begins an integer of a base into item.
unsigned long _Scan_TakeDigits(_Scan_Source *source, int width, int base, char *item)
{
    int ch;
    int digit;
    unsigned long first;
    unsigned long len = 0;
    unsigned long read = 0;
    unsigned long digits = 0;
    unsigned long max = width == _SCAN_WIDTH_NONE ? _SCAN_SIZE_MAX : (unsigned long) width;

    // Phase: read the sign
    ch = _Scan_PeekSource(source);
    if (ch == '+' || ch == '-') {
        item[len++] = (char) ch;
        _Scan_AdvanceSource(source);
        read++;
    }

    // Phase: read the prefix, which settles the base if the directive did not
    first = len;
    if (read < max && _Scan_PeekSource(source) == '0') {
        item[len++] = '0';
        _Scan_AdvanceSource(source);
        read++;
        digits++;
        ch = _Scan_PeekSource(source);
        if (read < max && (ch == 'x' || ch == 'X') && (base == _SCAN_BASE_PREFIX || base == _SCAN_BASE_HEX)) {
            item[len++] = (char) ch;
            _Scan_AdvanceSource(source);
            read++;
            first = len;
            digits = 0;
            base = _SCAN_BASE_HEX;
        } else if (base == _SCAN_BASE_PREFIX) {
            base = _SCAN_BASE_OCTAL;
        }
    } else if (base == _SCAN_BASE_PREFIX) {
        base = _SCAN_BASE_DECIMAL;
    }

    // Phase: read the digits, keeping one leading zero and what item holds
    for (; read < max; read++) {
        ch = _Scan_PeekSource(source);
        digit = _Scan_IsDigit(ch) ? ch - '0' : _Scan_IsHexDigit(ch) ? _Scan_ToLower(ch) - 'a' + _SCAN_BASE_DECIMAL : base;
        if (digit >= base) {
            break;
        }
        if (len < _SCAN_ITEM - 1 && (ch != '0' || len != first + 1 || item[first] != '0')) {
            item[len++] = (char) ch;
        }
        _Scan_AdvanceSource(source);
        digits++;
    }
    item[len] = '\0';
    if (digits == 0) {
        source->ss_failure = read == 0 && ch == _FILE_END ? _SCAN_FAILURE_INPUT : _SCAN_FAILURE_MATCHING;
    }
    return digits;
}

// Store value at dest as an integer of a length's type, signed or not.
void _Scan_StoreInteger(void *dest, _Scan_Length length, unsigned long long value)
{
    switch (length) {
        case _SCAN_LENGTH_CHAR: {
            *(unsigned char *) dest = (unsigned char) value;
        } break;
        case _SCAN_LENGTH_SHORT: {
            *(unsigned short *) dest = (unsigned short) value;
        } break;
        case _SCAN_LENGTH_LLONG: {
            *(unsigned long long *) dest = value;
        } break;
        case _SCAN_LENGTH_INTMAX: {
            *(_Scan_UintmaxType *) dest = (_Scan_UintmaxType) value;
        } break;
        case _SCAN_LENGTH_LONG:
        case _SCAN_LENGTH_SIZE:
        case _SCAN_LENGTH_PTRDIFF: {
            *(unsigned long *) dest = (unsigned long) value;
        } break;
        default: {
            *(unsigned int *) dest = (unsigned int) value;
        } break;
    }
}

// Take the source's next characters into item while set has them, max at most.
unsigned long _Scan_TakeChars(_Scan_Source *source, _Scan_Item *item, const char *set, unsigned long max)
{
    int ch;
    char *text;
    unsigned long size;
    unsigned long count = 0;

    while (count < max && item->si_room > 0) {
        ch = _Scan_PeekSource(source);
        if (ch == _FILE_END || ch == '\0' || _Str_FindChar(set, _Scan_ToLower(ch)) == _SCAN_NULL) {
            break;
        }

        // Phase: grow the item to hold one character more and its null
        if (item->si_len + 2 > item->si_size) {
            size = item->si_size == 0 ? _SCAN_ITEM : 2 * item->si_size;
            text = _Mem_ResizeMemory(item->si_text, size);
            if (text == _SCAN_NULL) {
                source->ss_failure = _SCAN_FAILURE_INPUT;
                item->si_room = 0;
                break;
            }
            item->si_text = text;
            item->si_size = size;
        }
        item->si_text[item->si_len++] = (char) ch;
        item->si_text[item->si_len] = '\0';
        _Scan_AdvanceSource(source);
        item->si_room--;
        count++;
    }
    return count;
}

// Take the source's next characters into item while they spell word, any case.
unsigned long _Scan_TakeWord(_Scan_Source *source, _Scan_Item *item, const char *word)
{
    unsigned long count = 0;
    char set[2] = {'\0', '\0'};

    for (; word[count] != '\0'; count++) {
        set[0] = word[count];
        if (_Scan_TakeChars(source, item, set, 1) == 0) {
            break;
        }
    }
    return count;
}

// Take the longest input that is or begins a floating number into a new item.
void _Scan_TakeReal(_Scan_Source *source, int width, _Scan_Item *item)
{
    int ch;
    unsigned long digits;
    const char *mark = _SCAN_EXPONENT;
    const char *set = _SCAN_DECIMALS;

    item->si_text = _SCAN_NULL;
    item->si_len = 0;
    item->si_size = 0;
    item->si_room = width == _SCAN_WIDTH_NONE ? _SCAN_SIZE_MAX : (unsigned long) width;

    // Phase: read the sign
    _Scan_TakeChars(source, item, _SCAN_SIGNS, 1);
    ch = item->si_room > 0 ? _Scan_ToLower(_Scan_PeekSource(source)) : _FILE_END;

    // Phase: read an infinity, or a NaN and what its parentheses hold
    if (ch == _SCAN_INFINITY[0]) {
        _Scan_TakeWord(source, item, _SCAN_INFINITY);
        return;
    }
    if (ch == _SCAN_NAN[0]) {
        if (_Scan_TakeWord(source, item, _SCAN_NAN) == sizeof(_SCAN_NAN) - 1 && _Scan_TakeChars(source, item, "(", 1) > 0) {
            _Scan_TakeChars(source, item, _SCAN_NCHARS, _SCAN_SIZE_MAX);
            _Scan_TakeChars(source, item, ")", 1);
        }
        return;
    }

    // Phase: read 0x, which makes the digits hex and the exponent a power of 2
    digits = _Scan_TakeChars(source, item, "0", 1);
    if (digits > 0 && _Scan_TakeChars(source, item, "x", 1) > 0) {
        digits = 0;
        set = _SCAN_HEXES;
        mark = _SCAN_POWER;
    }

    // Phase: read the digits and the point among them
    digits += _Scan_TakeChars(source, item, set, _SCAN_SIZE_MAX);
    _Scan_TakeChars(source, item, _SCAN_POINT, 1);
    digits += _Scan_TakeChars(source, item, set, _SCAN_SIZE_MAX);

    // Phase: read the exponent, which only a number with a digit has
    if (digits > 0 && _Scan_TakeChars(source, item, mark, 1) > 0) {
        _Scan_TakeChars(source, item, _SCAN_SIGNS, 1);
        _Scan_TakeChars(source, item, _SCAN_DECIMALS, _SCAN_SIZE_MAX);
    }
}

// Begin a directive's characters at where it stores, if anywhere.
void _Scan_BeginTarget(_Scan_Target *target, const _Scan_Directive *dir)
{
    target->st_chars = dir->sd_chars;
    target->st_str = dir->sd_chars == _SCAN_CHARS_NARROW ? dir->sd_dest : _SCAN_NULL;
    target->st_wstr = dir->sd_chars == _SCAN_CHARS_WIDE ? dir->sd_dest : _SCAN_NULL;
    target->st_count = 0;
}

// Put the byte ch, storing a wide character once its last byte is put.
void _Scan_PutByte(_Scan_Source *source, _Scan_Target *target, int ch)
{
    _Scan_WcharType wc;

    if (target->st_chars == _SCAN_CHARS_NARROW && target->st_str != _SCAN_NULL) {
        *target->st_str++ = (char) ch;
    }
    if (target->st_chars == _SCAN_CHARS_NARROW) {
        return;
    }
    target->st_byte[target->st_count++] = (char) ch;
    if (_Utf8_DecodeChar(target->st_byte, target->st_count, &wc) >= 0) {
        if (target->st_wstr != _SCAN_NULL) {
            *target->st_wstr++ = wc;
        }
        target->st_count = 0;
    } else if (target->st_count >= _Utf8_GetCharMax()) {
        _Err_Errno = _SYS_EILSEQ;
        source->ss_failure = _SCAN_FAILURE_INPUT;
    }
}

// End a directive's characters, a null after all but %c's, and count them.
void _Scan_EndTarget(_Scan_Source *source, _Scan_Target *target, _Scan_Kind kind)
{
    // Phase: fail a wide character that lacks bytes
    if (source->ss_failure == _SCAN_FAILURE_NONE && target->st_count > 0) {
        source->ss_failure = _SCAN_FAILURE_INPUT;
        _Err_Errno = _SYS_EILSEQ;
    }
    if (source->ss_failure != _SCAN_FAILURE_NONE) {
        return;
    }

    // Phase: end the string, and count the assignment
    if (target->st_str != _SCAN_NULL && kind != _SCAN_KIND_CHARS) {
        *target->st_str = '\0';
    }
    if (target->st_wstr != _SCAN_NULL && kind != _SCAN_KIND_CHARS) {
        *target->st_wstr = 0;
    }
    if (target->st_str != _SCAN_NULL || target->st_wstr != _SCAN_NULL) {
        source->ss_stored++;
    }
}

// Skip the white space the source is at, none being enough.
void _Scan_SkipSpace(_Scan_Source *source)
{
    while (_Scan_IsSpace(_Scan_PeekSource(source))) {
        _Scan_AdvanceSource(source);
    }
}

// Read len characters that should be text's, stopping at one that is not.
void _Scan_ReadText(_Scan_Source *source, const char *text, unsigned long len)
{
    int ch;

    for (unsigned long i = 0; i < len; i++) {
        ch = _Scan_PeekSource(source);
        if (ch != (unsigned char) text[i]) {
            _Scan_FailSource(source, ch);
            return;
        }
        _Scan_AdvanceSource(source);
    }
}

// Read the characters %c, %s or %[ takes, narrow or wide, width its bound.
void _Scan_ReadChars(_Scan_Source *source, const _Scan_Directive *dir)
{
    int ch = 0;
    _Scan_Target target;
    unsigned long len = 0;
    unsigned long least = 1;
    unsigned long max = dir->sd_kind == _SCAN_KIND_CHARS ? 1 : _SCAN_SIZE_MAX;

    // Phase: settle how many bytes it may take, and how many it must
    if (dir->sd_width != _SCAN_WIDTH_NONE) {
        max = (unsigned long) dir->sd_width;
    }
    if (dir->sd_kind == _SCAN_KIND_CHARS) {
        least = max;
    }

    // Phase: read them to where it stores
    _Scan_BeginTarget(&target, dir);
    while (len < max && source->ss_failure == _SCAN_FAILURE_NONE && (ch = _Scan_PeekSource(source)) != _FILE_END && _Scan_IsTaken(dir, ch)) {
        _Scan_PutByte(source, &target, ch);
        _Scan_AdvanceSource(source);
        len++;
    }
    if (len < least && source->ss_failure == _SCAN_FAILURE_NONE) {
        _Scan_FailSource(source, ch);
    }
    if (len >= least) {
        _Scan_EndTarget(source, &target, dir->sd_kind);
    }
}

// Read an integer in the directive's base, as its length's type.
void _Scan_ReadInteger(_Scan_Source *source, const _Scan_Directive *dir)
{
    char item[_SCAN_ITEM];
    unsigned long long value;

    if (_Scan_TakeDigits(source, dir->sd_width, dir->sd_base, item) == 0 || dir->sd_dest == _SCAN_NULL) {
        return;
    }
    if (dir->sd_sign == _SCAN_SIGN_SIGNED) {
        value = (unsigned long long) _Scan_ToSigned(item, dir->sd_base);
    } else {
        value = _Scan_ToUnsigned(item, dir->sd_base);
    }
    _Scan_StoreInteger(dir->sd_dest, dir->sd_length, value);
    source->ss_stored++;
}

// Read a pointer as %p writes one, (nil) or hex.
void _Scan_ReadPointer(_Scan_Source *source, const _Scan_Directive *dir)
{
    char item[_SCAN_ITEM];
    const char *nil = _SCAN_NIL;
    unsigned long long value = 0;

    if (_Scan_PeekSource(source) == nil[0] && (dir->sd_width == _SCAN_WIDTH_NONE || dir->sd_width >= _SCAN_NIL_WIDTH)) {
        // Phase: read a null pointer, in letters of either case
        for (unsigned long i = 0; nil[i] != '\0'; i++) {
            if (_Scan_ToLower(_Scan_PeekSource(source)) != nil[i]) {
                source->ss_failure = _SCAN_FAILURE_MATCHING;
                return;
            }
            _Scan_AdvanceSource(source);
        }
    } else {
        // Phase: read an address
        if (_Scan_TakeDigits(source, dir->sd_width, dir->sd_base, item) == 0) {
            return;
        }
        value = _Scan_ToUnsigned(item, dir->sd_base);
    }
    if (dir->sd_dest != _SCAN_NULL) {
        *(void **) dir->sd_dest = (void *) (unsigned long) value;
        source->ss_stored++;
    }
}

// Read a floating number, as its length's type.
void _Scan_ReadReal(_Scan_Source *source, const _Scan_Directive *dir)
{
    _Scan_Item item;

    _Scan_TakeReal(source, dir->sd_width, &item);
    if (source->ss_failure == _SCAN_FAILURE_NONE && item.si_len == 0) {
        _Scan_FailSource(source, _Scan_PeekSource(source));
    } else if (source->ss_failure == _SCAN_FAILURE_NONE && ! _Scan_ToReal(item.si_text, dir->sd_length, dir->sd_dest)) {
        source->ss_failure = _SCAN_FAILURE_MATCHING;
    } else if (source->ss_failure == _SCAN_FAILURE_NONE && dir->sd_dest != _SCAN_NULL) {
        source->ss_stored++;
    }
    _Mem_FreeMemory(item.si_text);
}

// Read what a directive converts, by its kind, after any white space it skips.
void _Scan_ReadDirective(_Scan_Source *source, const _Scan_Directive *dir)
{
    if (dir->sd_space == _SCAN_SPACE_SKIP) {
        _Scan_SkipSpace(source);
    }
    switch (dir->sd_kind) {
        case _SCAN_KIND_PERCENT: {
            _Scan_ReadText(source, "%", 1);
        } break;
        case _SCAN_KIND_CHARS:
        case _SCAN_KIND_STRING:
        case _SCAN_KIND_SET: {
            _Scan_ReadChars(source, dir);
        } break;
        case _SCAN_KIND_COUNT: {
            _Scan_StoreInteger(dir->sd_dest, dir->sd_length, source->ss_count);
        } break;
        case _SCAN_KIND_INTEGER: {
            _Scan_ReadInteger(source, dir);
        } break;
        case _SCAN_KIND_POINTER: {
            _Scan_ReadPointer(source, dir);
        } break;
        case _SCAN_KIND_REAL: {
            _Scan_ReadReal(source, dir);
        } break;
    }
}

// Read the source as the format says, each directive storing at its argument.
int _Scan_ReadFormat(_Scan_Source *source, const char *format, _Scan_VaList args)
{
    unsigned long len;
    _Scan_VaList list;
    _Scan_Directive dir;

    __builtin_va_copy(list, args);
    for (unsigned long i = 0; format[i] != '\0' && source->ss_failure == _SCAN_FAILURE_NONE; i += len) {
        if ((len = _Scan_MatchSpace(format + i)) > 0) {
            _Scan_SkipSpace(source);
        } else if ((len = _Scan_MatchText(format + i)) > 0) {
            _Scan_ReadText(source, format + i, len);
        } else if ((len = _Scan_MatchDirective(format + i, &list, &dir)) > 0) {
            _Scan_ReadDirective(source, &dir);
        } else {
            source->ss_failure = _SCAN_FAILURE_MATCHING;
        }
    }
    __builtin_va_end(list);
    return _Scan_EndSource(source);
}
