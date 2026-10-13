/*
 * C header file for formatted output.
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

#ifndef _PRINT_H
#define _PRINT_H

// The streams formatted output goes to.
#include <_file.h>

// A pointer to nothing, of any type.
#define _PRINT_NULL ((void *) 0)

// The greatest and the least int, and the greatest size.
#define _PRINT_INT_MAX  0x7FFFFFFF
#define _PRINT_INT_MIN  (-_PRINT_INT_MAX - 1)
#define _PRINT_SIZE_MAX ((unsigned long) -1)

// The most bytes a character has in any locale.
#define _PRINT_BYTES_MAX 16

// The character a number's fraction begins after, in either locale.
#define _PRINT_POINT '.'

// A long double's bits, the exponent above its greatest and a double's least.
#define _PRINT_LDOUBLE_MANT 64
#define _PRINT_LDOUBLE_EXP  16384
#define _PRINT_DOUBLE_MIN   (-1021)

// A directive's flags: -, +, a space, # and 0.
#define _PRINT_FLAG_LEFT  0x1
#define _PRINT_FLAG_SIGN  0x2
#define _PRINT_FLAG_SPACE 0x4
#define _PRINT_FLAG_ALT   0x8
#define _PRINT_FLAG_ZERO  0x10

// The base a width or a precision is written in.
#define _PRINT_COUNT_BASE 10

// The precision of a directive that gives none.
#define _PRINT_PRECISION_NONE (-1)

// The width or precision a directive leaves to an argument, until it is taken.
#define _PRINT_COUNT_ARGUMENT (-2)

// The lengths a conversion allows, each a bit: none alone, an integer's,
// a floating number's, and the l of a wide character or string.
#define _PRINT_LENGTHS_NONE    (1 << _PRINT_LENGTH_NONE)
#define _PRINT_LENGTHS_INTEGER (~(1 << _PRINT_LENGTH_LDOUBLE))
#define _PRINT_LENGTHS_REAL    (1 << _PRINT_LENGTH_NONE | 1 << _PRINT_LENGTH_LONG | 1 << _PRINT_LENGTH_LDOUBLE)
#define _PRINT_LENGTHS_WIDE    (1 << _PRINT_LENGTH_LONG)

// How many conversions there are, a letter in capitals being one of its own.
#define _PRINT_RULES 21

// The characters a field's padding is given by at a time.
#define _PRINT_PAD_CHUNK 32

// The bases an integer is written in, none for another conversion.
#define _PRINT_BASE_NONE    0
#define _PRINT_BASE_OCTAL   8
#define _PRINT_BASE_DECIMAL 10
#define _PRINT_BASE_HEX     16

// The digits of the largest base, and what leads an octal and a hex number.
#define _PRINT_DIGITS_LOWER "0123456789abcdef"
#define _PRINT_DIGITS_UPPER "0123456789ABCDEF"
#define _PRINT_PREFIX_OCTAL "0"
#define _PRINT_PREFIX_LOWER "0x"
#define _PRINT_PREFIX_UPPER "0X"

// The most digits an integer has, in octal.
#define _PRINT_DIGITS_MAX 22

// An integer conversion's precision if it gives none.
#define _PRINT_INTEGER_PRECISION 1

// The most digits a floating value has, a long double's 11514, and a few.
#define _PRINT_REAL_DIGITS 11520

// A floating conversion's precision if it gives none, and %g's if it gives 0.
#define _PRINT_REAL_PRECISION 6
#define _PRINT_REAL_LEAST     1

// The least exponent %g writes without one.
#define _PRINT_GENERAL_MIN (-4)

// The fewest digits an exponent has.
#define _PRINT_EXPONENT_MIN 2

// The bits of a hex digit, and the hex digits of a mantissa, its first too.
#define _PRINT_HEX_BITS   4
#define _PRINT_HEX_DIGITS 16

// The fewest digits a power of two has.
#define _PRINT_POWER_MIN 1

// The argument list a format's directives take from.
typedef __builtin_va_list _Print_VaList;

// The widest integers, which %jd and %ju convert.
typedef long          _Print_IntmaxType;
typedef unsigned long _Print_UintmaxType;

// A wide character, and one as an argument.
typedef int          _Print_WcharType;
typedef unsigned int _Print_WintType;

// Whether the sink took all it was given.
typedef enum _Print_Status _Print_Status;
enum _Print_Status {
    _PRINT_STATUS_GOOD,  // it has, so far
    _PRINT_STATUS_FAILED // a write failed, or the count overflowed
};

// A directive's length modifier.
typedef enum _Print_Length _Print_Length;
enum _Print_Length {
    _PRINT_LENGTH_NONE,    // none
    _PRINT_LENGTH_CHAR,    // hh
    _PRINT_LENGTH_SHORT,   // h
    _PRINT_LENGTH_LONG,    // l
    _PRINT_LENGTH_LLONG,   // ll
    _PRINT_LENGTH_INTMAX,  // j
    _PRINT_LENGTH_SIZE,    // z
    _PRINT_LENGTH_PTRDIFF, // t
    _PRINT_LENGTH_LDOUBLE  // L
};

// What a directive converts, the conversions alike in all but a field as one.
typedef enum _Print_Kind _Print_Kind;
enum _Print_Kind {
    _PRINT_KIND_PERCENT,    // %%, a percent sign
    _PRINT_KIND_CHAR,       // %c, a character
    _PRINT_KIND_STRING,     // %s, a string
    _PRINT_KIND_INTEGER,    // %d %i %u %o %x %X, an integer in a base
    _PRINT_KIND_POINTER,    // %p, a pointer
    _PRINT_KIND_COUNT,      // %n, the characters written so far, stored
    _PRINT_KIND_REAL,       // %f %F %e %E %g %G, a floating value in decimal
    _PRINT_KIND_HEX_REAL,   // %a %A, a floating value in hex
    _PRINT_KIND_WIDE_CHAR,  // %lc, a wide character
    _PRINT_KIND_WIDE_STRING // %ls, a wide string
};

// The argument a directive takes.
typedef enum _Print_Arg _Print_Arg;
enum _Print_Arg {
    _PRINT_ARG_NONE,       // none
    _PRINT_ARG_CHAR,       // an int, converted to a char
    _PRINT_ARG_STRING,     // a pointer to char
    _PRINT_ARG_SIGNED,     // a signed integer of the length's type
    _PRINT_ARG_UNSIGNED,   // an unsigned integer of the length's type
    _PRINT_ARG_POINTER,    // a pointer to void, to write
    _PRINT_ARG_DEST,       // a pointer to an integer, to store at
    _PRINT_ARG_REAL,       // a double, or a long double for L
    _PRINT_ARG_WIDE_CHAR,  // a wide character as an argument
    _PRINT_ARG_WIDE_STRING // a pointer to wide characters
};

// How a floating value is written in decimal.
typedef enum _Print_Style _Print_Style;
enum _Print_Style {
    _PRINT_STYLE_NONE,        // not at all, the conversion being another
    _PRINT_STYLE_FIXED,       // as %f does, with no exponent
    _PRINT_STYLE_EXPONENTIAL, // as %e does, one digit before its point
    _PRINT_STYLE_GENERAL      // as %g does, whichever is shorter
};

// The letters a conversion writes in.
typedef enum _Print_Case _Print_Case;
enum _Print_Case {
    _PRINT_CASE_LOWER, // lowercase, as %x and %e write
    _PRINT_CASE_UPPER  // capitals, as %X and %E write
};

// The sign of a number to write.
typedef enum _Print_Sign _Print_Sign;
enum _Print_Sign {
    _PRINT_SIGN_PLUS, // zero or more
    _PRINT_SIGN_MINUS // less than zero, or a set sign bit
};

// What a floating value is.
typedef enum _Print_Real _Print_Real;
enum _Print_Real {
    _PRINT_REAL_FINITE,   // a number, zero and the subnormals among them
    _PRINT_REAL_INFINITE, // an infinity
    _PRINT_REAL_NAN       // not a number
};

// Where formatted output goes: a stream, or else a string of a size.
typedef struct _Print_Sink _Print_Sink;
struct _Print_Sink {
    _File_Stream *ps_stream; // the stream, or _PRINT_NULL for a string
    char *ps_str;            // the string, where there is no stream
    unsigned long ps_size;   // the string's size, its null counted
    unsigned long ps_count;  // the characters given so far, kept or not
    _Print_Status ps_status; // whether it took them all
};

// A conversion: its letter, the lengths that go with it and what they mean.
typedef struct _Print_Rule _Print_Rule;
struct _Print_Rule {
    char pr_letter;        // the conversion's letter
    int pr_lengths;        // the lengths it allows, _PRINT_LENGTHS_*
    _Print_Kind pr_kind;   // what it converts
    _Print_Arg pr_arg;     // the argument it takes
    int pr_base;           // an integer's base, or _PRINT_BASE_NONE
    _Print_Style pr_style; // a floating value's style, or _PRINT_STYLE_NONE
    _Print_Case pr_case;   // the letters it writes in
};

// A directive as the format gives it, with the argument it converts.
typedef struct _Print_Directive _Print_Directive;
struct _Print_Directive {
    _Print_Kind pd_kind;             // what it converts
    int pd_flags;                    // its flags, _PRINT_FLAG_* together
    int pd_width;                    // its width, 0 for none
    int pd_precision;                // its precision, or _PRINT_PRECISION_NONE
    _Print_Length pd_length;         // its length modifier
    int pd_base;                     // an integer's base
    _Print_Style pd_style;           // a floating value's style
    _Print_Case pd_case;             // the letters it writes in
    _Print_Sign pd_sign;             // an integer's sign
    _Print_UintmaxType pd_uval;      // an integer's magnitude, or a character
    long double pd_fval;             // a floating value
    const char *pd_str;              // a string
    const _Print_WcharType *pd_wstr; // a wide string
    void *pd_dest;                   // where %n stores
};

// A floating value's decimal digits, no zero first or last.
typedef struct _Print_Numeral _Print_Numeral;
struct _Print_Numeral {
    _Print_Real pn_kind;               // finite, an infinity or a NaN
    _Print_Sign pn_sign;               // _PRINT_SIGN_MINUS for a set sign bit
    char pn_digit[_PRINT_REAL_DIGITS]; // the digits, the greatest first
    unsigned long pn_len;              // how many there are, 0 for zero
    int pn_exp;                        // the first's power of ten, 0 for zero
};

// The conversions, a letter that means two things by its length being two.
extern const _Print_Rule _Print_Rules[_PRINT_RULES];

// Sink
void _Print_PutText(_Print_Sink *sink, const char *data, unsigned long len);
void _Print_PutPad(_Print_Sink *sink, int ch, unsigned long len);
int  _Print_EndSink(_Print_Sink *sink);

// Matching
unsigned long      _Print_MatchText(const char *format);
unsigned long      _Print_MatchHead(const char *format, _Print_Directive *dir);
const _Print_Rule *_Print_FindRule(char letter, _Print_Length length);
void               _Print_TakeCounts(_Print_VaList *list, _Print_Directive *dir);
void               _Print_TakeArgument(_Print_VaList *list, _Print_Arg arg, _Print_Directive *dir);
unsigned long      _Print_MatchDirective(const char *format, _Print_VaList *list, _Print_Directive *dir);

// Converting
unsigned long _Print_ToDigits(char *end, _Print_UintmaxType value, int base, const char *digits);
_Print_Real   _Print_ToParts(long double value, _Print_Sign *sign, unsigned long long *mant, int *exp);
void          _Print_ToNumeral(long double value, _Print_Numeral *num);

// Numbers
void _Print_RoundNumeral(_Print_Numeral *num, long keep);
void _Print_StoreCount(void *dest, _Print_Length length, unsigned long count);

// Fields
void          _Print_WriteField(_Print_Sink *sink, int flags, int width, const char *data, unsigned long len);
unsigned long _Print_WriteLead(_Print_Sink *sink, int flags, int width, _Print_Sign sign, const char *prefix, unsigned long len);
void          _Print_WriteSpan(_Print_Sink *sink, const _Print_Numeral *num, long high, unsigned long count);
void          _Print_WriteNumeral(_Print_Sink *sink, int flags, int width, const _Print_Numeral *num, unsigned long frac, const char *mark);
void          _Print_WriteSpecial(_Print_Sink *sink, const _Print_Directive *dir, _Print_Sign sign, _Print_Real kind);
void          _Print_WriteMantissa(_Print_Sink *sink, const _Print_Directive *dir, _Print_Sign sign, unsigned long long mant, int exp);

// Formatted output
void _Print_WriteString(_Print_Sink *sink, const _Print_Directive *dir);
void _Print_WriteInteger(_Print_Sink *sink, const _Print_Directive *dir);
void _Print_WriteReal(_Print_Sink *sink, const _Print_Directive *dir);
void _Print_WriteHexReal(_Print_Sink *sink, const _Print_Directive *dir);
void _Print_WriteWideString(_Print_Sink *sink, const _Print_Directive *dir);
void _Print_WriteDirective(_Print_Sink *sink, const _Print_Directive *dir);
int  _Print_WriteFormat(_Print_Sink *sink, const char *format, _Print_VaList args);

#endif // _PRINT_H
