/*
 * C source file for format conversion of integer types.
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
#include <inttypes.h>

// The error number of a value out of range.
#include <errno.h>

// The white space the conversions skip.
#include <ctype.h>

// The range of a narrow character.
#include <limits.h>

// Types the checks compare against.
#include <stddef.h>

// Check the header's own types against stddef.h's.
typedef char _Inttypes_CheckSize[sizeof(_Inttypes_SizeType) == sizeof(size_t) && (_Inttypes_SizeType) -1 == (size_t) -1 ? 1 : -1];
typedef char _Inttypes_CheckWchar[sizeof(_Inttypes_WcharType) == sizeof(wchar_t) && ((_Inttypes_WcharType) -1 < 0) == ((wchar_t) -1 < 0) ? 1 : -1];

// Read character index of str, a wide string when wide.
int _Inttypes_CharAt(const void *str, _Bool wide, _Inttypes_SizeType index)
{
    if (wide) {
        return ((const _Inttypes_WcharType *) str)[index];
    }
    return ((const unsigned char *) str)[index];
}

// Check whether ch is white space in the "C" locale.
_Bool _Inttypes_IsSpace(int ch)
{
    return ch >= 0 && ch <= UCHAR_MAX && isspace(ch);
}

// Give the value of the digit ch, or _INTTYPES_BASE_MAX for none.
int _Inttypes_Digit(int ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'z') {
        return ch - 'a' + _INTTYPES_DIGIT_LETTER;
    }
    if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A' + _INTTYPES_DIGIT_LETTER;
    }
    return _INTTYPES_BASE_MAX;
}

// Convert the number at the start of str to its magnitude, ending at *end.
uintmax_t _Inttypes_Convert(const void *str, _Bool wide, _Inttypes_SizeType *end, int base, _Bool *negative, _Bool *overflow)
{
    _Inttypes_SizeType i = 0;
    _Inttypes_SizeType start;
    uintmax_t value = 0;
    int ch;
    int digit;

    *end = 0;
    *negative = 0;
    *overflow = 0;
    if (base < 0 || base == 1 || base > _INTTYPES_BASE_MAX) {
        return 0;
    }
    while (_Inttypes_IsSpace(_Inttypes_CharAt(str, wide, i))) {
        i++;
    }
    ch = _Inttypes_CharAt(str, wide, i);
    if (ch == '+' || ch == '-') {
        *negative = ch == '-';
        i++;
    }
    if ((base == 0 || base == _INTTYPES_BASE_HEX)
        && _Inttypes_CharAt(str, wide, i) == '0'
        && (_Inttypes_CharAt(str, wide, i + 1) == 'x' || _Inttypes_CharAt(str, wide, i + 1) == 'X')
        && _Inttypes_Digit(_Inttypes_CharAt(str, wide, i + 2)) < _INTTYPES_BASE_HEX) {
        base = _INTTYPES_BASE_HEX;
        i += 2;
    } else if (base == 0) {
        base = _Inttypes_CharAt(str, wide, i) == '0' ? _INTTYPES_BASE_OCTAL : _INTTYPES_BASE_DECIMAL;
    }
    start = i;
    while ((digit = _Inttypes_Digit(_Inttypes_CharAt(str, wide, i))) < base) {
        if (value > (UINTMAX_MAX - (uintmax_t) digit) / (uintmax_t) base) {
            *overflow = 1;
        } else {
            value = value * (uintmax_t) base + (uintmax_t) digit;
        }
        i++;
    }
    if (i > start) {
        *end = i;
    }
    return value;
}

// Give the intmax_t of a magnitude, clamped with ERANGE if out of range.
intmax_t _Inttypes_ToSigned(uintmax_t value, _Bool negative, _Bool overflow)
{
    uintmax_t limit = negative ? (uintmax_t) INTMAX_MAX + 1 : (uintmax_t) INTMAX_MAX;

    if (overflow || value > limit) {
        errno = ERANGE;
        return negative ? INTMAX_MIN : INTMAX_MAX;
    }
    if (negative && value == limit) {
        return INTMAX_MIN;
    }
    return negative ? -(intmax_t) value : (intmax_t) value;
}

// Give the uintmax_t of a magnitude, UINTMAX_MAX with ERANGE if too big.
uintmax_t _Inttypes_ToUnsigned(uintmax_t value, _Bool negative, _Bool overflow)
{
    if (overflow) {
        errno = ERANGE;
        return UINTMAX_MAX;
    }
    return negative ? 0U - value : value;
}

// Give the absolute value of j.
intmax_t imaxabs(intmax_t j)
{
    return j < 0 ? -j : j;
}

// Divide numer by denom, giving the quotient and the remainder.
imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom)
{
    imaxdiv_t result;

    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

// Convert the start of the string nptr to an intmax_t in base.
intmax_t strtoimax(const char *restrict nptr, char **restrict endptr, int base)
{
    _Inttypes_SizeType end;
    _Bool negative;
    _Bool overflow;
    uintmax_t value = _Inttypes_Convert(nptr, 0, &end, base, &negative, &overflow);

    if (endptr != NULL) {
        *endptr = (char *) nptr + end;
    }
    return _Inttypes_ToSigned(value, negative, overflow);
}

// Convert the start of the string nptr to a uintmax_t in base.
uintmax_t strtoumax(const char *restrict nptr, char **restrict endptr, int base)
{
    _Inttypes_SizeType end;
    _Bool negative;
    _Bool overflow;
    uintmax_t value = _Inttypes_Convert(nptr, 0, &end, base, &negative, &overflow);

    if (endptr != NULL) {
        *endptr = (char *) nptr + end;
    }
    return _Inttypes_ToUnsigned(value, negative, overflow);
}

// Convert the start of the wide string nptr to an intmax_t in base.
intmax_t wcstoimax(const _Inttypes_WcharType *restrict nptr, _Inttypes_WcharType **restrict endptr, int base)
{
    _Inttypes_SizeType end;
    _Bool negative;
    _Bool overflow;
    uintmax_t value = _Inttypes_Convert(nptr, 1, &end, base, &negative, &overflow);

    if (endptr != NULL) {
        *endptr = (_Inttypes_WcharType *) nptr + end;
    }
    return _Inttypes_ToSigned(value, negative, overflow);
}

// Convert the start of the wide string nptr to a uintmax_t in base.
uintmax_t wcstoumax(const _Inttypes_WcharType *restrict nptr, _Inttypes_WcharType **restrict endptr, int base)
{
    _Inttypes_SizeType end;
    _Bool negative;
    _Bool overflow;
    uintmax_t value = _Inttypes_Convert(nptr, 1, &end, base, &negative, &overflow);

    if (endptr != NULL) {
        *endptr = (_Inttypes_WcharType *) nptr + end;
    }
    return _Inttypes_ToUnsigned(value, negative, overflow);
}
