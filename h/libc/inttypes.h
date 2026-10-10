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

#ifndef _INTTYPES_H
#define _INTTYPES_H

// Standard headers.
#include <stdint.h>

// The base a leading 0 selects.
#define _INTTYPES_BASE_OCTAL 8

// The base a number without a prefix is read in.
#define _INTTYPES_BASE_DECIMAL 10

// The base a leading 0x selects.
#define _INTTYPES_BASE_HEX 16

// The greatest base, whose digits run to z, and the value of a non-digit.
#define _INTTYPES_BASE_MAX 36

// The value of the digit a.
#define _INTTYPES_DIGIT_LETTER 10

// (S7.8.1) Macros for format specifiers
#define PRId8       "d"
#define PRId16      "d"
#define PRId32      "d"
#define PRId64      "ld"
#define PRIdLEAST8  "d"
#define PRIdLEAST16 "d"
#define PRIdLEAST32 "d"
#define PRIdLEAST64 "ld"
#define PRIdFAST8   "d"
#define PRIdFAST16  "ld"
#define PRIdFAST32  "ld"
#define PRIdFAST64  "ld"
#define PRIdMAX     "ld"
#define PRIdPTR     "ld"
#define PRIi8       "i"
#define PRIi16      "i"
#define PRIi32      "i"
#define PRIi64      "li"
#define PRIiLEAST8  "i"
#define PRIiLEAST16 "i"
#define PRIiLEAST32 "i"
#define PRIiLEAST64 "li"
#define PRIiFAST8   "i"
#define PRIiFAST16  "li"
#define PRIiFAST32  "li"
#define PRIiFAST64  "li"
#define PRIiMAX     "li"
#define PRIiPTR     "li"
#define PRIo8       "o"
#define PRIo16      "o"
#define PRIo32      "o"
#define PRIo64      "lo"
#define PRIoLEAST8  "o"
#define PRIoLEAST16 "o"
#define PRIoLEAST32 "o"
#define PRIoLEAST64 "lo"
#define PRIoFAST8   "o"
#define PRIoFAST16  "lo"
#define PRIoFAST32  "lo"
#define PRIoFAST64  "lo"
#define PRIoMAX     "lo"
#define PRIoPTR     "lo"
#define PRIu8       "u"
#define PRIu16      "u"
#define PRIu32      "u"
#define PRIu64      "lu"
#define PRIuLEAST8  "u"
#define PRIuLEAST16 "u"
#define PRIuLEAST32 "u"
#define PRIuLEAST64 "lu"
#define PRIuFAST8   "u"
#define PRIuFAST16  "lu"
#define PRIuFAST32  "lu"
#define PRIuFAST64  "lu"
#define PRIuMAX     "lu"
#define PRIuPTR     "lu"
#define PRIx8       "x"
#define PRIx16      "x"
#define PRIx32      "x"
#define PRIx64      "lx"
#define PRIxLEAST8  "x"
#define PRIxLEAST16 "x"
#define PRIxLEAST32 "x"
#define PRIxLEAST64 "lx"
#define PRIxFAST8   "x"
#define PRIxFAST16  "lx"
#define PRIxFAST32  "lx"
#define PRIxFAST64  "lx"
#define PRIxMAX     "lx"
#define PRIxPTR     "lx"
#define PRIX8       "X"
#define PRIX16      "X"
#define PRIX32      "X"
#define PRIX64      "lX"
#define PRIXLEAST8  "X"
#define PRIXLEAST16 "X"
#define PRIXLEAST32 "X"
#define PRIXLEAST64 "lX"
#define PRIXFAST8   "X"
#define PRIXFAST16  "lX"
#define PRIXFAST32  "lX"
#define PRIXFAST64  "lX"
#define PRIXMAX     "lX"
#define PRIXPTR     "lX"
#define SCNd8       "hhd"
#define SCNd16      "hd"
#define SCNd32      "d"
#define SCNd64      "ld"
#define SCNdLEAST8  "hhd"
#define SCNdLEAST16 "hd"
#define SCNdLEAST32 "d"
#define SCNdLEAST64 "ld"
#define SCNdFAST8   "hhd"
#define SCNdFAST16  "ld"
#define SCNdFAST32  "ld"
#define SCNdFAST64  "ld"
#define SCNdMAX     "ld"
#define SCNdPTR     "ld"
#define SCNi8       "hhi"
#define SCNi16      "hi"
#define SCNi32      "i"
#define SCNi64      "li"
#define SCNiLEAST8  "hhi"
#define SCNiLEAST16 "hi"
#define SCNiLEAST32 "i"
#define SCNiLEAST64 "li"
#define SCNiFAST8   "hhi"
#define SCNiFAST16  "li"
#define SCNiFAST32  "li"
#define SCNiFAST64  "li"
#define SCNiMAX     "li"
#define SCNiPTR     "li"
#define SCNo8       "hho"
#define SCNo16      "ho"
#define SCNo32      "o"
#define SCNo64      "lo"
#define SCNoLEAST8  "hho"
#define SCNoLEAST16 "ho"
#define SCNoLEAST32 "o"
#define SCNoLEAST64 "lo"
#define SCNoFAST8   "hho"
#define SCNoFAST16  "lo"
#define SCNoFAST32  "lo"
#define SCNoFAST64  "lo"
#define SCNoMAX     "lo"
#define SCNoPTR     "lo"
#define SCNu8       "hhu"
#define SCNu16      "hu"
#define SCNu32      "u"
#define SCNu64      "lu"
#define SCNuLEAST8  "hhu"
#define SCNuLEAST16 "hu"
#define SCNuLEAST32 "u"
#define SCNuLEAST64 "lu"
#define SCNuFAST8   "hhu"
#define SCNuFAST16  "lu"
#define SCNuFAST32  "lu"
#define SCNuFAST64  "lu"
#define SCNuMAX     "lu"
#define SCNuPTR     "lu"
#define SCNx8       "hhx"
#define SCNx16      "hx"
#define SCNx32      "x"
#define SCNx64      "lx"
#define SCNxLEAST8  "hhx"
#define SCNxLEAST16 "hx"
#define SCNxLEAST32 "x"
#define SCNxLEAST64 "lx"
#define SCNxFAST8   "hhx"
#define SCNxFAST16  "lx"
#define SCNxFAST32  "lx"
#define SCNxFAST64  "lx"
#define SCNxMAX     "lx"
#define SCNxPTR     "lx"

