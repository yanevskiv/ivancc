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

#ifndef __LIBC_IMPL_INTTYPES_H__
#define __LIBC_IMPL_INTTYPES_H__

// The integer types the functions take and return.
#include <libc/impl/libc_stdint.h>

// The sizes, wide characters and null pointer the conversions take.
#include <libc/impl/libc_stddef.h>

// Macros for format specifiers
#define __LIBC_IMPL_INTTYPES_PRId8       "d"
#define __LIBC_IMPL_INTTYPES_PRId16      "d"
#define __LIBC_IMPL_INTTYPES_PRId32      "d"
#define __LIBC_IMPL_INTTYPES_PRId64      "ld"
#define __LIBC_IMPL_INTTYPES_PRIdLEAST8  "d"
#define __LIBC_IMPL_INTTYPES_PRIdLEAST16 "d"
#define __LIBC_IMPL_INTTYPES_PRIdLEAST32 "d"
#define __LIBC_IMPL_INTTYPES_PRIdLEAST64 "ld"
#define __LIBC_IMPL_INTTYPES_PRIdFAST8   "d"
#define __LIBC_IMPL_INTTYPES_PRIdFAST16  "ld"
#define __LIBC_IMPL_INTTYPES_PRIdFAST32  "ld"
#define __LIBC_IMPL_INTTYPES_PRIdFAST64  "ld"
#define __LIBC_IMPL_INTTYPES_PRIdMAX     "ld"
#define __LIBC_IMPL_INTTYPES_PRIdPTR     "ld"
#define __LIBC_IMPL_INTTYPES_PRIi8       "i"
#define __LIBC_IMPL_INTTYPES_PRIi16      "i"
#define __LIBC_IMPL_INTTYPES_PRIi32      "i"
#define __LIBC_IMPL_INTTYPES_PRIi64      "li"
#define __LIBC_IMPL_INTTYPES_PRIiLEAST8  "i"
#define __LIBC_IMPL_INTTYPES_PRIiLEAST16 "i"
#define __LIBC_IMPL_INTTYPES_PRIiLEAST32 "i"
#define __LIBC_IMPL_INTTYPES_PRIiLEAST64 "li"
#define __LIBC_IMPL_INTTYPES_PRIiFAST8   "i"
#define __LIBC_IMPL_INTTYPES_PRIiFAST16  "li"
#define __LIBC_IMPL_INTTYPES_PRIiFAST32  "li"
#define __LIBC_IMPL_INTTYPES_PRIiFAST64  "li"
#define __LIBC_IMPL_INTTYPES_PRIiMAX     "li"
#define __LIBC_IMPL_INTTYPES_PRIiPTR     "li"
#define __LIBC_IMPL_INTTYPES_PRIo8       "o"
#define __LIBC_IMPL_INTTYPES_PRIo16      "o"
#define __LIBC_IMPL_INTTYPES_PRIo32      "o"
#define __LIBC_IMPL_INTTYPES_PRIo64      "lo"
#define __LIBC_IMPL_INTTYPES_PRIoLEAST8  "o"
#define __LIBC_IMPL_INTTYPES_PRIoLEAST16 "o"
#define __LIBC_IMPL_INTTYPES_PRIoLEAST32 "o"
#define __LIBC_IMPL_INTTYPES_PRIoLEAST64 "lo"
#define __LIBC_IMPL_INTTYPES_PRIoFAST8   "o"
#define __LIBC_IMPL_INTTYPES_PRIoFAST16  "lo"
#define __LIBC_IMPL_INTTYPES_PRIoFAST32  "lo"
#define __LIBC_IMPL_INTTYPES_PRIoFAST64  "lo"
#define __LIBC_IMPL_INTTYPES_PRIoMAX     "lo"
#define __LIBC_IMPL_INTTYPES_PRIoPTR     "lo"
#define __LIBC_IMPL_INTTYPES_PRIu8       "u"
#define __LIBC_IMPL_INTTYPES_PRIu16      "u"
#define __LIBC_IMPL_INTTYPES_PRIu32      "u"
#define __LIBC_IMPL_INTTYPES_PRIu64      "lu"
#define __LIBC_IMPL_INTTYPES_PRIuLEAST8  "u"
#define __LIBC_IMPL_INTTYPES_PRIuLEAST16 "u"
#define __LIBC_IMPL_INTTYPES_PRIuLEAST32 "u"
#define __LIBC_IMPL_INTTYPES_PRIuLEAST64 "lu"
#define __LIBC_IMPL_INTTYPES_PRIuFAST8   "u"
#define __LIBC_IMPL_INTTYPES_PRIuFAST16  "lu"
#define __LIBC_IMPL_INTTYPES_PRIuFAST32  "lu"
#define __LIBC_IMPL_INTTYPES_PRIuFAST64  "lu"
#define __LIBC_IMPL_INTTYPES_PRIuMAX     "lu"
#define __LIBC_IMPL_INTTYPES_PRIuPTR     "lu"
#define __LIBC_IMPL_INTTYPES_PRIx8       "x"
#define __LIBC_IMPL_INTTYPES_PRIx16      "x"
#define __LIBC_IMPL_INTTYPES_PRIx32      "x"
#define __LIBC_IMPL_INTTYPES_PRIx64      "lx"
#define __LIBC_IMPL_INTTYPES_PRIxLEAST8  "x"
#define __LIBC_IMPL_INTTYPES_PRIxLEAST16 "x"
#define __LIBC_IMPL_INTTYPES_PRIxLEAST32 "x"
#define __LIBC_IMPL_INTTYPES_PRIxLEAST64 "lx"
#define __LIBC_IMPL_INTTYPES_PRIxFAST8   "x"
#define __LIBC_IMPL_INTTYPES_PRIxFAST16  "lx"
#define __LIBC_IMPL_INTTYPES_PRIxFAST32  "lx"
#define __LIBC_IMPL_INTTYPES_PRIxFAST64  "lx"
#define __LIBC_IMPL_INTTYPES_PRIxMAX     "lx"
#define __LIBC_IMPL_INTTYPES_PRIxPTR     "lx"
#define __LIBC_IMPL_INTTYPES_PRIX8       "X"
#define __LIBC_IMPL_INTTYPES_PRIX16      "X"
#define __LIBC_IMPL_INTTYPES_PRIX32      "X"
#define __LIBC_IMPL_INTTYPES_PRIX64      "lX"
#define __LIBC_IMPL_INTTYPES_PRIXLEAST8  "X"
#define __LIBC_IMPL_INTTYPES_PRIXLEAST16 "X"
#define __LIBC_IMPL_INTTYPES_PRIXLEAST32 "X"
#define __LIBC_IMPL_INTTYPES_PRIXLEAST64 "lX"
#define __LIBC_IMPL_INTTYPES_PRIXFAST8   "X"
#define __LIBC_IMPL_INTTYPES_PRIXFAST16  "lX"
#define __LIBC_IMPL_INTTYPES_PRIXFAST32  "lX"
#define __LIBC_IMPL_INTTYPES_PRIXFAST64  "lX"
#define __LIBC_IMPL_INTTYPES_PRIXMAX     "lX"
#define __LIBC_IMPL_INTTYPES_PRIXPTR     "lX"
#define __LIBC_IMPL_INTTYPES_SCNd8       "hhd"
#define __LIBC_IMPL_INTTYPES_SCNd16      "hd"
#define __LIBC_IMPL_INTTYPES_SCNd32      "d"
#define __LIBC_IMPL_INTTYPES_SCNd64      "ld"
#define __LIBC_IMPL_INTTYPES_SCNdLEAST8  "hhd"
#define __LIBC_IMPL_INTTYPES_SCNdLEAST16 "hd"
#define __LIBC_IMPL_INTTYPES_SCNdLEAST32 "d"
#define __LIBC_IMPL_INTTYPES_SCNdLEAST64 "ld"
#define __LIBC_IMPL_INTTYPES_SCNdFAST8   "hhd"
#define __LIBC_IMPL_INTTYPES_SCNdFAST16  "ld"
#define __LIBC_IMPL_INTTYPES_SCNdFAST32  "ld"
#define __LIBC_IMPL_INTTYPES_SCNdFAST64  "ld"
#define __LIBC_IMPL_INTTYPES_SCNdMAX     "ld"
#define __LIBC_IMPL_INTTYPES_SCNdPTR     "ld"
#define __LIBC_IMPL_INTTYPES_SCNi8       "hhi"
#define __LIBC_IMPL_INTTYPES_SCNi16      "hi"
#define __LIBC_IMPL_INTTYPES_SCNi32      "i"
#define __LIBC_IMPL_INTTYPES_SCNi64      "li"
#define __LIBC_IMPL_INTTYPES_SCNiLEAST8  "hhi"
#define __LIBC_IMPL_INTTYPES_SCNiLEAST16 "hi"
#define __LIBC_IMPL_INTTYPES_SCNiLEAST32 "i"
#define __LIBC_IMPL_INTTYPES_SCNiLEAST64 "li"
#define __LIBC_IMPL_INTTYPES_SCNiFAST8   "hhi"
#define __LIBC_IMPL_INTTYPES_SCNiFAST16  "li"
#define __LIBC_IMPL_INTTYPES_SCNiFAST32  "li"
#define __LIBC_IMPL_INTTYPES_SCNiFAST64  "li"
#define __LIBC_IMPL_INTTYPES_SCNiMAX     "li"
#define __LIBC_IMPL_INTTYPES_SCNiPTR     "li"
#define __LIBC_IMPL_INTTYPES_SCNo8       "hho"
#define __LIBC_IMPL_INTTYPES_SCNo16      "ho"
#define __LIBC_IMPL_INTTYPES_SCNo32      "o"
#define __LIBC_IMPL_INTTYPES_SCNo64      "lo"
#define __LIBC_IMPL_INTTYPES_SCNoLEAST8  "hho"
#define __LIBC_IMPL_INTTYPES_SCNoLEAST16 "ho"
#define __LIBC_IMPL_INTTYPES_SCNoLEAST32 "o"
#define __LIBC_IMPL_INTTYPES_SCNoLEAST64 "lo"
#define __LIBC_IMPL_INTTYPES_SCNoFAST8   "hho"
#define __LIBC_IMPL_INTTYPES_SCNoFAST16  "lo"
#define __LIBC_IMPL_INTTYPES_SCNoFAST32  "lo"
#define __LIBC_IMPL_INTTYPES_SCNoFAST64  "lo"
#define __LIBC_IMPL_INTTYPES_SCNoMAX     "lo"
#define __LIBC_IMPL_INTTYPES_SCNoPTR     "lo"
#define __LIBC_IMPL_INTTYPES_SCNu8       "hhu"
#define __LIBC_IMPL_INTTYPES_SCNu16      "hu"
#define __LIBC_IMPL_INTTYPES_SCNu32      "u"
#define __LIBC_IMPL_INTTYPES_SCNu64      "lu"
#define __LIBC_IMPL_INTTYPES_SCNuLEAST8  "hhu"
#define __LIBC_IMPL_INTTYPES_SCNuLEAST16 "hu"
#define __LIBC_IMPL_INTTYPES_SCNuLEAST32 "u"
#define __LIBC_IMPL_INTTYPES_SCNuLEAST64 "lu"
#define __LIBC_IMPL_INTTYPES_SCNuFAST8   "hhu"
#define __LIBC_IMPL_INTTYPES_SCNuFAST16  "lu"
#define __LIBC_IMPL_INTTYPES_SCNuFAST32  "lu"
#define __LIBC_IMPL_INTTYPES_SCNuFAST64  "lu"
#define __LIBC_IMPL_INTTYPES_SCNuMAX     "lu"
#define __LIBC_IMPL_INTTYPES_SCNuPTR     "lu"
#define __LIBC_IMPL_INTTYPES_SCNx8       "hhx"
#define __LIBC_IMPL_INTTYPES_SCNx16      "hx"
#define __LIBC_IMPL_INTTYPES_SCNx32      "x"
#define __LIBC_IMPL_INTTYPES_SCNx64      "lx"
#define __LIBC_IMPL_INTTYPES_SCNxLEAST8  "hhx"
#define __LIBC_IMPL_INTTYPES_SCNxLEAST16 "hx"
#define __LIBC_IMPL_INTTYPES_SCNxLEAST32 "x"
#define __LIBC_IMPL_INTTYPES_SCNxLEAST64 "lx"
#define __LIBC_IMPL_INTTYPES_SCNxFAST8   "hhx"
#define __LIBC_IMPL_INTTYPES_SCNxFAST16  "lx"
#define __LIBC_IMPL_INTTYPES_SCNxFAST32  "lx"
#define __LIBC_IMPL_INTTYPES_SCNxFAST64  "lx"
#define __LIBC_IMPL_INTTYPES_SCNxMAX     "lx"
#define __LIBC_IMPL_INTTYPES_SCNxPTR     "lx"

