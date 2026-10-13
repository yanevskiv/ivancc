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

// The conversion the four functions stand on.
#include <_int.h>

// Types the checks compare against.
#include <stddef.h>

// Check the header's own type against stddef.h's.
typedef char _Inttypes_CheckWchar[sizeof(_Inttypes_WcharType) == sizeof(wchar_t) && ((_Inttypes_WcharType) -1 < 0) == ((wchar_t) -1 < 0) ? 1 : -1];

// Check the types _int converts to and counts in against C99's.
typedef char _Inttypes_CheckSigned[sizeof(long) == sizeof(intmax_t) && _INT_SIGNED_MAX == INTMAX_MAX ? 1 : -1];
typedef char _Inttypes_CheckUnsigned[sizeof(unsigned long) == sizeof(uintmax_t) && _INT_VALUE_MAX == UINTMAX_MAX ? 1 : -1];
typedef char _Inttypes_CheckWide[sizeof(int) == sizeof(_Inttypes_WcharType) ? 1 : -1];
typedef char _Inttypes_CheckSize[sizeof(unsigned long) == sizeof(size_t) ? 1 : -1];

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
    _Int_Sign sign;
    _Int_Range range;
    unsigned long end;
    uintmax_t value = _Int_ConvertNumber(nptr, _INT_TEXT_NARROW, &end, base, &sign, &range);

    if (endptr != NULL) {
        *endptr = (char *) nptr + end;
    }
    return _Int_MakeSigned(value, sign, range);
}

// Convert the start of the string nptr to a uintmax_t in base.
uintmax_t strtoumax(const char *restrict nptr, char **restrict endptr, int base)
{
    _Int_Sign sign;
    _Int_Range range;
    unsigned long end;
    uintmax_t value = _Int_ConvertNumber(nptr, _INT_TEXT_NARROW, &end, base, &sign, &range);

    if (endptr != NULL) {
        *endptr = (char *) nptr + end;
    }
    return _Int_MakeUnsigned(value, sign, range);
}

// Convert the start of the wide string nptr to an intmax_t in base.
intmax_t wcstoimax(const _Inttypes_WcharType *restrict nptr, _Inttypes_WcharType **restrict endptr, int base)
{
    _Int_Sign sign;
    _Int_Range range;
    unsigned long end;
    uintmax_t value = _Int_ConvertNumber(nptr, _INT_TEXT_WIDE, &end, base, &sign, &range);

    if (endptr != NULL) {
        *endptr = (_Inttypes_WcharType *) nptr + end;
    }
    return _Int_MakeSigned(value, sign, range);
}

// Convert the start of the wide string nptr to a uintmax_t in base.
uintmax_t wcstoumax(const _Inttypes_WcharType *restrict nptr, _Inttypes_WcharType **restrict endptr, int base)
{
    _Int_Sign sign;
    _Int_Range range;
    unsigned long end;
    uintmax_t value = _Int_ConvertNumber(nptr, _INT_TEXT_WIDE, &end, base, &sign, &range);

    if (endptr != NULL) {
        *endptr = (_Inttypes_WcharType *) nptr + end;
    }
    return _Int_MakeUnsigned(value, sign, range);
}
