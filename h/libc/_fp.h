/*
 * C header file for the conversion of text to a floating number.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivcc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivcc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivcc; see
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
    _FP_TEXT_NARROW, // a string of char
    _FP_TEXT_WIDE    // a string of wide characters
};

// A floating type, as the conversion needs to know it.
typedef struct _Fp_Format _Fp_Format;
struct _Fp_Format {
    int ff_mant;       // the bits of its mantissa
    int ff_min;        // its least exponent, one below C's
    int ff_max;        // its greatest exponent, one below C's
    _Bool ff_explicit; // whether it keeps its mantissa's leading bit
};

// A converted number, as the fields of its type.
typedef struct _Fp_Real _Fp_Real;
struct _Fp_Real {
    _Bool fr_negative;          // whether its sign bit is set
    unsigned int fr_exp;        // its biased exponent
    unsigned long long fr_mant; // its mantissa
};

// A number before it is rounded to a type.
typedef struct _Fp_Unrounded _Fp_Unrounded;
struct _Fp_Unrounded {
    unsigned long long fu_mant; // its first 64 bits, from the top one
    long fu_exp;                // the top bit's power of two
    int fu_rest;                // what lies below them, an _FP_REST_ value
};

// A decimal as it was scanned, before its digits are converted.
typedef struct _Fp_Decimal _Fp_Decimal;
struct _Fp_Decimal {
    unsigned long fd_first; // where its first significant digit is
    long fd_count;          // the digits kept, _FP_DIGITS at most
    long fd_exp;            // the last kept digit's power of ten
    _Bool fd_sticky;        // whether a digit that was dropped is nonzero
};

// The formats of float, double and long double.
extern const _Fp_Format _Fp_FloatFormat;
extern const _Fp_Format _Fp_DoubleFormat;
extern const _Fp_Format _Fp_LongDoubleFormat;

// Reading
int                _Fp_ReadChar(const void *str, _Fp_Text text, unsigned long index);
_Bool              _Fp_IsSpace(int ch);
_Bool              _Fp_IsAlnum(int ch);
unsigned long      _Fp_MatchWord(const void *str, _Fp_Text text, unsigned long index, const char *word);
unsigned long      _Fp_ReadExponent(const void *str, _Fp_Text text, unsigned long index, int letter, long *exp);
unsigned long long _Fp_ReadPayload(const void *str, _Fp_Text text, unsigned long index, unsigned long stop);
unsigned long      _Fp_ReadNotANumber(const void *str, _Fp_Text text, unsigned long index, unsigned long long *payload);
unsigned long      _Fp_ScanDecimal(const void *str, _Fp_Text text, unsigned long index, int point, _Fp_Decimal *dec);
unsigned long      _Fp_ReadHex(const void *str, _Fp_Text text, unsigned long index, int point, _Fp_Unrounded *value);
void               _Fp_DropBits(_Fp_Unrounded *value, long bits);
void               _Fp_RoundReal(const _Fp_Unrounded *value, const _Fp_Format *fmt, _Fp_Real *real);

// Converting
int           _Fp_ToLower(int ch);
int           _Fp_ToHexDigit(int ch);
void          _Fp_ToUnrounded(const void *str, _Fp_Text text, int point, const _Fp_Decimal *dec, _Fp_Unrounded *value);
unsigned long _Fp_ToReal(const void *str, _Fp_Text text, const _Fp_Format *fmt, _Fp_Real *real);
float         _Fp_ToFloat(const void *str, _Fp_Text text, unsigned long *end);
double        _Fp_ToDouble(const void *str, _Fp_Text text, unsigned long *end);
long double   _Fp_ToLongDouble(const void *str, _Fp_Text text, unsigned long *end);

#endif // _FP_H
