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
#include <libc/impl/libc_inttypes.h>

// The error number of a value out of range.
#include <libc/impl/libc_errno.h>

// The white space the conversions skip.
#include <libc/impl/libc_ctype.h>

// The range of a narrow character.
#include <libc/impl/libc_limits.h>

// Give the absolute value of j.
_Libc_Impl_Stdint_intmax_t _Libc_Impl_Inttypes_imaxabs(_Libc_Impl_Stdint_intmax_t j)
{
    return j < 0 ? -j : j;
}

// Divide numer by denom, giving the quotient and the remainder.
_Libc_Impl_Inttypes_imaxdiv_t _Libc_Impl_Inttypes_imaxdiv(_Libc_Impl_Stdint_intmax_t numer, _Libc_Impl_Stdint_intmax_t denom)
{
    _Libc_Impl_Inttypes_imaxdiv_t result;

    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

// Convert the start of the string nptr to an intmax_t in base.
_Libc_Impl_Stdint_intmax_t _Libc_Impl_Inttypes_strtoimax(const char *restrict nptr, char **restrict endptr, int base)
{
    _Libc_Impl_Stddef_size_t end;
    _Bool negative;
    _Bool overflow;
    _Libc_Impl_Stdint_uintmax_t value = _Libc_Impl_Inttypes_Convert(nptr, 0, &end, base, &negative, &overflow);

    if (endptr != _LIBC_IMPL_STDDEF_NULL) {
        *endptr = (char *) nptr + end;
    }
    return _Libc_Impl_Inttypes_ToSigned(value, negative, overflow);
}

// Convert the start of the string nptr to a uintmax_t in base.
_Libc_Impl_Stdint_uintmax_t _Libc_Impl_Inttypes_strtoumax(const char *restrict nptr, char **restrict endptr, int base)
{
    _Libc_Impl_Stddef_size_t end;
    _Bool negative;
    _Bool overflow;
    _Libc_Impl_Stdint_uintmax_t value = _Libc_Impl_Inttypes_Convert(nptr, 0, &end, base, &negative, &overflow);

    if (endptr != _LIBC_IMPL_STDDEF_NULL) {
        *endptr = (char *) nptr + end;
    }
    return _Libc_Impl_Inttypes_ToUnsigned(value, negative, overflow);
}

// Convert the start of the wide string nptr to an intmax_t in base.
_Libc_Impl_Stdint_intmax_t _Libc_Impl_Inttypes_wcstoimax(const _Libc_Impl_Stddef_wchar_t *restrict nptr, _Libc_Impl_Stddef_wchar_t **restrict endptr, int base)
{
    _Libc_Impl_Stddef_size_t end;
    _Bool negative;
    _Bool overflow;
    _Libc_Impl_Stdint_uintmax_t value = _Libc_Impl_Inttypes_Convert(nptr, 1, &end, base, &negative, &overflow);

    if (endptr != _LIBC_IMPL_STDDEF_NULL) {
        *endptr = (_Libc_Impl_Stddef_wchar_t *) nptr + end;
    }
    return _Libc_Impl_Inttypes_ToSigned(value, negative, overflow);
}

// Convert the start of the wide string nptr to a uintmax_t in base.
_Libc_Impl_Stdint_uintmax_t _Libc_Impl_Inttypes_wcstoumax(const _Libc_Impl_Stddef_wchar_t *restrict nptr, _Libc_Impl_Stddef_wchar_t **restrict endptr, int base)
{
    _Libc_Impl_Stddef_size_t end;
    _Bool negative;
    _Bool overflow;
    _Libc_Impl_Stdint_uintmax_t value = _Libc_Impl_Inttypes_Convert(nptr, 1, &end, base, &negative, &overflow);

    if (endptr != _LIBC_IMPL_STDDEF_NULL) {
        *endptr = (_Libc_Impl_Stddef_wchar_t *) nptr + end;
    }
    return _Libc_Impl_Inttypes_ToUnsigned(value, negative, overflow);
}

// Read character index of str, a wide string when wide.
int _Libc_Impl_Inttypes_CharAt(const void *str, _Bool wide, _Libc_Impl_Stddef_size_t index)
{
    if (wide) {
        return ((const _Libc_Impl_Stddef_wchar_t *) str)[index];
    }
    return ((const unsigned char *) str)[index];
}

// Check whether ch is white space in the "C" locale.
_Bool _Libc_Impl_Inttypes_IsSpace(int ch)
{
    return ch >= 0 && ch <= _LIBC_IMPL_LIMITS_UCHAR_MAX && _Libc_Impl_Ctype_isspace(ch);
}

// Give the value of the digit ch, or _LIBC_IMPL_INTTYPES_BASE_MAX for none.
int _Libc_Impl_Inttypes_Digit(int ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'z') {
        return ch - 'a' + _LIBC_IMPL_INTTYPES_DIGIT_LETTER;
    }
    if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A' + _LIBC_IMPL_INTTYPES_DIGIT_LETTER;
    }
    return _LIBC_IMPL_INTTYPES_BASE_MAX;
}

