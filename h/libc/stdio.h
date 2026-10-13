/*
 * C header file for input/output.
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

#ifndef _STDIO_H
#define _STDIO_H

// A directive's flags: -, +, a space, # and 0.
#define _STDIO_FLAG_LEFT  0x1
#define _STDIO_FLAG_SIGN  0x2
#define _STDIO_FLAG_SPACE 0x4
#define _STDIO_FLAG_ALT   0x8
#define _STDIO_FLAG_ZERO  0x10

// The base a width or a precision is written in.
#define _STDIO_COUNT_BASE 10

// The precision of a directive that gives none.
#define _STDIO_PRECISION_NONE (-1)

// The width or precision a directive leaves to an argument.
#define _STDIO_COUNT_ARGUMENT (-2)

// A directive's length modifier: none, hh, h, l, ll, j, z, t or L.
#define _STDIO_LENGTH_NONE    0
#define _STDIO_LENGTH_CHAR    1
#define _STDIO_LENGTH_SHORT   2
#define _STDIO_LENGTH_LONG    3
#define _STDIO_LENGTH_LLONG   4
#define _STDIO_LENGTH_INTMAX  5
#define _STDIO_LENGTH_SIZE    6
#define _STDIO_LENGTH_PTRDIFF 7
#define _STDIO_LENGTH_LDOUBLE 8

// The characters a field's padding is given by at a time.
#define _STDIO_PAD_CHUNK 32

// The bases an integer is written in, and the digits of the largest.
#define _STDIO_BASE_OCTAL   8
#define _STDIO_BASE_DECIMAL 10
#define _STDIO_BASE_HEX     16
#define _STDIO_DIGITS_LOWER "0123456789abcdef"
#define _STDIO_DIGITS_UPPER "0123456789ABCDEF"

// The most digits an integer has, in octal.
#define _STDIO_DIGITS_MAX 22

// An integer conversion's precision if it gives none.
#define _STDIO_INTEGER_PRECISION 1

// The most digits a floating value has, a long double's 11514, and a few.
#define _STDIO_REAL_DIGITS 11520

// A floating conversion's precision if it gives none, and %g's if it gives 0.
#define _STDIO_REAL_PRECISION 6
#define _STDIO_REAL_LEAST     1

// The least exponent %g writes without one.
#define _STDIO_GENERAL_MIN (-4)

// The fewest digits an exponent has.
#define _STDIO_EXPONENT_MIN 2

// The bits of a hex digit, and the hex digits of a mantissa, its first too.
#define _STDIO_HEX_BITS   4
#define _STDIO_HEX_DIGITS 16

// The fewest digits a power of two has.
#define _STDIO_POWER_MIN 1

// The width of an input directive that gives none.
#define _STDIO_SCAN_WIDTH_NONE 0

// An input directive's length modifier: none, hh, h, l, ll, j, z, t or L.
#define _STDIO_SCAN_LENGTH_NONE    0
#define _STDIO_SCAN_LENGTH_CHAR    1
#define _STDIO_SCAN_LENGTH_SHORT   2
#define _STDIO_SCAN_LENGTH_LONG    3
#define _STDIO_SCAN_LENGTH_LLONG   4
#define _STDIO_SCAN_LENGTH_INTMAX  5
#define _STDIO_SCAN_LENGTH_SIZE    6
#define _STDIO_SCAN_LENGTH_PTRDIFF 7
#define _STDIO_SCAN_LENGTH_LDOUBLE 8

// An input integer's base: the one its prefix names, 8, 10 or 16.
#define _STDIO_SCAN_BASE_PREFIX  0
#define _STDIO_SCAN_BASE_OCTAL   8
#define _STDIO_SCAN_BASE_DECIMAL 10
#define _STDIO_SCAN_BASE_HEX     16

// The size of an array that holds an input integer: more digits overflow.
#define _STDIO_SCAN_ITEM 32

// A null pointer as %p reads it, and the least width that holds it.
#define _STDIO_SCAN_NIL       "(nil)"
#define _STDIO_SCAN_NIL_WIDTH 5

// The conversions that read a floating number, and such a number's characters:
// a sign, a digit of either base, the point, either exponent's mark, the
// words, and what a NaN's parentheses hold between them.
#define _STDIO_SCAN_REALS    "aefgAEFG"
#define _STDIO_SCAN_SIGNS    "+-"
#define _STDIO_SCAN_DECIMALS "0123456789"
#define _STDIO_SCAN_HEXES    "0123456789abcdef"
#define _STDIO_SCAN_POINT    "."
#define _STDIO_SCAN_EXPONENT "e"
#define _STDIO_SCAN_POWER    "p"
#define _STDIO_SCAN_INFINITY "infinity"
#define _STDIO_SCAN_NAN      "nan"
#define _STDIO_SCAN_NCHARS   "0123456789abcdefghijklmnopqrstuvwxyz_"

// The most bytes of an input character not yet whole, which is MB_LEN_MAX.
#define _STDIO_SCAN_BYTES 16

// (S7.19.1) Introduction
#define NULL ((void *) 0)

// The buffering setvbuf takes: full, by line and none.
#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

// The size of the buffer setbuf takes.
#define BUFSIZ 8192

// What a function returns at the end of a file.
#define EOF (-1)

// The files a program is sure to have open at once.
#define FOPEN_MAX 16

// The size of an array that holds the longest file name.
#define FILENAME_MAX 4096

// The size of an array that holds a name tmpnam makes.
#define L_tmpnam 20

// The origins fseek takes: the position, the end and the start.
#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0

// The names tmpnam is sure to make, no two alike.
#define TMP_MAX 10000

// The standard streams, by the names a program gives them.
#define stderr (&_File_Stderr)
#define stdin  (&_File_Stdin)
#define stdout (&_File_Stdout)

// The argument list, which stdio.h may not name.
typedef __builtin_va_list _Stdio_VaList;

// The widest integers, which stdio.h may not name.
typedef long          _Stdio_IntmaxType;
typedef unsigned long _Stdio_UintmaxType;

// A wide character, and one as an argument, which stdio.h may not name.
typedef int          _Stdio_WcharType;
typedef unsigned int _Stdio_WintType;

#ifndef __SIZE_T__
#define __SIZE_T__
typedef unsigned long size_t;
#endif

typedef long fpos_t;

// The letters a conversion writes in.
typedef enum _Stdio_Case _Stdio_Case;
enum _Stdio_Case {
    _STDIO_CASE_LOWER,
    _STDIO_CASE_UPPER
};

// The sign of a number to write.
typedef enum _Stdio_Sign _Stdio_Sign;
enum _Stdio_Sign {
    _STDIO_SIGN_PLUS,
    _STDIO_SIGN_MINUS
};

// What a floating value is.
typedef enum _Stdio_Real _Stdio_Real;
enum _Stdio_Real {
    _STDIO_REAL_FINITE,
    _STDIO_REAL_INFINITE,
    _STDIO_REAL_NAN
};

// What an input directive does with what it converts.
typedef enum _Stdio_Assign _Stdio_Assign;
enum _Stdio_Assign {
    _STDIO_ASSIGN_STORE,
    _STDIO_ASSIGN_SUPPRESS
};

// The characters a scanset takes: those it lists, or all the others.
typedef enum _Stdio_Accept _Stdio_Accept;
enum _Stdio_Accept {
    _STDIO_ACCEPT_LISTED,
    _STDIO_ACCEPT_UNLISTED
};

// What stopped formatted input: nothing, no input or input that does not fit.
typedef enum _Stdio_Failure _Stdio_Failure;
enum _Stdio_Failure {
    _STDIO_FAILURE_NONE,
    _STDIO_FAILURE_INPUT,
    _STDIO_FAILURE_MATCHING
};

// A stream, as _file.h defines it, which this header may not include.
#ifndef __FILE_STREAM__
#define __FILE_STREAM__
typedef struct _File_Stream _File_Stream;
struct _File_Stream {
    int fs_fd;             // the file's descriptor, or _FILE_NOFILE
    int fs_flags;          // _FILE_READ, _FILE_WRITE and what happened
    int fs_state;          // _FILE_IDLE, _FILE_READING, _FILE_WRITING
    int fs_mode;           // _FILE_FULL, _FILE_LINE, _FILE_NONE or _FILE_AUTO
    unsigned char *fs_buf; // the buffer, the stream's own or one it was given
    unsigned long fs_size; // the buffer's size
    unsigned long fs_head; // the bytes of the buffer already read
    unsigned long fs_tail; // the bytes the buffer holds, read or to write
    unsigned char fs_byte; // an unbuffered stream's buffer
    unsigned char fs_back; // the character pushed back
    _File_Stream *fs_next; // the next of the open streams
};
#endif

// Where formatted output goes: a stream, or else a string of a size.
typedef struct _Stdio_Sink _Stdio_Sink;
struct _Stdio_Sink {
    _File_Stream *ss_stream; // the stream, or NULL for a string
    char *ss_str;            // the string, where there is no stream
    size_t ss_size;          // the string's size, its null counted
    size_t ss_count;         // the characters given so far, kept or not
    _Bool ss_failed;         // a write failed, or the count overflowed
};

// A floating value's decimal digits, no zero first or last.
typedef struct _Stdio_Numeral _Stdio_Numeral;
struct _Stdio_Numeral {
    _Stdio_Real sn_kind;               // finite, an infinity or a NaN
    _Stdio_Sign sn_sign;               // _STDIO_SIGN_MINUS for a set sign bit
    char sn_digit[_STDIO_REAL_DIGITS]; // the digits, the greatest first
    size_t sn_len;                     // how many there are, 0 for zero
    int sn_exp;                        // the first's power of ten, 0 for zero
};

// Where formatted input comes from: a stream, or else a string.
typedef struct _Stdio_Source _Stdio_Source;
struct _Stdio_Source {
    _File_Stream *so_stream;   // the stream, or NULL for a string
    const char *so_str;        // the string, where there is no stream
    size_t so_count;           // the characters read so far
    int so_stored;             // the assignments made so far
    _Stdio_Failure so_failure; // what stopped the input, if anything has
};

// An input item of any length: its characters, and how many more it may take.
typedef struct _Stdio_Item _Stdio_Item;
struct _Stdio_Item {
    char *si_text;  // the characters and a null, allocated, or NULL for none
    size_t si_len;  // how many characters there are
    size_t si_size; // the bytes si_text points to
    size_t si_room; // the characters the directive's width still allows
};

// The wide characters an input directive stores, decoded a byte at a time.
typedef struct _Stdio_Wide _Stdio_Wide;
struct _Stdio_Wide {
    _Stdio_WcharType *sw_dest;       // where the next one goes, or NULL
    char sw_byte[_STDIO_SCAN_BYTES]; // the bytes of a character not yet whole
    size_t sw_count;                 // how many such bytes there are
};

// (S7.19.1) Introduction
typedef struct _File_Stream FILE;

// The standard streams, which _file.c defines.
extern FILE _File_Stdin;
extern FILE _File_Stdout;
extern FILE _File_Stderr;

// Print matching
size_t             _Stdio_MatchPrintHead(const char *format, int *flags, int *width, int *precision, int *length);
void               _Stdio_TakeHead(_Stdio_VaList *list, int *flags, int *width, int *precision);
_Stdio_UintmaxType _Stdio_TakeUnsigned(_Stdio_VaList *list, int length);
_Bool              _Stdio_MatchPrintText(const char *format, size_t *len);
_Bool              _Stdio_MatchPrintPercent(const char *format, _Stdio_VaList *list, size_t *len);
_Bool              _Stdio_MatchPrintChar(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, char *ch);
_Bool              _Stdio_MatchPrintString(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, const char **str);
_Bool              _Stdio_MatchPrintDecimal(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_IntmaxType *value);
_Bool              _Stdio_MatchPrintUnsigned(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_UintmaxType *value);
_Bool              _Stdio_MatchPrintOctal(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_UintmaxType *value);
_Bool              _Stdio_MatchPrintHex(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_UintmaxType *value, _Stdio_Case *letters);
_Bool              _Stdio_MatchPrintPointer(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, const void **ptr);
_Bool              _Stdio_MatchPrintCount(const char *format, _Stdio_VaList *list, size_t *len, int *length, void **dest);
_Bool              _Stdio_MatchPrintFixed(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Stdio_Case *letters);
_Bool              _Stdio_MatchPrintExponential(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Stdio_Case *letters);
_Bool              _Stdio_MatchPrintGeneral(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Stdio_Case *letters);
_Bool              _Stdio_MatchPrintHexReal(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, double *value, _Stdio_Case *letters);
_Bool              _Stdio_MatchPrintHexLongReal(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Stdio_Case *letters);
_Bool              _Stdio_MatchPrintWideChar(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, _Stdio_WcharType *wc);
_Bool              _Stdio_MatchPrintWideString(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, const _Stdio_WcharType **wstr);

// Formatted output
size_t      _Stdio_WriteDigits(char *end, _Stdio_UintmaxType value, int base, const char *digits);
void        _Stdio_StoreCount(void *dest, int length, size_t count);
_Stdio_Real _Stdio_UnpackReal(long double value, _Stdio_Sign *sign, unsigned long long *mant, int *exp);
void        _Stdio_WriteNumeral(long double value, _Stdio_Numeral *num);
void        _Stdio_RoundNumeral(_Stdio_Numeral *num, long keep);
int         _Stdio_EndSink(_Stdio_Sink *sink);

// Emitting
void   _Stdio_EmitText(_Stdio_Sink *sink, const char *data, size_t len);
void   _Stdio_EmitPad(_Stdio_Sink *sink, int ch, size_t len);
void   _Stdio_EmitField(_Stdio_Sink *sink, int flags, int width, const char *data, size_t len);
size_t _Stdio_EmitLead(_Stdio_Sink *sink, int flags, int width, _Stdio_Sign sign, const char *prefix, size_t len);
void   _Stdio_EmitInteger(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_Sign sign, const char *prefix, const char *digits, size_t len);
void   _Stdio_EmitSpan(_Stdio_Sink *sink, const _Stdio_Numeral *num, long high, size_t count);
void   _Stdio_EmitNumeral(_Stdio_Sink *sink, int flags, int width, const _Stdio_Numeral *num, size_t frac, const char *mark);
void   _Stdio_EmitSpecial(_Stdio_Sink *sink, int flags, int width, _Stdio_Sign sign, _Stdio_Real kind, _Stdio_Case letters);
void   _Stdio_EmitMantissa(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_Sign sign, unsigned long long mant, int exp, _Stdio_Case letters);
void   _Stdio_EmitChar(_Stdio_Sink *sink, int flags, int width, char ch);
void   _Stdio_EmitString(_Stdio_Sink *sink, int flags, int width, int precision, const char *str);
void   _Stdio_EmitDecimal(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_IntmaxType value);
void   _Stdio_EmitUnsigned(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_UintmaxType value);
void   _Stdio_EmitOctal(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_UintmaxType value);
void   _Stdio_EmitHex(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_UintmaxType value, _Stdio_Case letters);
void   _Stdio_EmitPointer(_Stdio_Sink *sink, int flags, int width, int precision, const void *ptr);
void   _Stdio_EmitFixed(_Stdio_Sink *sink, int flags, int width, int precision, long double value, _Stdio_Case letters);
void   _Stdio_EmitExponential(_Stdio_Sink *sink, int flags, int width, int precision, long double value, _Stdio_Case letters);
void   _Stdio_EmitGeneral(_Stdio_Sink *sink, int flags, int width, int precision, long double value, _Stdio_Case letters);
void   _Stdio_EmitHexReal(_Stdio_Sink *sink, int flags, int width, int precision, double value, _Stdio_Case letters);
void   _Stdio_EmitHexLongReal(_Stdio_Sink *sink, int flags, int width, int precision, long double value, _Stdio_Case letters);
void   _Stdio_EmitWideChar(_Stdio_Sink *sink, int flags, int width, _Stdio_WcharType wc);
void   _Stdio_EmitWideString(_Stdio_Sink *sink, int flags, int width, int precision, const _Stdio_WcharType *wstr);
int    _Stdio_EmitFormat(_Stdio_Sink *sink, const char *format, _Stdio_VaList args);

// Scan matching
size_t _Stdio_MatchScanHead(const char *format, _Stdio_Assign *assign, int *width, int *length);
_Bool  _Stdio_MatchScanSpace(const char *format, size_t *len);
_Bool  _Stdio_MatchScanOrdinary(const char *format, size_t *len);
_Bool  _Stdio_MatchScanPercent(const char *format, size_t *len);
_Bool  _Stdio_MatchScanChar(const char *format, _Stdio_VaList *list, size_t *len, int *width, char **dest);
_Bool  _Stdio_MatchScanString(const char *format, _Stdio_VaList *list, size_t *len, int *width, char **dest);
_Bool  _Stdio_MatchScanSet(const char *format, _Stdio_VaList *list, size_t *len, int *width, char **dest, const char **set, size_t *count, _Stdio_Accept *accept);
_Bool  _Stdio_MatchScanCount(const char *format, _Stdio_VaList *list, size_t *len, int *length, void **dest);
_Bool  _Stdio_MatchScanDecimal(const char *format, _Stdio_VaList *list, size_t *len, int *width, int *length, void **dest);
_Bool  _Stdio_MatchScanInteger(const char *format, _Stdio_VaList *list, size_t *len, int *width, int *length, void **dest);
_Bool  _Stdio_MatchScanOctal(const char *format, _Stdio_VaList *list, size_t *len, int *width, int *length, void **dest);
_Bool  _Stdio_MatchScanUnsigned(const char *format, _Stdio_VaList *list, size_t *len, int *width, int *length, void **dest);
_Bool  _Stdio_MatchScanHex(const char *format, _Stdio_VaList *list, size_t *len, int *width, int *length, void **dest);
_Bool  _Stdio_MatchScanPointer(const char *format, _Stdio_VaList *list, size_t *len, int *width, void **dest);
_Bool  _Stdio_MatchScanReal(const char *format, _Stdio_VaList *list, size_t *len, int *width, int *length, void **dest);
_Bool  _Stdio_MatchScanWideChar(const char *format, _Stdio_VaList *list, size_t *len, int *width, _Stdio_WcharType **dest);
_Bool  _Stdio_MatchScanWideString(const char *format, _Stdio_VaList *list, size_t *len, int *width, _Stdio_WcharType **dest);
_Bool  _Stdio_MatchScanWideSet(const char *format, _Stdio_VaList *list, size_t *len, int *width, _Stdio_WcharType **dest, const char **set, size_t *count, _Stdio_Accept *accept);

// Formatted input
int    _Stdio_PeekSource(_Stdio_Source *source);
void   _Stdio_AdvanceSource(_Stdio_Source *source);
int    _Stdio_EndSource(_Stdio_Source *source);
_Bool  _Stdio_IsInSet(const char *set, size_t count, int ch);
_Bool  _Stdio_ReadInteger(_Stdio_Source *source, int width, int base, char *item);
void   _Stdio_StoreInteger(void *dest, int length, unsigned long long value);
size_t _Stdio_TakeChars(_Stdio_Source *source, _Stdio_Item *item, const char *set, size_t max);
size_t _Stdio_TakeWord(_Stdio_Source *source, _Stdio_Item *item, const char *word);
void   _Stdio_ReadReal(_Stdio_Source *source, int width, _Stdio_Item *item);
_Bool  _Stdio_ConvertReal(const char *str, int length, void *dest);
void   _Stdio_DecodeByte(_Stdio_Source *source, _Stdio_Wide *wide, int ch);
_Bool  _Stdio_EndWide(_Stdio_Source *source, const _Stdio_Wide *wide);

// Scanning
void _Stdio_ScanSpace(_Stdio_Source *source);
void _Stdio_ScanOrdinary(_Stdio_Source *source, const char *text, size_t len);
void _Stdio_ScanPercent(_Stdio_Source *source);
void _Stdio_ScanChar(_Stdio_Source *source, int width, char *dest);
void _Stdio_ScanString(_Stdio_Source *source, int width, char *dest);
void _Stdio_ScanSet(_Stdio_Source *source, int width, char *dest, const char *set, size_t count, _Stdio_Accept accept);
void _Stdio_ScanCount(_Stdio_Source *source, int length, void *dest);
void _Stdio_ScanDecimal(_Stdio_Source *source, int width, int length, void *dest);
void _Stdio_ScanInteger(_Stdio_Source *source, int width, int length, void *dest);
void _Stdio_ScanOctal(_Stdio_Source *source, int width, int length, void *dest);
void _Stdio_ScanUnsigned(_Stdio_Source *source, int width, int length, void *dest);
void _Stdio_ScanHex(_Stdio_Source *source, int width, int length, void *dest);
void _Stdio_ScanPointer(_Stdio_Source *source, int width, void *dest);
void _Stdio_ScanReal(_Stdio_Source *source, int width, int length, void *dest);
void _Stdio_ScanWideChar(_Stdio_Source *source, int width, _Stdio_WcharType *dest);
void _Stdio_ScanWideString(_Stdio_Source *source, int width, _Stdio_WcharType *dest);
void _Stdio_ScanWideSet(_Stdio_Source *source, int width, _Stdio_WcharType *dest, const char *set, size_t count, _Stdio_Accept accept);
int  _Stdio_ScanFormat(_Stdio_Source *source, const char *format, _Stdio_VaList args);

// (S7.19.4) Operations on files
int   remove(const char *filename);
int   rename(const char *old, const char *new);
FILE *tmpfile(void);
char *tmpnam(char *str);

// (S7.19.5) File access functions
int   fclose(FILE *stream);
int   fflush(FILE *stream);
FILE *fopen(const char *restrict filename, const char *restrict mode);
FILE *freopen(const char *restrict filename, const char *restrict mode, FILE *restrict stream);
void  setbuf(FILE *restrict stream, char *restrict buf);
int   setvbuf(FILE *restrict stream, char *restrict buf, int mode, size_t size);

// (S7.19.6) Formatted input/output functions
int fprintf(FILE *restrict stream, const char *restrict format, ...);
int fscanf(FILE *restrict stream, const char *restrict format, ...);
int printf(const char *restrict format, ...);
int scanf(const char *restrict format, ...);
int snprintf(char *restrict str, size_t n, const char *restrict format, ...);
int sprintf(char *restrict str, const char *restrict format, ...);
int sscanf(const char *restrict str, const char *restrict format, ...);
int vfprintf(FILE *restrict stream, const char *restrict format, _Stdio_VaList arg);
int vfscanf(FILE *restrict stream, const char *restrict format, _Stdio_VaList arg);
int vprintf(const char *restrict format, _Stdio_VaList arg);
int vscanf(const char *restrict format, _Stdio_VaList arg);
int vsnprintf(char *restrict str, size_t n, const char *restrict format, _Stdio_VaList arg);
int vsprintf(char *restrict str, const char *restrict format, _Stdio_VaList arg);
int vsscanf(const char *restrict str, const char *restrict format, _Stdio_VaList arg);

// (S7.19.7) Character input/output functions
int   fgetc(FILE *stream);
char *fgets(char *restrict str, int n, FILE *restrict stream);
int   fputc(int ch, FILE *stream);
int   fputs(const char *restrict str, FILE *restrict stream);
int   getc(FILE *stream);
int   getchar(void);
char *gets(char *str);
int   putc(int ch, FILE *stream);
int   putchar(int ch);
int   puts(const char *str);
int   ungetc(int ch, FILE *stream);

// (S7.19.8) Direct input/output functions
size_t fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);
size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);

// (S7.19.9) File positioning functions
int  fgetpos(FILE *restrict stream, fpos_t *restrict pos);
int  fseek(FILE *stream, long offset, int whence);
int  fsetpos(FILE *stream, const fpos_t *pos);
long ftell(FILE *stream);
void rewind(FILE *stream);

// (S7.19.10) Error-handling functions
void clearerr(FILE *stream);
int  feof(FILE *stream);
int  ferror(FILE *stream);
void perror(const char *str);

#endif // _STDIO_H