// The base a leading 0 selects.
#define __LIBC_IMPL_INTTYPES_BASE_OCTAL 8

// The base a number without a prefix is read in.
#define __LIBC_IMPL_INTTYPES_BASE_DECIMAL 10

// The base a leading 0x selects.
#define __LIBC_IMPL_INTTYPES_BASE_HEX 16

// The greatest base, whose digits run to z, and the value of a non-digit.
#define __LIBC_IMPL_INTTYPES_BASE_MAX 36

// The value of the digit a.
#define __LIBC_IMPL_INTTYPES_DIGIT_LETTER 10

// Format conversion of integer types
typedef struct {
    __libc_impl_stdint_intmax_t quot;
    __libc_impl_stdint_intmax_t rem;
} __libc_impl_inttypes_imaxdiv_t;

// Functions for greatest-width integer types
__libc_impl_stdint_intmax_t __libc_impl_inttypes_imaxabs(__libc_impl_stdint_intmax_t j);
__libc_impl_inttypes_imaxdiv_t __libc_impl_inttypes_imaxdiv(__libc_impl_stdint_intmax_t numer, __libc_impl_stdint_intmax_t denom);
__libc_impl_stdint_intmax_t __libc_impl_inttypes_strtoimax(const char *restrict nptr, char **restrict endptr, int base);
__libc_impl_stdint_uintmax_t __libc_impl_inttypes_strtoumax(const char *restrict nptr, char **restrict endptr, int base);
__libc_impl_stdint_intmax_t __libc_impl_inttypes_wcstoimax(const __libc_impl_stddef_wchar_t *restrict nptr, __libc_impl_stddef_wchar_t **restrict endptr, int base);
__libc_impl_stdint_uintmax_t __libc_impl_inttypes_wcstoumax(const __libc_impl_stddef_wchar_t *restrict nptr, __libc_impl_stddef_wchar_t **restrict endptr, int base);

// Conversion
int __libc_impl_inttypes_char_at(const void *str, _Bool wide, __libc_impl_stddef_size_t index);
_Bool __libc_impl_inttypes_is_space(int ch);
int __libc_impl_inttypes_digit(int ch);
__libc_impl_stdint_uintmax_t __libc_impl_inttypes_convert(const void *str, _Bool wide, __libc_impl_stddef_size_t *end, int base, _Bool *negative, _Bool *overflow);
__libc_impl_stdint_intmax_t __libc_impl_inttypes_to_signed(__libc_impl_stdint_uintmax_t value, _Bool negative, _Bool overflow);
__libc_impl_stdint_uintmax_t __libc_impl_inttypes_to_unsigned(__libc_impl_stdint_uintmax_t value, _Bool negative, _Bool overflow);

#endif // __LIBC_IMPL_INTTYPES_H__
