/*
 * C header file for general utilities.
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

#ifndef __STDLIB_H__
#define __STDLIB_H__

// The significant digits strtod keeps, past a long double halfway point's 11515.
#define _STDLIB_DIGITS 11520

// The decimal exponents past which a number overflows or is zero in every format.
#define _STDLIB_EXP10_MAX 4933
#define _STDLIB_EXP10_MIN (-4951)

// The magnitude an exponent stops growing at, past every format's range.
#define _STDLIB_EXP_LIMIT 100000000000000000L

// The digits a word of strtod's big integer takes at once, and their scale.
#define _STDLIB_CHUNK_DIGITS 9
#define _STDLIB_CHUNK_SCALE  1000000000U

// The bits of a hexadecimal digit, a value past every one, and its top bit.
#define _STDLIB_HEX_BITS  4
#define _STDLIB_HEX_LIMIT 16
#define _STDLIB_HEX_TOP   8

// The bits of the mantissa strtod rounds from.
#define _STDLIB_MANT_BITS 64

// What lies below a mantissa's last bit, against half that bit.
#define _STDLIB_REST_NONE  0
#define _STDLIB_REST_BELOW 1
#define _STDLIB_REST_HALF  2
#define _STDLIB_REST_ABOVE 3

// The sign bits of a float, a double and a long double's exponent word.
#define _STDLIB_FLOAT_SIGN   0x80000000U
#define _STDLIB_DOUBLE_SIGN  0x8000000000000000ULL
#define _STDLIB_LDOUBLE_SIGN 0x8000U

// The alignment of the memory malloc gives, any object's, as glibc's,
// and the smallest block, a header and that much memory.
#define _STDLIB_ALIGN     16
#define _STDLIB_BLOCK_MIN 32

// The least the heap grows by, so that few allocations move the break.
#define _STDLIB_GROW 0x10000

// The functions a block of atexit's holds, C99's least (S7.20.4.2p2).
#define _STDLIB_EXITS 32

// The status abort exits with if SIGABRT did not end the program, as glibc's.
#define _STDLIB_ABORT_STATUS 127

// (S7.20) General utilities
#define NULL ((void *) 0)

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#ifndef __SIZE_T__
#define __SIZE_T__
typedef unsigned long size_t;
#endif

#ifndef __WCHAR_T__
#define __WCHAR_T__
typedef int wchar_t;
#endif

// A floating type: its mantissa's bits, its exponents' range, and an explicit leading bit.
struct _Stdlib_Format {
    int sf_mant;
    int sf_min;
    int sf_max;
    _Bool sf_explicit;
};

// A converted number's sign and its biased exponent and mantissa fields.
struct _Stdlib_Real {
    _Bool sr_negative;
    unsigned int sr_exp;
    unsigned long long sr_mant;
};

// A number before rounding: 64 bits from the top one, its exponent and the rest below.
struct _Stdlib_Unrounded {
    unsigned long long su_mant;
    long su_exp;
    int su_rest;
};

// A decimal's first significant digit, the digits kept, its exponent and a dropped nonzero.
struct _Stdlib_Decimal {
    size_t sd_first;
    long sd_count;
    long sd_exp;
    _Bool sd_sticky;
};

// A block of the heap: its size with this header, and while free the next free block.
struct _Stdlib_Block {
    size_t sb_size;
    struct _Stdlib_Block *sb_next;
};

// A block of the functions atexit registered, and the block before it.
struct _Stdlib_Exits {
    struct _Stdlib_Exits *se_next;
    int se_count;
    void (*se_func[_STDLIB_EXITS])(void);
};

// The formats of float, double and long double.
extern const struct _Stdlib_Format _Stdlib_FloatFormat;
extern const struct _Stdlib_Format _Stdlib_DoubleFormat;
extern const struct _Stdlib_Format _Stdlib_LongDoubleFormat;

// The free blocks, in address order.
extern struct _Stdlib_Block *_Stdlib_FreeList;

// The first block of atexit's, and the newest.
extern struct _Stdlib_Exits _Stdlib_ExitBase;
extern struct _Stdlib_Exits *_Stdlib_ExitTop;

// Conversion
int _Stdlib_CharAt(const void *str, _Bool wide, size_t index);
_Bool _Stdlib_IsSpace(int ch);
int _Stdlib_HexDigit(int ch);
size_t _Stdlib_Match(const void *str, _Bool wide, size_t index, const char *word);
size_t _Stdlib_Exponent(const void *str, _Bool wide, size_t index, int letter, long *exp);
unsigned long long _Stdlib_Payload(const void *str, _Bool wide, size_t index, size_t stop);
size_t _Stdlib_NotANumber(const void *str, _Bool wide, size_t index, unsigned long long *payload);
size_t _Stdlib_ScanDecimal(const void *str, _Bool wide, size_t index, int point, struct _Stdlib_Decimal *dec);
void _Stdlib_Decimal(const void *str, _Bool wide, int point, const struct _Stdlib_Decimal *dec, struct _Stdlib_Unrounded *value);
size_t _Stdlib_Hex(const void *str, _Bool wide, size_t index, int point, struct _Stdlib_Unrounded *value);
void _Stdlib_Shift(struct _Stdlib_Unrounded *value, long bits);
void _Stdlib_Round(const struct _Stdlib_Unrounded *value, const struct _Stdlib_Format *fmt, struct _Stdlib_Real *real);
size_t _Stdlib_ToReal(const void *str, _Bool wide, const struct _Stdlib_Format *fmt, struct _Stdlib_Real *real);

// Heap
size_t _Stdlib_BlockSize(size_t size);
struct _Stdlib_Block **_Stdlib_Fit(size_t size);
struct _Stdlib_Block **_Stdlib_Link(const struct _Stdlib_Block *block);
void *_Stdlib_Take(struct _Stdlib_Block **link, size_t size);
void _Stdlib_Trim(struct _Stdlib_Block *block, size_t size);
void _Stdlib_Release(struct _Stdlib_Block *block);
int _Stdlib_Grow(size_t size);

// Exit
void (*_Stdlib_PopExit(void))(void);

// (S7.20.1) Numeric conversion functions
double atof(const char *nptr);
double strtod(const char *restrict nptr, char **restrict endptr);
float strtof(const char *restrict nptr, char **restrict endptr);
long double strtold(const char *restrict nptr, char **restrict endptr);

// (S7.20.3) Memory management functions
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);

// (S7.20.4) Communication with the environment
void abort(void);
int atexit(void (*func)(void));
void exit(int status);
void _Exit(int status);
char *getenv(const char *name);
int system(const char *string);

#endif // __STDLIB_H__
