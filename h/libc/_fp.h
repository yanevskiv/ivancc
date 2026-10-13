/*
 * C header file for the conversion of text to a floating number.
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


#ifndef _FP_H
#define _FP_H

// The significant digits kept, past a long double halfway point's 11515.
#define _FP_DIGITS 11520

// The decimal exponents past which every format overflows or gives zero.
#define _FP_EXP10_MAX 4933
#define _FP_EXP10_MIN (-4951)

// The magnitude an exponent stops growing at, past every format's range.
#define _FP_EXP_LIMIT 100000000000000000L

// The digits a word of the big integer takes at once, and their scale.
#define _FP_CHUNK_DIGITS 9
#define _FP_CHUNK_SCALE  1000000000U

// The bits of a hexadecimal digit, a value past every one, and its top bit.
#define _FP_HEX_BITS  4
#define _FP_HEX_LIMIT 16
#define _FP_HEX_TOP   8

// The bits of the mantissa a number is rounded from.
#define _FP_MANT_BITS 64

// What lies below a mantissa's last bit, against half that bit.
#define _FP_REST_NONE  0
#define _FP_REST_BELOW 1
#define _FP_REST_HALF  2
#define _FP_REST_ABOVE 3

// The sign bits of a float, a double and a long double's exponent word.
#define _FP_FLOAT_SIGN   0x80000000U
#define _FP_DOUBLE_SIGN  0x8000000000000000ULL
#define _FP_LDOUBLE_SIGN 0x8000U

// The greatest payload a NaN's parentheses give.
#define _FP_PAYLOAD_MAX (~0ULL)

// The character between a number's whole part and its fraction, every locale's.
#define _FP_POINT '.'

// A float's mantissa bits and its least and greatest exponent, one below C's.
#define _FP_FLOAT_MANT 24
#define _FP_FLOAT_MIN  (-126)
#define _FP_FLOAT_MAX  127

// A double's mantissa bits and its least and greatest exponent, one below C's.
#define _FP_DOUBLE_MANT 53
#define _FP_DOUBLE_MIN  (-1022)
#define _FP_DOUBLE_MAX  1023

// A long double's mantissa bits and exponents, one below C's.
#define _FP_LDOUBLE_MANT 64
#define _FP_LDOUBLE_MIN  (-16382)
#define _FP_LDOUBLE_MAX  16383

// The characters a text to convert is made of.
typedef enum _Fp_Text _Fp_Text;
enum _Fp_Text {
    _FP_TEXT_NARROW,
    _FP_TEXT_WIDE
};

// A floating type: its mantissa's bits, its exponents' range, and whether
// its leading bit is explicit.
typedef struct _Fp_Format _Fp_Format;
struct _Fp_Format {
    int ff_mant;
    int ff_min;
    int ff_max;
    _Bool ff_explicit;
};

// A converted number's sign and its biased exponent and mantissa fields.
typedef struct _Fp_Real _Fp_Real;
struct _Fp_Real {
    _Bool fr_negative;
    unsigned int fr_exp;
    unsigned long long fr_mant;
};

// A number before rounding: 64 bits from the top one, its exponent and the
// rest below.
typedef struct _Fp_Unrounded _Fp_Unrounded;
struct _Fp_Unrounded {
    unsigned long long fu_mant;
    long fu_exp;
    int fu_rest;
};

// A decimal's first significant digit, the digits kept, its exponent and
// whether a nonzero digit was dropped.
typedef struct _Fp_Decimal _Fp_Decimal;
struct _Fp_Decimal {
    unsigned long fd_first;
    long fd_count;
    long fd_exp;
    _Bool fd_sticky;
};

// The formats of float, double and long double.
extern const _Fp_Format _Fp_FloatFormat;
extern const _Fp_Format _Fp_DoubleFormat;
extern const _Fp_Format _Fp_LongDoubleFormat;

// Conversion
int                _Fp_ReadChar(const void *str, _Fp_Text text, unsigned long index);
_Bool              _Fp_IsSpace(int ch);
_Bool              _Fp_IsAlnum(int ch);
int                _Fp_ConvertLower(int ch);
int                _Fp_ConvertHexDigit(int ch);
unsigned long      _Fp_MatchWord(const void *str, _Fp_Text text, unsigned long index, const char *word);
unsigned long      _Fp_ReadExponent(const void *str, _Fp_Text text, unsigned long index, int letter, long *exp);
unsigned long long _Fp_ReadPayload(const void *str, _Fp_Text text, unsigned long index, unsigned long stop);
unsigned long      _Fp_ReadNotANumber(const void *str, _Fp_Text text, unsigned long index, unsigned long long *payload);
unsigned long      _Fp_ScanDecimal(const void *str, _Fp_Text text, unsigned long index, int point, _Fp_Decimal *dec);
void               _Fp_ConvertDecimal(const void *str, _Fp_Text text, int point, const _Fp_Decimal *dec, _Fp_Unrounded *value);
unsigned long      _Fp_ReadHex(const void *str, _Fp_Text text, unsigned long index, int point, _Fp_Unrounded *value);
void               _Fp_DropBits(_Fp_Unrounded *value, long bits);
void               _Fp_RoundReal(const _Fp_Unrounded *value, const _Fp_Format *fmt, _Fp_Real *real);
unsigned long      _Fp_ConvertReal(const void *str, _Fp_Text text, const _Fp_Format *fmt, _Fp_Real *real);
float              _Fp_ConvertFloat(const void *str, _Fp_Text text, unsigned long *end);
double             _Fp_ConvertDouble(const void *str, _Fp_Text text, unsigned long *end);
long double        _Fp_ConvertLongDouble(const void *str, _Fp_Text text, unsigned long *end);

#endif // _FP_H
