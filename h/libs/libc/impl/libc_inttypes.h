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

#ifndef _LIBC_IMPL_INTTYPES_H
#define _LIBC_IMPL_INTTYPES_H

// The integer types the functions take and return.
#include <libc/impl/libc_stdint.h>

// The sizes, wide characters and null pointer the conversions take.
#include <libc/impl/libc_stddef.h>

// Macros for format specifiers
#define _LIBC_IMPL_INTTYPES_PRId8       "d"
#define _LIBC_IMPL_INTTYPES_PRId16      "d"
#define _LIBC_IMPL_INTTYPES_PRId32      "d"
#define _LIBC_IMPL_INTTYPES_PRId64      "ld"
#define _LIBC_IMPL_INTTYPES_PRIdLEAST8  "d"
#define _LIBC_IMPL_INTTYPES_PRIdLEAST16 "d"
#define _LIBC_IMPL_INTTYPES_PRIdLEAST32 "d"
#define _LIBC_IMPL_INTTYPES_PRIdLEAST64 "ld"
#define _LIBC_IMPL_INTTYPES_PRIdFAST8   "d"
#define _LIBC_IMPL_INTTYPES_PRIdFAST16  "ld"
#define _LIBC_IMPL_INTTYPES_PRIdFAST32  "ld"
#define _LIBC_IMPL_INTTYPES_PRIdFAST64  "ld"
#define _LIBC_IMPL_INTTYPES_PRIdMAX     "ld"
#define _LIBC_IMPL_INTTYPES_PRIdPTR     "ld"
#define _LIBC_IMPL_INTTYPES_PRIi8       "i"
#define _LIBC_IMPL_INTTYPES_PRIi16      "i"
#define _LIBC_IMPL_INTTYPES_PRIi32      "i"
#define _LIBC_IMPL_INTTYPES_PRIi64      "li"
#define _LIBC_IMPL_INTTYPES_PRIiLEAST8  "i"
#define _LIBC_IMPL_INTTYPES_PRIiLEAST16 "i"
#define _LIBC_IMPL_INTTYPES_PRIiLEAST32 "i"
#define _LIBC_IMPL_INTTYPES_PRIiLEAST64 "li"
#define _LIBC_IMPL_INTTYPES_PRIiFAST8   "i"
#define _LIBC_IMPL_INTTYPES_PRIiFAST16  "li"
#define _LIBC_IMPL_INTTYPES_PRIiFAST32  "li"
#define _LIBC_IMPL_INTTYPES_PRIiFAST64  "li"
#define _LIBC_IMPL_INTTYPES_PRIiMAX     "li"
#define _LIBC_IMPL_INTTYPES_PRIiPTR     "li"
#define _LIBC_IMPL_INTTYPES_PRIo8       "o"
#define _LIBC_IMPL_INTTYPES_PRIo16      "o"
#define _LIBC_IMPL_INTTYPES_PRIo32      "o"
#define _LIBC_IMPL_INTTYPES_PRIo64      "lo"
#define _LIBC_IMPL_INTTYPES_PRIoLEAST8  "o"
#define _LIBC_IMPL_INTTYPES_PRIoLEAST16 "o"
#define _LIBC_IMPL_INTTYPES_PRIoLEAST32 "o"
#define _LIBC_IMPL_INTTYPES_PRIoLEAST64 "lo"
#define _LIBC_IMPL_INTTYPES_PRIoFAST8   "o"
#define _LIBC_IMPL_INTTYPES_PRIoFAST16  "lo"
#define _LIBC_IMPL_INTTYPES_PRIoFAST32  "lo"
#define _LIBC_IMPL_INTTYPES_PRIoFAST64  "lo"
#define _LIBC_IMPL_INTTYPES_PRIoMAX     "lo"
#define _LIBC_IMPL_INTTYPES_PRIoPTR     "lo"
#define _LIBC_IMPL_INTTYPES_PRIu8       "u"
#define _LIBC_IMPL_INTTYPES_PRIu16      "u"
#define _LIBC_IMPL_INTTYPES_PRIu32      "u"
#define _LIBC_IMPL_INTTYPES_PRIu64      "lu"
#define _LIBC_IMPL_INTTYPES_PRIuLEAST8  "u"
#define _LIBC_IMPL_INTTYPES_PRIuLEAST16 "u"
#define _LIBC_IMPL_INTTYPES_PRIuLEAST32 "u"
#define _LIBC_IMPL_INTTYPES_PRIuLEAST64 "lu"
#define _LIBC_IMPL_INTTYPES_PRIuFAST8   "u"
#define _LIBC_IMPL_INTTYPES_PRIuFAST16  "lu"
#define _LIBC_IMPL_INTTYPES_PRIuFAST32  "lu"
#define _LIBC_IMPL_INTTYPES_PRIuFAST64  "lu"
#define _LIBC_IMPL_INTTYPES_PRIuMAX     "lu"
#define _LIBC_IMPL_INTTYPES_PRIuPTR     "lu"
#define _LIBC_IMPL_INTTYPES_PRIx8       "x"
#define _LIBC_IMPL_INTTYPES_PRIx16      "x"
#define _LIBC_IMPL_INTTYPES_PRIx32      "x"
#define _LIBC_IMPL_INTTYPES_PRIx64      "lx"
#define _LIBC_IMPL_INTTYPES_PRIxLEAST8  "x"
#define _LIBC_IMPL_INTTYPES_PRIxLEAST16 "x"
#define _LIBC_IMPL_INTTYPES_PRIxLEAST32 "x"
#define _LIBC_IMPL_INTTYPES_PRIxLEAST64 "lx"
#define _LIBC_IMPL_INTTYPES_PRIxFAST8   "x"
#define _LIBC_IMPL_INTTYPES_PRIxFAST16  "lx"
#define _LIBC_IMPL_INTTYPES_PRIxFAST32  "lx"
#define _LIBC_IMPL_INTTYPES_PRIxFAST64  "lx"
#define _LIBC_IMPL_INTTYPES_PRIxMAX     "lx"
#define _LIBC_IMPL_INTTYPES_PRIxPTR     "lx"
#define _LIBC_IMPL_INTTYPES_PRIX8       "X"
#define _LIBC_IMPL_INTTYPES_PRIX16      "X"
#define _LIBC_IMPL_INTTYPES_PRIX32      "X"
#define _LIBC_IMPL_INTTYPES_PRIX64      "lX"
#define _LIBC_IMPL_INTTYPES_PRIXLEAST8  "X"
#define _LIBC_IMPL_INTTYPES_PRIXLEAST16 "X"
#define _LIBC_IMPL_INTTYPES_PRIXLEAST32 "X"
#define _LIBC_IMPL_INTTYPES_PRIXLEAST64 "lX"
#define _LIBC_IMPL_INTTYPES_PRIXFAST8   "X"
#define _LIBC_IMPL_INTTYPES_PRIXFAST16  "lX"
#define _LIBC_IMPL_INTTYPES_PRIXFAST32  "lX"
#define _LIBC_IMPL_INTTYPES_PRIXFAST64  "lX"
#define _LIBC_IMPL_INTTYPES_PRIXMAX     "lX"
#define _LIBC_IMPL_INTTYPES_PRIXPTR     "lX"
#define _LIBC_IMPL_INTTYPES_SCNd8       "hhd"
#define _LIBC_IMPL_INTTYPES_SCNd16      "hd"
#define _LIBC_IMPL_INTTYPES_SCNd32      "d"
#define _LIBC_IMPL_INTTYPES_SCNd64      "ld"
#define _LIBC_IMPL_INTTYPES_SCNdLEAST8  "hhd"
#define _LIBC_IMPL_INTTYPES_SCNdLEAST16 "hd"
#define _LIBC_IMPL_INTTYPES_SCNdLEAST32 "d"
#define _LIBC_IMPL_INTTYPES_SCNdLEAST64 "ld"
#define _LIBC_IMPL_INTTYPES_SCNdFAST8   "hhd"
#define _LIBC_IMPL_INTTYPES_SCNdFAST16  "ld"
#define _LIBC_IMPL_INTTYPES_SCNdFAST32  "ld"
#define _LIBC_IMPL_INTTYPES_SCNdFAST64  "ld"
#define _LIBC_IMPL_INTTYPES_SCNdMAX     "ld"
#define _LIBC_IMPL_INTTYPES_SCNdPTR     "ld"
#define _LIBC_IMPL_INTTYPES_SCNi8       "hhi"
#define _LIBC_IMPL_INTTYPES_SCNi16      "hi"
#define _LIBC_IMPL_INTTYPES_SCNi32      "i"
#define _LIBC_IMPL_INTTYPES_SCNi64      "li"
#define _LIBC_IMPL_INTTYPES_SCNiLEAST8  "hhi"
#define _LIBC_IMPL_INTTYPES_SCNiLEAST16 "hi"
#define _LIBC_IMPL_INTTYPES_SCNiLEAST32 "i"
#define _LIBC_IMPL_INTTYPES_SCNiLEAST64 "li"
#define _LIBC_IMPL_INTTYPES_SCNiFAST8   "hhi"
#define _LIBC_IMPL_INTTYPES_SCNiFAST16  "li"
#define _LIBC_IMPL_INTTYPES_SCNiFAST32  "li"
#define _LIBC_IMPL_INTTYPES_SCNiFAST64  "li"
#define _LIBC_IMPL_INTTYPES_SCNiMAX     "li"
#define _LIBC_IMPL_INTTYPES_SCNiPTR     "li"
#define _LIBC_IMPL_INTTYPES_SCNo8       "hho"
#define _LIBC_IMPL_INTTYPES_SCNo16      "ho"
#define _LIBC_IMPL_INTTYPES_SCNo32      "o"
#define _LIBC_IMPL_INTTYPES_SCNo64      "lo"
#define _LIBC_IMPL_INTTYPES_SCNoLEAST8  "hho"
#define _LIBC_IMPL_INTTYPES_SCNoLEAST16 "ho"
#define _LIBC_IMPL_INTTYPES_SCNoLEAST32 "o"
#define _LIBC_IMPL_INTTYPES_SCNoLEAST64 "lo"
#define _LIBC_IMPL_INTTYPES_SCNoFAST8   "hho"
#define _LIBC_IMPL_INTTYPES_SCNoFAST16  "lo"
#define _LIBC_IMPL_INTTYPES_SCNoFAST32  "lo"
#define _LIBC_IMPL_INTTYPES_SCNoFAST64  "lo"
#define _LIBC_IMPL_INTTYPES_SCNoMAX     "lo"
#define _LIBC_IMPL_INTTYPES_SCNoPTR     "lo"
#define _LIBC_IMPL_INTTYPES_SCNu8       "hhu"
#define _LIBC_IMPL_INTTYPES_SCNu16      "hu"
#define _LIBC_IMPL_INTTYPES_SCNu32      "u"
#define _LIBC_IMPL_INTTYPES_SCNu64      "lu"
#define _LIBC_IMPL_INTTYPES_SCNuLEAST8  "hhu"
#define _LIBC_IMPL_INTTYPES_SCNuLEAST16 "hu"
#define _LIBC_IMPL_INTTYPES_SCNuLEAST32 "u"
#define _LIBC_IMPL_INTTYPES_SCNuLEAST64 "lu"
#define _LIBC_IMPL_INTTYPES_SCNuFAST8   "hhu"
#define _LIBC_IMPL_INTTYPES_SCNuFAST16  "lu"
#define _LIBC_IMPL_INTTYPES_SCNuFAST32  "lu"
#define _LIBC_IMPL_INTTYPES_SCNuFAST64  "lu"
#define _LIBC_IMPL_INTTYPES_SCNuMAX     "lu"
#define _LIBC_IMPL_INTTYPES_SCNuPTR     "lu"
#define _LIBC_IMPL_INTTYPES_SCNx8       "hhx"
#define _LIBC_IMPL_INTTYPES_SCNx16      "hx"
#define _LIBC_IMPL_INTTYPES_SCNx32      "x"
#define _LIBC_IMPL_INTTYPES_SCNx64      "lx"
#define _LIBC_IMPL_INTTYPES_SCNxLEAST8  "hhx"
#define _LIBC_IMPL_INTTYPES_SCNxLEAST16 "hx"
#define _LIBC_IMPL_INTTYPES_SCNxLEAST32 "x"
#define _LIBC_IMPL_INTTYPES_SCNxLEAST64 "lx"
#define _LIBC_IMPL_INTTYPES_SCNxFAST8   "hhx"
#define _LIBC_IMPL_INTTYPES_SCNxFAST16  "lx"
#define _LIBC_IMPL_INTTYPES_SCNxFAST32  "lx"
#define _LIBC_IMPL_INTTYPES_SCNxFAST64  "lx"
#define _LIBC_IMPL_INTTYPES_SCNxMAX     "lx"
#define _LIBC_IMPL_INTTYPES_SCNxPTR     "lx"

