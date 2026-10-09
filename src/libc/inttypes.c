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

// The implementation.
#include <ivancc/impl/libc_inttypes.h>

// Give the absolute value of j.
intmax_t imaxabs(intmax_t j)
{
    return __libc_inttypes_imaxabs(j);
}

// Divide numer by denom, giving the quotient and the remainder.
imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom)
{
    return __libc_inttypes_imaxdiv(numer, denom);
}

// Convert the start of the string nptr to an intmax_t in base.
intmax_t strtoimax(const char *restrict nptr, char **restrict endptr, int base)
{
    return __libc_inttypes_strtoimax(nptr, endptr, base);
}

// Convert the start of the string nptr to a uintmax_t in base.
uintmax_t strtoumax(const char *restrict nptr, char **restrict endptr, int base)
{
    return __libc_inttypes_strtoumax(nptr, endptr, base);
}

// Convert the start of the wide string nptr to an intmax_t in base.
intmax_t wcstoimax(const __libc_wchar_t *restrict nptr, __libc_wchar_t **restrict endptr, int base)
{
    return __libc_inttypes_wcstoimax(nptr, endptr, base);
}

// Convert the start of the wide string nptr to a uintmax_t in base.
uintmax_t wcstoumax(const __libc_wchar_t *restrict nptr, __libc_wchar_t **restrict endptr, int base)
{
    return __libc_inttypes_wcstoumax(nptr, endptr, base);
}
