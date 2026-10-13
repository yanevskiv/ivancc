/*
 * C header file for formatted output and input.
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


#ifndef _FMT_H
#define _FMT_H

// The streams formatted output goes to and formatted input comes from.
#include <_file.h>

// A pointer to nothing, of any type.
#define _FMT_NULL ((void *) 0)

// The greatest and the least int, and the greatest size.
#define _FMT_INT_MAX  0x7FFFFFFF
#define _FMT_INT_MIN  (-_FMT_INT_MAX - 1)
#define _FMT_SIZE_MAX ((unsigned long) -1)

// The most bytes a character has in any locale.
#define _FMT_BYTES_MAX 16

// The character a number's fraction begins after, in either locale.
#define _FMT_POINT '.'

// A long double's bits, the exponent above its greatest and a double's least.
#define _FMT_LDOUBLE_MANT 64
#define _FMT_LDOUBLE_EXP  16384
#define _FMT_DOUBLE_MIN   (-1021)

// A directive's flags: -, +, a space, # and 0.
#define _FMT_FLAG_LEFT  0x1
#define _FMT_FLAG_SIGN  0x2
#define _FMT_FLAG_SPACE 0x4
#define _FMT_FLAG_ALT   0x8
#define _FMT_FLAG_ZERO  0x10

// The base a width or a precision is written in.
#define _FMT_COUNT_BASE 10

// The precision of a directive that gives none.
#define _FMT_PRECISION_NONE (-1)

// The width or precision a directive leaves to an argument.
#define _FMT_COUNT_ARGUMENT (-2)

// A directive's length modifier: none, hh, h, l, ll, j, z, t or L.
#define _FMT_LENGTH_NONE    0
#define _FMT_LENGTH_CHAR    1
#define _FMT_LENGTH_SHORT   2
#define _FMT_LENGTH_LONG    3
#define _FMT_LENGTH_LLONG   4
#define _FMT_LENGTH_INTMAX  5
#define _FMT_LENGTH_SIZE    6
#define _FMT_LENGTH_PTRDIFF 7
#define _FMT_LENGTH_LDOUBLE 8

// The characters a field's padding is given by at a time.
#define _FMT_PAD_CHUNK 32

// The bases an integer is written in, and the digits of the largest.
#define _FMT_BASE_OCTAL   8
#define _FMT_BASE_DECIMAL 10
#define _FMT_BASE_HEX     16
#define _FMT_DIGITS_LOWER "0123456789abcdef"
#define _FMT_DIGITS_UPPER "0123456789ABCDEF"

// The most digits an integer has, in octal.
#define _FMT_DIGITS_MAX 22

// An integer conversion's precision if it gives none.
#define _FMT_INTEGER_PRECISION 1

// The most digits a floating value has, a long double's 11514, and a few.
#define _FMT_REAL_DIGITS 11520

// A floating conversion's precision if it gives none, and %g's if it gives 0.
#define _FMT_REAL_PRECISION 6
#define _FMT_REAL_LEAST     1

// The least exponent %g writes without one.
#define _FMT_GENERAL_MIN (-4)

// The fewest digits an exponent has.
#define _FMT_EXPONENT_MIN 2

// The bits of a hex digit, and the hex digits of a mantissa, its first too.
#define _FMT_HEX_BITS   4
#define _FMT_HEX_DIGITS 16

// The fewest digits a power of two has.
#define _FMT_POWER_MIN 1

// The width of an input directive that gives none.
#define _FMT_SCAN_WIDTH_NONE 0

// An input directive's length modifier: none, hh, h, l, ll, j, z, t or L.
#define _FMT_SCAN_LENGTH_NONE    0
#define _FMT_SCAN_LENGTH_CHAR    1
#define _FMT_SCAN_LENGTH_SHORT   2
#define _FMT_SCAN_LENGTH_LONG    3
#define _FMT_SCAN_LENGTH_LLONG   4
#define _FMT_SCAN_LENGTH_INTMAX  5
#define _FMT_SCAN_LENGTH_SIZE    6
#define _FMT_SCAN_LENGTH_PTRDIFF 7
#define _FMT_SCAN_LENGTH_LDOUBLE 8

// An input integer's base: the one its prefix names, 8, 10 or 16.
#define _FMT_SCAN_BASE_PREFIX  0
#define _FMT_SCAN_BASE_OCTAL   8
#define _FMT_SCAN_BASE_DECIMAL 10
#define _FMT_SCAN_BASE_HEX     16

// The size of an array that holds an input integer: more digits overflow.
#define _FMT_SCAN_ITEM 32

// A null pointer as %p reads it, and the least width that holds it.
#define _FMT_SCAN_NIL       "(nil)"
#define _FMT_SCAN_NIL_WIDTH 5

// The conversions that read a floating number, and such a number's characters:
// a sign, a digit of either base, the point, either exponent's mark, the
// words, and what a NaN's parentheses hold between them.
#define _FMT_SCAN_REALS    "aefgAEFG"
#define _FMT_SCAN_SIGNS    "+-"
#define _FMT_SCAN_DECIMALS "0123456789"
#define _FMT_SCAN_HEXES    "0123456789abcdef"
#define _FMT_SCAN_POINT    "."
#define _FMT_SCAN_EXPONENT "e"
#define _FMT_SCAN_POWER    "p"
#define _FMT_SCAN_INFINITY "infinity"
#define _FMT_SCAN_NAN      "nan"
#define _FMT_SCAN_NCHARS   "0123456789abcdefghijklmnopqrstuvwxyz_"

// The argument list a format's directives take from.
typedef __builtin_va_list _Fmt_VaList;

// The widest integers, which %jd and %ju convert.
typedef long          _Fmt_IntmaxType;
typedef unsigned long _Fmt_UintmaxType;

// A wide character, and one as an argument.
typedef int          _Fmt_WcharType;
typedef unsigned int _Fmt_WintType;

// The letters a conversion writes in.
typedef enum _Fmt_Case _Fmt_Case;
enum _Fmt_Case {
    _FMT_CASE_LOWER, // lowercase, as %x and %e write
    _FMT_CASE_UPPER  // capitals, as %X and %E write
};

// The sign of a number to write.
typedef enum _Fmt_Sign _Fmt_Sign;
enum _Fmt_Sign {
    _FMT_SIGN_PLUS, // zero or more
    _FMT_SIGN_MINUS // less than zero, or a set sign bit
};

// What a floating value is.
typedef enum _Fmt_Real _Fmt_Real;
enum _Fmt_Real {
    _FMT_REAL_FINITE,   // a number, zero and the subnormals among them
    _FMT_REAL_INFINITE, // an infinity
    _FMT_REAL_NAN       // not a number
};

// What an input directive does with what it converts.
typedef enum _Fmt_Assign _Fmt_Assign;
enum _Fmt_Assign {
    _FMT_ASSIGN_STORE,   // store it at the next argument
    _FMT_ASSIGN_SUPPRESS // store nothing, the directive having an asterisk
};

// The characters a scanset takes: those it lists, or all the others.
typedef enum _Fmt_Accept _Fmt_Accept;
enum _Fmt_Accept {
    _FMT_ACCEPT_LISTED,  // the characters between its brackets
    _FMT_ACCEPT_UNLISTED // every other, the scanset beginning with ^
};

// What stopped formatted input: nothing, no input or input that does not fit.
typedef enum _Fmt_Failure _Fmt_Failure;
enum _Fmt_Failure {
    _FMT_FAILURE_NONE,    // nothing has
    _FMT_FAILURE_INPUT,   // the input ended, could not be read or decoded
    _FMT_FAILURE_MATCHING // the input is not what a directive takes
};

// Where formatted output goes: a stream, or else a string of a size.
typedef struct _Fmt_Sink _Fmt_Sink;
struct _Fmt_Sink {
    _File_Stream *fs_stream; // the stream, or _FMT_NULL for a string
    char *fs_str;            // the string, where there is no stream
    unsigned long fs_size;   // the string's size, its null counted
    unsigned long fs_count;  // the characters given so far, kept or not
    _Bool fs_failed;         // a write failed, or the count overflowed
};

// A floating value's decimal digits, no zero first or last.
typedef struct _Fmt_Numeral _Fmt_Numeral;
struct _Fmt_Numeral {
    _Fmt_Real fn_kind;               // finite, an infinity or a NaN
    _Fmt_Sign fn_sign;               // _FMT_SIGN_MINUS for a set sign bit
    char fn_digit[_FMT_REAL_DIGITS]; // the digits, the greatest first
    unsigned long fn_len;            // how many there are, 0 for zero
    int fn_exp;                      // the first's power of ten, 0 for zero
};

// Where formatted input comes from: a stream, or else a string.
typedef struct _Fmt_Source _Fmt_Source;
struct _Fmt_Source {
    _File_Stream *fo_stream; // the stream, or _FMT_NULL for a string
    const char *fo_str;      // the string, where there is no stream
    unsigned long fo_count;  // the characters read so far
    int fo_stored;           // the assignments made so far
    _Fmt_Failure fo_failure; // what stopped the input, if anything has
};

// An input item of any length: its characters, and how many more it may take.
typedef struct _Fmt_Item _Fmt_Item;
struct _Fmt_Item {
    char *fi_text;         // the characters and a null, allocated, or null
    unsigned long fi_len;  // how many characters there are
    unsigned long fi_size; // the bytes fi_text points to
    unsigned long fi_room; // the characters the directive's width still allows
};

// The wide characters an input directive stores, decoded a byte at a time.
typedef struct _Fmt_Wide _Fmt_Wide;
struct _Fmt_Wide {
    _Fmt_WcharType *fw_dest;      // where the next one goes, or _FMT_NULL
    char fw_byte[_FMT_BYTES_MAX]; // the bytes of a character not yet whole
    unsigned long fw_count;       // how many such bytes there are
};

// Characters
_Bool _Fmt_IsSpace(int ch);
_Bool _Fmt_IsDigit(int ch);
_Bool _Fmt_IsHexDigit(int ch);

// Converting
int           _Fmt_ToLower(int ch);
long          _Fmt_ToSigned(const char *str, int base);
unsigned long _Fmt_ToUnsigned(const char *str, int base);
_Bool         _Fmt_ToReal(const char *str, int length, void *dest);

// Print matching
unsigned long    _Fmt_MatchPrintHead(const char *format, int *flags, int *width, int *precision, int *length);
void             _Fmt_TakeHead(_Fmt_VaList *list, int *flags, int *width, int *precision);
_Fmt_UintmaxType _Fmt_TakeUnsigned(_Fmt_VaList *list, int length);
_Bool            _Fmt_MatchPrintText(const char *format, unsigned long *len);
_Bool            _Fmt_MatchPrintPercent(const char *format, _Fmt_VaList *list, unsigned long *len);
_Bool            _Fmt_MatchPrintChar(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, char *ch);
_Bool            _Fmt_MatchPrintString(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, const char **str);
_Bool            _Fmt_MatchPrintDecimal(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, _Fmt_IntmaxType *value);
_Bool            _Fmt_MatchPrintUnsigned(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, _Fmt_UintmaxType *value);
_Bool            _Fmt_MatchPrintOctal(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, _Fmt_UintmaxType *value);
_Bool            _Fmt_MatchPrintHex(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, _Fmt_UintmaxType *value, _Fmt_Case *letters);
_Bool            _Fmt_MatchPrintPointer(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, const void **ptr);
_Bool            _Fmt_MatchPrintCount(const char *format, _Fmt_VaList *list, unsigned long *len, int *length, void **dest);
_Bool            _Fmt_MatchPrintFixed(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, long double *value, _Fmt_Case *letters);
_Bool            _Fmt_MatchPrintExponential(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, long double *value, _Fmt_Case *letters);
_Bool            _Fmt_MatchPrintGeneral(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, long double *value, _Fmt_Case *letters);
_Bool            _Fmt_MatchPrintHexReal(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, double *value, _Fmt_Case *letters);
_Bool            _Fmt_MatchPrintHexLongReal(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, long double *value, _Fmt_Case *letters);
_Bool            _Fmt_MatchPrintWideChar(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, _Fmt_WcharType *wc);
_Bool            _Fmt_MatchPrintWideString(const char *format, _Fmt_VaList *list, unsigned long *len, int *flags, int *width, int *precision, const _Fmt_WcharType **wstr);

// Formatted output
unsigned long _Fmt_WriteDigits(char *end, _Fmt_UintmaxType value, int base, const char *digits);
void          _Fmt_StoreCount(void *dest, int length, unsigned long count);
_Fmt_Real     _Fmt_UnpackReal(long double value, _Fmt_Sign *sign, unsigned long long *mant, int *exp);
void          _Fmt_WriteNumeral(long double value, _Fmt_Numeral *num);
void          _Fmt_RoundNumeral(_Fmt_Numeral *num, long keep);
int           _Fmt_EndSink(_Fmt_Sink *sink);

// Emitting
void          _Fmt_EmitText(_Fmt_Sink *sink, const char *data, unsigned long len);
void          _Fmt_EmitPad(_Fmt_Sink *sink, int ch, unsigned long len);
void          _Fmt_EmitField(_Fmt_Sink *sink, int flags, int width, const char *data, unsigned long len);
unsigned long _Fmt_EmitLead(_Fmt_Sink *sink, int flags, int width, _Fmt_Sign sign, const char *prefix, unsigned long len);
void          _Fmt_EmitInteger(_Fmt_Sink *sink, int flags, int width, int precision, _Fmt_Sign sign, const char *prefix, const char *digits, unsigned long len);
void          _Fmt_EmitSpan(_Fmt_Sink *sink, const _Fmt_Numeral *num, long high, unsigned long count);
void          _Fmt_EmitNumeral(_Fmt_Sink *sink, int flags, int width, const _Fmt_Numeral *num, unsigned long frac, const char *mark);
void          _Fmt_EmitSpecial(_Fmt_Sink *sink, int flags, int width, _Fmt_Sign sign, _Fmt_Real kind, _Fmt_Case letters);
void          _Fmt_EmitMantissa(_Fmt_Sink *sink, int flags, int width, int precision, _Fmt_Sign sign, unsigned long long mant, int exp, _Fmt_Case letters);
void          _Fmt_EmitChar(_Fmt_Sink *sink, int flags, int width, char ch);
void          _Fmt_EmitString(_Fmt_Sink *sink, int flags, int width, int precision, const char *str);
void          _Fmt_EmitDecimal(_Fmt_Sink *sink, int flags, int width, int precision, _Fmt_IntmaxType value);
void          _Fmt_EmitUnsigned(_Fmt_Sink *sink, int flags, int width, int precision, _Fmt_UintmaxType value);
void          _Fmt_EmitOctal(_Fmt_Sink *sink, int flags, int width, int precision, _Fmt_UintmaxType value);
void          _Fmt_EmitHex(_Fmt_Sink *sink, int flags, int width, int precision, _Fmt_UintmaxType value, _Fmt_Case letters);
void          _Fmt_EmitPointer(_Fmt_Sink *sink, int flags, int width, int precision, const void *ptr);
void          _Fmt_EmitFixed(_Fmt_Sink *sink, int flags, int width, int precision, long double value, _Fmt_Case letters);
void          _Fmt_EmitExponential(_Fmt_Sink *sink, int flags, int width, int precision, long double value, _Fmt_Case letters);
void          _Fmt_EmitGeneral(_Fmt_Sink *sink, int flags, int width, int precision, long double value, _Fmt_Case letters);
void          _Fmt_EmitHexReal(_Fmt_Sink *sink, int flags, int width, int precision, double value, _Fmt_Case letters);
void          _Fmt_EmitHexLongReal(_Fmt_Sink *sink, int flags, int width, int precision, long double value, _Fmt_Case letters);
void          _Fmt_EmitWideChar(_Fmt_Sink *sink, int flags, int width, _Fmt_WcharType wc);
void          _Fmt_EmitWideString(_Fmt_Sink *sink, int flags, int width, int precision, const _Fmt_WcharType *wstr);
int           _Fmt_EmitFormat(_Fmt_Sink *sink, const char *format, _Fmt_VaList args);

// Scan matching
unsigned long _Fmt_MatchScanHead(const char *format, _Fmt_Assign *assign, int *width, int *length);
_Bool         _Fmt_MatchScanSpace(const char *format, unsigned long *len);
_Bool         _Fmt_MatchScanOrdinary(const char *format, unsigned long *len);
_Bool         _Fmt_MatchScanPercent(const char *format, unsigned long *len);
_Bool         _Fmt_MatchScanChar(const char *format, _Fmt_VaList *list, unsigned long *len, int *width, char **dest);
_Bool         _Fmt_MatchScanString(const char *format, _Fmt_VaList *list, unsigned long *len, int *width, char **dest);
_Bool         _Fmt_MatchScanSet(const char *format, _Fmt_VaList *list, unsigned long *len, int *width, char **dest, const char **set, unsigned long *count, _Fmt_Accept *accept);
_Bool         _Fmt_MatchScanCount(const char *format, _Fmt_VaList *list, unsigned long *len, int *length, void **dest);
_Bool         _Fmt_MatchScanDecimal(const char *format, _Fmt_VaList *list, unsigned long *len, int *width, int *length, void **dest);
_Bool         _Fmt_MatchScanInteger(const char *format, _Fmt_VaList *list, unsigned long *len, int *width, int *length, void **dest);
_Bool         _Fmt_MatchScanOctal(const char *format, _Fmt_VaList *list, unsigned long *len, int *width, int *length, void **dest);
_Bool         _Fmt_MatchScanUnsigned(const char *format, _Fmt_VaList *list, unsigned long *len, int *width, int *length, void **dest);
_Bool         _Fmt_MatchScanHex(const char *format, _Fmt_VaList *list, unsigned long *len, int *width, int *length, void **dest);
_Bool         _Fmt_MatchScanPointer(const char *format, _Fmt_VaList *list, unsigned long *len, int *width, void **dest);
_Bool         _Fmt_MatchScanReal(const char *format, _Fmt_VaList *list, unsigned long *len, int *width, int *length, void **dest);
_Bool         _Fmt_MatchScanWideChar(const char *format, _Fmt_VaList *list, unsigned long *len, int *width, _Fmt_WcharType **dest);
_Bool         _Fmt_MatchScanWideString(const char *format, _Fmt_VaList *list, unsigned long *len, int *width, _Fmt_WcharType **dest);
_Bool         _Fmt_MatchScanWideSet(const char *format, _Fmt_VaList *list, unsigned long *len, int *width, _Fmt_WcharType **dest, const char **set, unsigned long *count, _Fmt_Accept *accept);

// Formatted input
int           _Fmt_PeekSource(_Fmt_Source *source);
void          _Fmt_AdvanceSource(_Fmt_Source *source);
int           _Fmt_EndSource(_Fmt_Source *source);
_Bool         _Fmt_IsInSet(const char *set, unsigned long count, int ch);
_Bool         _Fmt_ReadInteger(_Fmt_Source *source, int width, int base, char *item);
void          _Fmt_StoreInteger(void *dest, int length, unsigned long long value);
unsigned long _Fmt_TakeChars(_Fmt_Source *source, _Fmt_Item *item, const char *set, unsigned long max);
unsigned long _Fmt_TakeWord(_Fmt_Source *source, _Fmt_Item *item, const char *word);
void          _Fmt_ReadReal(_Fmt_Source *source, int width, _Fmt_Item *item);
void          _Fmt_DecodeByte(_Fmt_Source *source, _Fmt_Wide *wide, int ch);
_Bool         _Fmt_EndWide(_Fmt_Source *source, const _Fmt_Wide *wide);

// Scanning
void _Fmt_ScanSpace(_Fmt_Source *source);
void _Fmt_ScanOrdinary(_Fmt_Source *source, const char *text, unsigned long len);
void _Fmt_ScanPercent(_Fmt_Source *source);
void _Fmt_ScanChar(_Fmt_Source *source, int width, char *dest);
void _Fmt_ScanString(_Fmt_Source *source, int width, char *dest);
void _Fmt_ScanSet(_Fmt_Source *source, int width, char *dest, const char *set, unsigned long count, _Fmt_Accept accept);
void _Fmt_ScanCount(_Fmt_Source *source, int length, void *dest);
void _Fmt_ScanDecimal(_Fmt_Source *source, int width, int length, void *dest);
void _Fmt_ScanInteger(_Fmt_Source *source, int width, int length, void *dest);
void _Fmt_ScanOctal(_Fmt_Source *source, int width, int length, void *dest);
void _Fmt_ScanUnsigned(_Fmt_Source *source, int width, int length, void *dest);
void _Fmt_ScanHex(_Fmt_Source *source, int width, int length, void *dest);
void _Fmt_ScanPointer(_Fmt_Source *source, int width, void *dest);
void _Fmt_ScanReal(_Fmt_Source *source, int width, int length, void *dest);
void _Fmt_ScanWideChar(_Fmt_Source *source, int width, _Fmt_WcharType *dest);
void _Fmt_ScanWideString(_Fmt_Source *source, int width, _Fmt_WcharType *dest);
void _Fmt_ScanWideSet(_Fmt_Source *source, int width, _Fmt_WcharType *dest, const char *set, unsigned long count, _Fmt_Accept accept);
int  _Fmt_ScanFormat(_Fmt_Source *source, const char *format, _Fmt_VaList args);


#endif // _FMT_H
