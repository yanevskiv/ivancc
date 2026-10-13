/*
 * C source file for the conversion of text to an integer.
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
#include <_int.h>

// The error numbers of a base and of a value out of range.
#include <_sys.h>

// The error number of the last failed call.
#include <_err.h>

// Read character index of str, a wide string or a narrow one.
int _Int_ReadChar(const void *str, _Int_Text text, unsigned long index)
{
    if (text == _INT_TEXT_WIDE) {
        return ((const int *) str)[index];
    }
    return ((const unsigned char *) str)[index];
}

// Check whether ch is white space in the "C" locale.
_Bool _Int_IsSpace(int ch)
{
    return ch == ' ' || (ch >= '\t' && ch <= '\r');
}

// Convert the digit ch to its value, or _INT_BASE_MAX for none.
int _Int_ToDigit(int ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'z') {
        return ch - 'a' + _INT_DIGIT_LETTER;
    }
    if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A' + _INT_DIGIT_LETTER;
    }
    return _INT_BASE_MAX;
}

// Convert the number at the start of str to its magnitude, ending at *end.
unsigned long _Int_ToNumber(const void *str, _Int_Text text, unsigned long *end, int base, _Int_Sign *sign, _Int_Range *range)
{
    unsigned long i = 0;
    unsigned long start;
    unsigned long value = 0;
    int ch;
    int digit;

    *end = 0;
    *sign = _INT_SIGN_PLUS;
    *range = _INT_RANGE_INSIDE;
    if (base < 0 || base == 1 || base > _INT_BASE_MAX) {
        _Err_Errno = _SYS_EINVAL;
        return 0;
    }
    while (_Int_IsSpace(_Int_ReadChar(str, text, i))) {
        i++;
    }
    ch = _Int_ReadChar(str, text, i);
    if (ch == '+' || ch == '-') {
        *sign = ch == '-' ? _INT_SIGN_MINUS : _INT_SIGN_PLUS;
        i++;
    }
    if ((base == 0 || base == _INT_BASE_HEX)
        && _Int_ReadChar(str, text, i) == '0'
        && (_Int_ReadChar(str, text, i + 1) == 'x' || _Int_ReadChar(str, text, i + 1) == 'X')
        && _Int_ToDigit(_Int_ReadChar(str, text, i + 2)) < _INT_BASE_HEX) {
        base = _INT_BASE_HEX;
        i += 2;
    } else if (base == 0) {
        base = _Int_ReadChar(str, text, i) == '0' ? _INT_BASE_OCTAL : _INT_BASE_DECIMAL;
    }
    start = i;
    while ((digit = _Int_ToDigit(_Int_ReadChar(str, text, i))) < base) {
        if (value > (_INT_VALUE_MAX - (unsigned long) digit) / (unsigned long) base) {
            *range = _INT_RANGE_OUTSIDE;
        } else {
            value = value * (unsigned long) base + (unsigned long) digit;
        }
        i++;
    }
    if (i > start) {
        *end = i;
    }
    return value;
}

// Make the signed value of a magnitude, clamped with ERANGE if out of range.
long _Int_MakeSigned(unsigned long value, _Int_Sign sign, _Int_Range range)
{
    unsigned long limit = sign == _INT_SIGN_MINUS ? (unsigned long) _INT_SIGNED_MAX + 1 : (unsigned long) _INT_SIGNED_MAX;

    if (range == _INT_RANGE_OUTSIDE || value > limit) {
        _Err_Errno = _SYS_ERANGE;
        return sign == _INT_SIGN_MINUS ? _INT_SIGNED_MIN : _INT_SIGNED_MAX;
    }
    if (sign == _INT_SIGN_MINUS && value == limit) {
        return _INT_SIGNED_MIN;
    }
    return sign == _INT_SIGN_MINUS ? -(long) value : (long) value;
}

// Make the unsigned value of a magnitude, the greatest with ERANGE if too big.
unsigned long _Int_MakeUnsigned(unsigned long value, _Int_Sign sign, _Int_Range range)
{
    if (range == _INT_RANGE_OUTSIDE) {
        _Err_Errno = _SYS_ERANGE;
        return _INT_VALUE_MAX;
    }
    return sign == _INT_SIGN_MINUS ? 0U - value : value;
}