// Convert the number at the start of str to its magnitude, ending at *end.
_Libc_Impl_Stdint_uintmax_t _Libc_Impl_Inttypes_Convert(const void *str, _Bool wide, _Libc_Impl_Stddef_size_t *end, int base, _Bool *negative, _Bool *overflow)
{
    _Libc_Impl_Stddef_size_t i = 0;
    _Libc_Impl_Stddef_size_t start;
    _Libc_Impl_Stdint_uintmax_t value = 0;
    int ch;
    int digit;

    *end = 0;
    *negative = 0;
    *overflow = 0;
    if (base < 0 || base == 1 || base > _LIBC_IMPL_INTTYPES_BASE_MAX) {
        return 0;
    }
    while (_Libc_Impl_Inttypes_IsSpace(_Libc_Impl_Inttypes_CharAt(str, wide, i))) {
        i++;
    }
    ch = _Libc_Impl_Inttypes_CharAt(str, wide, i);
    if (ch == '+' || ch == '-') {
        *negative = ch == '-';
        i++;
    }
    if ((base == 0 || base == _LIBC_IMPL_INTTYPES_BASE_HEX)
        && _Libc_Impl_Inttypes_CharAt(str, wide, i) == '0'
        && (_Libc_Impl_Inttypes_CharAt(str, wide, i + 1) == 'x' || _Libc_Impl_Inttypes_CharAt(str, wide, i + 1) == 'X')
        && _Libc_Impl_Inttypes_Digit(_Libc_Impl_Inttypes_CharAt(str, wide, i + 2)) < _LIBC_IMPL_INTTYPES_BASE_HEX) {
        base = _LIBC_IMPL_INTTYPES_BASE_HEX;
        i += 2;
    } else if (base == 0) {
        base = _Libc_Impl_Inttypes_CharAt(str, wide, i) == '0' ? _LIBC_IMPL_INTTYPES_BASE_OCTAL : _LIBC_IMPL_INTTYPES_BASE_DECIMAL;
    }
    start = i;
    while ((digit = _Libc_Impl_Inttypes_Digit(_Libc_Impl_Inttypes_CharAt(str, wide, i))) < base) {
        if (value > (_LIBC_IMPL_STDINT_UINTMAX_MAX - (_Libc_Impl_Stdint_uintmax_t) digit) / (_Libc_Impl_Stdint_uintmax_t) base) {
            *overflow = 1;
        } else {
            value = value * (_Libc_Impl_Stdint_uintmax_t) base + (_Libc_Impl_Stdint_uintmax_t) digit;
        }
        i++;
    }
    if (i > start) {
        *end = i;
    }
    return value;
}

// Give the intmax_t of a magnitude, clamped with ERANGE if out of range.
_Libc_Impl_Stdint_intmax_t _Libc_Impl_Inttypes_ToSigned(_Libc_Impl_Stdint_uintmax_t value, _Bool negative, _Bool overflow)
{
    _Libc_Impl_Stdint_uintmax_t limit = negative ? (_Libc_Impl_Stdint_uintmax_t) _LIBC_IMPL_STDINT_INTMAX_MAX + 1 : (_Libc_Impl_Stdint_uintmax_t) _LIBC_IMPL_STDINT_INTMAX_MAX;

    if (overflow || value > limit) {
        errno = _LIBC_IMPL_ERRNO_ERANGE;
        return negative ? _LIBC_IMPL_STDINT_INTMAX_MIN : _LIBC_IMPL_STDINT_INTMAX_MAX;
    }
    if (negative && value == limit) {
        return _LIBC_IMPL_STDINT_INTMAX_MIN;
    }
    return negative ? -(_Libc_Impl_Stdint_intmax_t) value : (_Libc_Impl_Stdint_intmax_t) value;
}

// Give the uintmax_t of a magnitude, UINTMAX_MAX with ERANGE if too big.
_Libc_Impl_Stdint_uintmax_t _Libc_Impl_Inttypes_ToUnsigned(_Libc_Impl_Stdint_uintmax_t value, _Bool negative, _Bool overflow)
{
    if (overflow) {
        errno = _LIBC_IMPL_ERRNO_ERANGE;
        return _LIBC_IMPL_STDINT_UINTMAX_MAX;
    }
    return negative ? 0U - value : value;
}