// The size and wide character, which inttypes.h may not name.
typedef unsigned long _Inttypes_SizeType;
typedef int _Inttypes_WcharType;

// (S7.8) Format conversion of integer types
typedef struct {
    intmax_t quot;
    intmax_t rem;
} imaxdiv_t;

// Conversion
int _Inttypes_CharAt(const void *str, _Bool wide, _Inttypes_SizeType index);
_Bool _Inttypes_IsSpace(int ch);
int _Inttypes_Digit(int ch);
uintmax_t _Inttypes_Convert(const void *str, _Bool wide, _Inttypes_SizeType *end, int base, _Bool *negative, _Bool *overflow);
intmax_t _Inttypes_ToSigned(uintmax_t value, _Bool negative, _Bool overflow);
uintmax_t _Inttypes_ToUnsigned(uintmax_t value, _Bool negative, _Bool overflow);

// (S7.8.2) Functions for greatest-width integer types
intmax_t imaxabs(intmax_t j);
imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom);
intmax_t strtoimax(const char *restrict nptr, char **restrict endptr, int base);
uintmax_t strtoumax(const char *restrict nptr, char **restrict endptr, int base);
intmax_t wcstoimax(const _Inttypes_WcharType *restrict nptr, _Inttypes_WcharType **restrict endptr, int base);
uintmax_t wcstoumax(const _Inttypes_WcharType *restrict nptr, _Inttypes_WcharType **restrict endptr, int base);

#endif // _INTTYPES_H
