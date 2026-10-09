/*
 * C header file for format conversion of integer types.
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

#ifndef __LIBC_INTTYPES_H__
#define __LIBC_INTTYPES_H__

// The integer types the functions take and return.
#include <inttypes.h>

// The flags the conversions report.
#include <stdbool.h>

// The sizes and null pointer the conversions take.
#include <stddef.h>

// The base a leading 0 selects.
#define __LIBC_INTTYPES_BASE_OCTAL 8

// The base a number without a prefix is read in.
#define __LIBC_INTTYPES_BASE_DECIMAL 10

// The base a leading 0x selects.
#define __LIBC_INTTYPES_BASE_HEX 16

// The greatest base, whose digits run to z, and the value of a non-digit.
#define __LIBC_INTTYPES_BASE_MAX 36

// The value of the digit a.
#define __LIBC_INTTYPES_DIGIT_LETTER 10

// (S7.8.2) Functions for greatest-width integer types
intmax_t __libc_inttypes_imaxabs(intmax_t j);
imaxdiv_t __libc_inttypes_imaxdiv(intmax_t numer, intmax_t denom);
intmax_t __libc_inttypes_strtoimax(const char *restrict nptr, char **restrict endptr, int base);
uintmax_t __libc_inttypes_strtoumax(const char *restrict nptr, char **restrict endptr, int base);
intmax_t __libc_inttypes_wcstoimax(const __libc_wchar_t *restrict nptr, __libc_wchar_t **restrict endptr, int base);
uintmax_t __libc_inttypes_wcstoumax(const __libc_wchar_t *restrict nptr, __libc_wchar_t **restrict endptr, int base);

// Conversion
int __libc_inttypes_char_at(const void *str, bool wide, size_t index);
bool __libc_inttypes_is_space(int ch);
int __libc_inttypes_digit(int ch);
uintmax_t __libc_inttypes_convert(const void *str, bool wide, size_t *end, int base, bool *negative, bool *overflow);
intmax_t __libc_inttypes_to_signed(uintmax_t value, bool negative, bool overflow);
uintmax_t __libc_inttypes_to_unsigned(uintmax_t value, bool negative, bool overflow);

#endif // __LIBC_INTTYPES_H__
