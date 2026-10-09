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
#include <errno.h>

// The white space the conversions skip.
#include <libc/impl/libc_ctype.h>

// The range of a narrow character.
#include <limits.h>

// Give the absolute value of j.
intmax_t __libc_impl_inttypes_imaxabs(intmax_t j)
{
    return j < 0 ? -j : j;
}

// Divide numer by denom, giving the quotient and the remainder.
imaxdiv_t __libc_impl_inttypes_imaxdiv(intmax_t numer, intmax_t denom)
{
    imaxdiv_t result;

    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

// Convert the start of the string nptr to an intmax_t in base.
intmax_t __libc_impl_inttypes_strtoimax(const char *restrict nptr, char **restrict endptr, int base)
{
    size_t end;
    bool negative;
    bool overflow;
    uintmax_t value = __libc_impl_inttypes_convert(nptr, false, &end, base, &negative, &overflow);

    if (endptr != NULL) {
        *endptr = (char *) nptr + end;
    }
    return __libc_impl_inttypes_to_signed(value, negative, overflow);
}

// Convert the start of the string nptr to a uintmax_t in base.
uintmax_t __libc_impl_inttypes_strtoumax(const char *restrict nptr, char **restrict endptr, int base)
{
    size_t end;
    bool negative;
    bool overflow;
    uintmax_t value = __libc_impl_inttypes_convert(nptr, false, &end, base, &negative, &overflow);

    if (endptr != NULL) {
        *endptr = (char *) nptr + end;
    }
    return __libc_impl_inttypes_to_unsigned(value, negative, overflow);
}

// Convert the start of the wide string nptr to an intmax_t in base.
intmax_t __libc_impl_inttypes_wcstoimax(const __libc_wchar_t *restrict nptr, __libc_wchar_t **restrict endptr, int base)
{
    size_t end;
    bool negative;
    bool overflow;
    uintmax_t value = __libc_impl_inttypes_convert(nptr, true, &end, base, &negative, &overflow);

    if (endptr != NULL) {
        *endptr = (__libc_wchar_t *) nptr + end;
    }
    return __libc_impl_inttypes_to_signed(value, negative, overflow);
}

// Convert the start of the wide string nptr to a uintmax_t in base.
uintmax_t __libc_impl_inttypes_wcstoumax(const __libc_wchar_t *restrict nptr, __libc_wchar_t **restrict endptr, int base)
{
    size_t end;
    bool negative;
    bool overflow;
    uintmax_t value = __libc_impl_inttypes_convert(nptr, true, &end, base, &negative, &overflow);

    if (endptr != NULL) {
        *endptr = (__libc_wchar_t *) nptr + end;
    }
    return __libc_impl_inttypes_to_unsigned(value, negative, overflow);
}

// Read character index of str, a wide string when wide.
int __libc_impl_inttypes_char_at(const void *str, bool wide, size_t index)
{
    if (wide) {
        return ((const __libc_wchar_t *) str)[index];
    }
    return ((const unsigned char *) str)[index];
}

// Check whether ch is white space in the "C" locale.
bool __libc_impl_inttypes_is_space(int ch)
{
    return ch >= 0 && ch <= UCHAR_MAX && __libc_impl_ctype_isspace(ch);
}

// Give the value of the digit ch, or __LIBC_IMPL_INTTYPES_BASE_MAX for none.
int __libc_impl_inttypes_digit(int ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'z') {
        return ch - 'a' + __LIBC_IMPL_INTTYPES_DIGIT_LETTER;
    }
    if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A' + __LIBC_IMPL_INTTYPES_DIGIT_LETTER;
    }
    return __LIBC_IMPL_INTTYPES_BASE_MAX;
}

// Convert the number at the start of str to its magnitude, ending at *end.
uintmax_t __libc_impl_inttypes_convert(const void *str, bool wide, size_t *end, int base, bool *negative, bool *overflow)
{
    size_t i = 0;
    size_t start;
    uintmax_t value = 0;
    int ch;
    int digit;

    *end = 0;
    *negative = false;
    *overflow = false;
    if (base < 0 || base == 1 || base > __LIBC_IMPL_INTTYPES_BASE_MAX) {
        return 0;
    }
    while (__libc_impl_inttypes_is_space(__libc_impl_inttypes_char_at(str, wide, i))) {
        i++;
    }
    ch = __libc_impl_inttypes_char_at(str, wide, i);
    if (ch == '+' || ch == '-') {
        *negative = ch == '-';
        i++;
    }
    if ((base == 0 || base == __LIBC_IMPL_INTTYPES_BASE_HEX)
        && __libc_impl_inttypes_char_at(str, wide, i) == '0'
        && (__libc_impl_inttypes_char_at(str, wide, i + 1) == 'x' || __libc_impl_inttypes_char_at(str, wide, i + 1) == 'X')
        && __libc_impl_inttypes_digit(__libc_impl_inttypes_char_at(str, wide, i + 2)) < __LIBC_IMPL_INTTYPES_BASE_HEX) {
        base = __LIBC_IMPL_INTTYPES_BASE_HEX;
        i += 2;
    } else if (base == 0) {
        base = __libc_impl_inttypes_char_at(str, wide, i) == '0' ? __LIBC_IMPL_INTTYPES_BASE_OCTAL : __LIBC_IMPL_INTTYPES_BASE_DECIMAL;
    }
    start = i;
    while ((digit = __libc_impl_inttypes_digit(__libc_impl_inttypes_char_at(str, wide, i))) < base) {
        if (value > (UINTMAX_MAX - (uintmax_t) digit) / (uintmax_t) base) {
            *overflow = true;
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
intmax_t __libc_impl_inttypes_to_signed(uintmax_t value, bool negative, bool overflow)
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
uintmax_t __libc_impl_inttypes_to_unsigned(uintmax_t value, bool negative, bool overflow)
{
    if (overflow) {
        errno = ERANGE;
        return UINTMAX_MAX;
    }
    return negative ? 0U - value : value;
}
