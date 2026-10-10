/*
 * C source file for the UTF-8 encoder and decoder.
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
#include <_utf8.h>

// Whether LC_CTYPE's locale encodes in UTF-8, "C"'s ASCII at startup.
_Bool _Utf8_Enabled;

// Give the length of the sequence lead begins, or 0 for none.
int _Utf8_LeadLength(unsigned int lead)
{
    int len = 1;

    if (lead <= _UTF8_ASCII_MAX) {
        return 1;
    }
    if (lead < _UTF8_LEAD_MIN) {
        return 0;
    }
    while (lead << len & _UTF8_TAIL_MARK) {
        len++;
    }
    return len <= _UTF8_MAX ? len : 0;
}

// Give the length of the shortest sequence that encodes code, or 0 for none.
int _Utf8_CodeLength(unsigned int code)
{
    int len = 2;

    if (code <= _UTF8_ASCII_MAX) {
        return 1;
    }
    if (code > _UTF8_CODE_MAX || (code >= _UTF8_SURROGATE_FIRST && code <= _UTF8_SURROGATE_LAST)) {
        return 0;
    }
    while (code >> ((_UTF8_TAIL_BITS - 1) * len + 1) != 0) {
        len++;
    }
    return len;
}

// Decode the sequence in the n bytes at str into *code, and give its length.
int _Utf8_Decode(const unsigned char *str, unsigned long n, unsigned int *code)
{
    unsigned int value;
    int len;
    int i;

    if (n == 0) {
        return _UTF8_INCOMPLETE;
    }
    len = _Utf8_LeadLength(str[0]);
    if (len == 0) {
        return _UTF8_INVALID;
    }
    value = len == 1 ? str[0] : str[0] & _UTF8_ASCII_MAX >> len;
    for (i = 1; i < len && (unsigned long) i < n; i++) {
        if ((str[i] & _UTF8_TAIL_MASK) != _UTF8_TAIL_MARK) {
            return _UTF8_INVALID;
        }
        value = value << _UTF8_TAIL_BITS | (str[i] & _UTF8_TAIL_VALUE);
    }
    if ((unsigned long) len > n) {
        return _UTF8_INCOMPLETE;
    }
    if (_Utf8_CodeLength(value) != len) {
        return _UTF8_INVALID;
    }
    *code = value;
    return len;
}

// Encode code at str, and give the length of its sequence.
int _Utf8_Encode(unsigned int code, unsigned char *str)
{
    int len = _Utf8_CodeLength(code);
    int i;

    if (len == 0) {
        return _UTF8_INVALID;
    }
    if (len == 1) {
        str[0] = (unsigned char) code;
        return 1;
    }
    for (i = len - 1; i > 0; i--) {
        str[i] = (unsigned char) (_UTF8_TAIL_MARK | (code & _UTF8_TAIL_VALUE));
        code >>= _UTF8_TAIL_BITS;
    }
    str[0] = (unsigned char) (_UTF8_LEAD_ONES >> len | code);
    return len;
}