// The base a leading 0 selects.
#define _LIBC_IMPL_INTTYPES_BASE_OCTAL 8

// The base a number without a prefix is read in.
#define _LIBC_IMPL_INTTYPES_BASE_DECIMAL 10

// The base a leading 0x selects.
#define _LIBC_IMPL_INTTYPES_BASE_HEX 16

// The greatest base, whose digits run to z, and the value of a non-digit.
#define _LIBC_IMPL_INTTYPES_BASE_MAX 36

// The value of the digit a.
#define _LIBC_IMPL_INTTYPES_DIGIT_LETTER 10

// Format conversion of integer types
typedef struct {
    _Libc_Impl_Stdint_intmax_t quot;
    _Libc_Impl_Stdint_intmax_t rem;
} _Libc_Impl_Inttypes_imaxdiv_t;

// Functions for greatest-width integer types
_Libc_Impl_Stdint_intmax_t _Libc_Impl_Inttypes_imaxabs(_Libc_Impl_Stdint_intmax_t j);
_Libc_Impl_Inttypes_imaxdiv_t _Libc_Impl_Inttypes_imaxdiv(_Libc_Impl_Stdint_intmax_t numer, _Libc_Impl_Stdint_intmax_t denom);
_Libc_Impl_Stdint_intmax_t _Libc_Impl_Inttypes_strtoimax(const char *restrict nptr, char **restrict endptr, int base);
_Libc_Impl_Stdint_uintmax_t _Libc_Impl_Inttypes_strtoumax(const char *restrict nptr, char **restrict endptr, int base);
_Libc_Impl_Stdint_intmax_t _Libc_Impl_Inttypes_wcstoimax(const _Libc_Impl_Stddef_wchar_t *restrict nptr, _Libc_Impl_Stddef_wchar_t **restrict endptr, int base);
_Libc_Impl_Stdint_uintmax_t _Libc_Impl_Inttypes_wcstoumax(const _Libc_Impl_Stddef_wchar_t *restrict nptr, _Libc_Impl_Stddef_wchar_t **restrict endptr, int base);

// Conversion
int _Libc_Impl_Inttypes_CharAt(const void *str, _Bool wide, _Libc_Impl_Stddef_size_t index);
_Bool _Libc_Impl_Inttypes_IsSpace(int ch);
int _Libc_Impl_Inttypes_Digit(int ch);
_Libc_Impl_Stdint_uintmax_t _Libc_Impl_Inttypes_Convert(const void *str, _Bool wide, _Libc_Impl_Stddef_size_t *end, int base, _Bool *negative, _Bool *overflow);
_Libc_Impl_Stdint_intmax_t _Libc_Impl_Inttypes_ToSigned(_Libc_Impl_Stdint_uintmax_t value, _Bool negative, _Bool overflow);
_Libc_Impl_Stdint_uintmax_t _Libc_Impl_Inttypes_ToUnsigned(_Libc_Impl_Stdint_uintmax_t value, _Bool negative, _Bool overflow);

#endif // _LIBC_IMPL_INTTYPES_H
