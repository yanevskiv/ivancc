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

// Get the length of the sequence lead begins, or 0 for none.
int _Utf8_GetLeadLength(unsigned int lead)
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

// Get the length of the shortest sequence that encodes code, or 0 for none.
int _Utf8_GetCodeLength(unsigned int code)
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
int _Utf8_DecodeSequence(const unsigned char *str, unsigned long n, unsigned int *code)
{
    unsigned int value;
    int len;
    int i;

    if (n == 0) {
        return _UTF8_INCOMPLETE;
    }
    len = _Utf8_GetLeadLength(str[0]);
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
    if (_Utf8_GetCodeLength(value) != len) {
        return _UTF8_INVALID;
    }
    *code = value;
    return len;
}

// Encode code at str, and give the length of its sequence.
int _Utf8_EncodeCode(unsigned int code, unsigned char *str)
{
    int len = _Utf8_GetCodeLength(code);
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

// Check whether code is in one of count ranges, which are in order.
_Bool _Utf8_IsInRanges(const _Utf8_Table_Range *ranges, unsigned long count, unsigned int code)
{
    unsigned long mid;
    unsigned long low = 0;
    unsigned long high = count;

    while (low < high) {
        mid = low + (high - low) / 2;
        if (ranges[mid].tr_last < code) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return low < count && ranges[low].tr_first <= code;
}

// Check whether code is in class, as Unicode has it.
_Bool _Utf8_IsInClass(unsigned int code, _Utf8_Class class)
{
    _Bool found;

    switch (class) {
        case _UTF8_CLASS_ALNUM: {
            found = _Utf8_IsInClass(code, _UTF8_CLASS_ALPHA) || _Utf8_IsInClass(code, _UTF8_CLASS_DIGIT);
        } break;
        case _UTF8_CLASS_ALPHA: {
            found = _Utf8_IsInRanges(_Utf8_Table_Alpha, _UTF8_TABLE_ALPHA_COUNT, code);
        } break;
        case _UTF8_CLASS_BLANK: {
            found = _Utf8_IsInRanges(_Utf8_Table_Blank, _UTF8_TABLE_BLANK_COUNT, code);
        } break;
        case _UTF8_CLASS_CNTRL: {
            found = _Utf8_IsInRanges(_Utf8_Table_Cntrl, _UTF8_TABLE_CNTRL_COUNT, code);
        } break;
        case _UTF8_CLASS_DIGIT: {
            found = code >= '0' && code <= '9';
        } break;
        case _UTF8_CLASS_GRAPH: {
            found = _Utf8_IsInRanges(_Utf8_Table_Graph, _UTF8_TABLE_GRAPH_COUNT, code);
        } break;
        case _UTF8_CLASS_LOWER: {
            found = _Utf8_IsInRanges(_Utf8_Table_Lower, _UTF8_TABLE_LOWER_COUNT, code);
        } break;
        case _UTF8_CLASS_PRINT: {
            found = _Utf8_IsInClass(code, _UTF8_CLASS_GRAPH);
            if (_Utf8_IsInClass(code, _UTF8_CLASS_BLANK)) {
                found = ! _Utf8_IsInClass(code, _UTF8_CLASS_CNTRL);
            }
        } break;
        case _UTF8_CLASS_PUNCT: {
            found = _Utf8_IsInClass(code, _UTF8_CLASS_GRAPH) && ! _Utf8_IsInClass(code, _UTF8_CLASS_ALNUM);
        } break;
        case _UTF8_CLASS_SPACE: {
            found = _Utf8_IsInRanges(_Utf8_Table_Space, _UTF8_TABLE_SPACE_COUNT, code);
        } break;
        case _UTF8_CLASS_UPPER: {
            found = _Utf8_IsInRanges(_Utf8_Table_Upper, _UTF8_TABLE_UPPER_COUNT, code);
        } break;
        default: {
            found = (code >= '0' && code <= '9') || (code >= 'A' && code <= 'F') || (code >= 'a' && code <= 'f');
        } break;
    }
    return found;
}

// Map code by the one of count shifts that holds it, or give it as it is.
unsigned int _Utf8_ToShifted(const _Utf8_Table_Shift *shifts, unsigned long count, unsigned int code)
{
    unsigned long mid;
    unsigned long low = 0;
    unsigned long high = count;

    while (low < high) {
        mid = low + (high - low) / 2;
        if (shifts[mid].ts_last < code) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    if (low == count || code < shifts[low].ts_first || (code - shifts[low].ts_first) % shifts[low].ts_step != 0) {
        return code;
    }
    return (unsigned int) ((int) code + shifts[low].ts_delta);
}

// Map a lower-case letter to its upper case, and give any other code as it is.
unsigned int _Utf8_ToUpper(unsigned int code)
{
    return _Utf8_ToShifted(_Utf8_Table_ToUpper, _UTF8_TABLE_TO_UPPER_COUNT, code);
}

// Map an upper-case letter to its lower case, and give any other code as it is.
unsigned int _Utf8_ToLower(unsigned int code)
{
    return _Utf8_ToShifted(_Utf8_Table_ToLower, _UTF8_TABLE_TO_LOWER_COUNT, code);
}

// Get the bytes of the longest character in LC_CTYPE's encoding.
unsigned long _Utf8_GetCharMax(void)
{
    return _Utf8_Enabled ? _UTF8_MAX : _UTF8_ASCII_LEN;
}

// Decode the character in the n bytes at str into *wc, and give its length.
int _Utf8_DecodeChar(const char *str, unsigned long n, int *wc)
{
    int len;
    unsigned int code;
    const unsigned char *ptr = (const unsigned char *) str;

    if (_Utf8_Enabled) {
        len = _Utf8_DecodeSequence(ptr, n, &code);
        if (len > 0) {
            *wc = (int) code;
        }
        return len;
    }
    if (n == 0) {
        return _UTF8_INCOMPLETE;
    }
    if (*ptr > _UTF8_ASCII_MAX) {
        return _UTF8_INVALID;
    }
    *wc = *ptr;
    return _UTF8_ASCII_LEN;
}

// Encode wc at str in LC_CTYPE's encoding, and give its length.
int _Utf8_EncodeChar(int wc, char *str)
{
    if (wc < 0) {
        return _UTF8_INVALID;
    }
    if (_Utf8_Enabled) {
        return _Utf8_EncodeCode((unsigned int) wc, (unsigned char *) str);
    }
    if (wc >= _UTF8_TAG_FIRST && wc <= _UTF8_TAG_LAST) {
        return 0;
    }
    if ((unsigned int) wc > _UTF8_ASCII_MAX) {
        return _UTF8_INVALID;
    }
    *str = (char) wc;
    return _UTF8_ASCII_LEN;
}
