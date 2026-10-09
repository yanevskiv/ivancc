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
__libc_impl_stdint_intmax_t __libc_impl_inttypes_imaxabs(__libc_impl_stdint_intmax_t j)
{
    return j < 0 ? -j : j;
}

// Divide numer by denom, giving the quotient and the remainder.
__libc_impl_inttypes_imaxdiv_t __libc_impl_inttypes_imaxdiv(__libc_impl_stdint_intmax_t numer, __libc_impl_stdint_intmax_t denom)
{
    __libc_impl_inttypes_imaxdiv_t result;

    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

// Convert the start of the string nptr to an intmax_t in base.
__libc_impl_stdint_intmax_t __libc_impl_inttypes_strtoimax(const char *restrict nptr, char **restrict endptr, int base)
{
    __libc_impl_stddef_size_t end;
    _Bool negative;
    _Bool overflow;
    __libc_impl_stdint_uintmax_t value = __libc_impl_inttypes_convert(nptr, 0, &end, base, &negative, &overflow);

    if (endptr != __LIBC_IMPL_STDDEF_NULL) {
        *endptr = (char *) nptr + end;
    }
    return __libc_impl_inttypes_to_signed(value, negative, overflow);
}

// Convert the start of the string nptr to a uintmax_t in base.
__libc_impl_stdint_uintmax_t __libc_impl_inttypes_strtoumax(const char *restrict nptr, char **restrict endptr, int base)
{
    __libc_impl_stddef_size_t end;
    _Bool negative;
    _Bool overflow;
    __libc_impl_stdint_uintmax_t value = __libc_impl_inttypes_convert(nptr, 0, &end, base, &negative, &overflow);

    if (endptr != __LIBC_IMPL_STDDEF_NULL) {
        *endptr = (char *) nptr + end;
    }
    return __libc_impl_inttypes_to_unsigned(value, negative, overflow);
}

// Convert the start of the wide string nptr to an intmax_t in base.
__libc_impl_stdint_intmax_t __libc_impl_inttypes_wcstoimax(const __libc_impl_stddef_wchar_t *restrict nptr, __libc_impl_stddef_wchar_t **restrict endptr, int base)
{
    __libc_impl_stddef_size_t end;
    _Bool negative;
    _Bool overflow;
    __libc_impl_stdint_uintmax_t value = __libc_impl_inttypes_convert(nptr, 1, &end, base, &negative, &overflow);

    if (endptr != __LIBC_IMPL_STDDEF_NULL) {
        *endptr = (__libc_impl_stddef_wchar_t *) nptr + end;
    }
    return __libc_impl_inttypes_to_signed(value, negative, overflow);
}

// Convert the start of the wide string nptr to a uintmax_t in base.
__libc_impl_stdint_uintmax_t __libc_impl_inttypes_wcstoumax(const __libc_impl_stddef_wchar_t *restrict nptr, __libc_impl_stddef_wchar_t **restrict endptr, int base)
{
    __libc_impl_stddef_size_t end;
    _Bool negative;
    _Bool overflow;
    __libc_impl_stdint_uintmax_t value = __libc_impl_inttypes_convert(nptr, 1, &end, base, &negative, &overflow);

    if (endptr != __LIBC_IMPL_STDDEF_NULL) {
        *endptr = (__libc_impl_stddef_wchar_t *) nptr + end;
    }
    return __libc_impl_inttypes_to_unsigned(value, negative, overflow);
}

// Read character index of str, a wide string when wide.
int __libc_impl_inttypes_char_at(const void *str, _Bool wide, __libc_impl_stddef_size_t index)
{
    if (wide) {
        return ((const __libc_impl_stddef_wchar_t *) str)[index];
    }
    return ((const unsigned char *) str)[index];
}

// Check whether ch is white space in the "C" locale.
_Bool __libc_impl_inttypes_is_space(int ch)
{
    return ch >= 0 && ch <= __LIBC_IMPL_LIMITS_UCHAR_MAX && __libc_impl_ctype_isspace(ch);
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
__libc_impl_stdint_uintmax_t __libc_impl_inttypes_convert(const void *str, _Bool wide, __libc_impl_stddef_size_t *end, int base, _Bool *negative, _Bool *overflow)
{
    __libc_impl_stddef_size_t i = 0;
    __libc_impl_stddef_size_t start;
    __libc_impl_stdint_uintmax_t value = 0;
    int ch;
    int digit;

    *end = 0;
    *negative = 0;
    *overflow = 0;
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
        if (value > (__LIBC_IMPL_STDINT_UINTMAX_MAX - (__libc_impl_stdint_uintmax_t) digit) / (__libc_impl_stdint_uintmax_t) base) {
            *overflow = 1;
        } else {
            value = value * (__libc_impl_stdint_uintmax_t) base + (__libc_impl_stdint_uintmax_t) digit;
        }
        i++;
    }
    if (i > start) {
        *end = i;
    }
    return value;
}

// Give the intmax_t of a magnitude, clamped with ERANGE if out of range.
__libc_impl_stdint_intmax_t __libc_impl_inttypes_to_signed(__libc_impl_stdint_uintmax_t value, _Bool negative, _Bool overflow)
{
    __libc_impl_stdint_uintmax_t limit = negative ? (__libc_impl_stdint_uintmax_t) __LIBC_IMPL_STDINT_INTMAX_MAX + 1 : (__libc_impl_stdint_uintmax_t) __LIBC_IMPL_STDINT_INTMAX_MAX;

    if (overflow || value > limit) {
        errno = __LIBC_IMPL_ERRNO_ERANGE;
        return negative ? __LIBC_IMPL_STDINT_INTMAX_MIN : __LIBC_IMPL_STDINT_INTMAX_MAX;
    }
    if (negative && value == limit) {
        return __LIBC_IMPL_STDINT_INTMAX_MIN;
    }
    return negative ? -(__libc_impl_stdint_intmax_t) value : (__libc_impl_stdint_intmax_t) value;
}

// Give the uintmax_t of a magnitude, UINTMAX_MAX with ERANGE if too big.
__libc_impl_stdint_uintmax_t __libc_impl_inttypes_to_unsigned(__libc_impl_stdint_uintmax_t value, _Bool negative, _Bool overflow)
{
    if (overflow) {
        errno = __LIBC_IMPL_ERRNO_ERANGE;
        return __LIBC_IMPL_STDINT_UINTMAX_MAX;
    }
    return negative ? 0U - value : value;
}
