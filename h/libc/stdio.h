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

// What a stream may do, and what has happened to it.
#define _STDIO_READ    0x1
#define _STDIO_WRITE   0x2
#define _STDIO_ERROR   0x4
#define _STDIO_OWNBUF  0x8
#define _STDIO_SETUP   0x10
#define _STDIO_EOF     0x20
#define _STDIO_BACK    0x40
#define _STDIO_OWNFILE 0x80

// What a stream did last.
#define _STDIO_IDLE    0
#define _STDIO_READING 1
#define _STDIO_WRITING 2

// The buffering a stream's first use picks: by line on a terminal, else full.
#define _STDIO_AUTO (-1)

// The descriptor of a stream with no file.
#define _STDIO_NOFILE (-1)

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
#define stderr (&_Stdio_Stderr)
#define stdin  (&_Stdio_Stdin)
#define stdout (&_Stdio_Stdout)

// The argument list, which stdio.h may not name.
typedef __builtin_va_list _Stdio_VaList;

// The widest integers, which stdio.h may not name.
typedef long          _Stdio_IntmaxType;
typedef unsigned long _Stdio_UintmaxType;

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

// A stream: its descriptor, its buffer, what it may do and the stream after it.
typedef struct _Stdio_File _Stdio_File;
struct _Stdio_File {
    int sf_fd;             // the file's descriptor, or _STDIO_NOFILE
    int sf_flags;          // _STDIO_READ, _STDIO_WRITE and what happened
    int sf_state;          // _STDIO_IDLE, _STDIO_READING, _STDIO_WRITING
    int sf_mode;           // _IOFBF, _IOLBF, _IONBF or _STDIO_AUTO
    unsigned char *sf_buf; // the buffer, the stream's own or setvbuf's
    size_t sf_size;        // the buffer's size
    size_t sf_head;        // the bytes of the buffer already read
    size_t sf_tail;        // the bytes the buffer holds, read or to write
    unsigned char sf_byte; // an unbuffered stream's buffer
    unsigned char sf_back; // the character ungetc pushed back
    _Stdio_File *sf_next;  // the next of the open streams
};

// Where formatted output goes: a stream, or else a string of a size.
typedef struct _Stdio_Sink _Stdio_Sink;
struct _Stdio_Sink {
    _Stdio_File *ss_stream; // the stream, or NULL for a string
    char *ss_str;           // the string, where there is no stream
    size_t ss_size;         // the string's size, its null counted
    size_t ss_count;        // the characters given so far, kept or not
    _Bool ss_failed;        // a write failed, or the count overflowed
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

// (S7.19.1) Introduction
typedef struct _Stdio_File FILE;

// The standard streams.
extern FILE _Stdio_Stdin;
extern FILE _Stdio_Stdout;
extern FILE _Stdio_Stderr;

// The open streams, newest first.
extern FILE *_Stdio_Streams;

// The number of the temporary name made last, once a seed has begun them.
extern unsigned long _Stdio_TempNumber;
extern _Bool _Stdio_TempSeeded;

// The name tmpnam returns when it is given no array.
extern char _Stdio_TempBuffer[L_tmpnam];

// Opening
FILE *_Stdio_New(void);
_Bool _Stdio_IsMode(const char *mode);
int   _Stdio_OpenFlags(const char *mode);
void  _Stdio_Attach(FILE *stream, int fd, int flags);
int   _Stdio_Open(FILE *stream, const char *filename, const char *mode);
int   _Stdio_Reset(FILE *stream);
void  _Stdio_Free(FILE *stream);

// Buffering
void  _Stdio_Setup(FILE *stream);
_Bool _Stdio_IsTerminal(int fd);
int   _Stdio_Lend(FILE *stream, unsigned char *buf, size_t size);
int   _Stdio_Reclaim(FILE *stream);

// Input
int    _Stdio_ToRead(FILE *stream);
size_t _Stdio_Get(FILE *stream, void *data, size_t len);
size_t _Stdio_Take(FILE *stream, unsigned char *data, size_t len);
size_t _Stdio_Read(FILE *stream, unsigned char *data, size_t len);
char  *_Stdio_GetLine(FILE *stream, char *str, size_t max);

// Output
int    _Stdio_ToWrite(FILE *stream);
size_t _Stdio_Put(FILE *stream, const void *data, size_t len);
int    _Stdio_WriteAll(FILE *stream, const unsigned char *data, size_t len);
int    _Stdio_Flush(FILE *stream);
int    _Stdio_FlushAll(void);
void   _Stdio_FlushLines(void);

// Parsing
size_t             _Stdio_ParseHead(const char *format, int *flags, int *width, int *precision, int *length);
void               _Stdio_TakeHead(_Stdio_VaList *list, int *flags, int *width, int *precision);
_Stdio_UintmaxType _Stdio_TakeUnsigned(_Stdio_VaList *list, int length);
_Bool              _Stdio_ParseText(const char *format, size_t *len);
_Bool              _Stdio_ParsePercent(const char *format, _Stdio_VaList *list, size_t *len);
_Bool              _Stdio_ParseChar(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, char *ch);
_Bool              _Stdio_ParseString(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, const char **str);
_Bool              _Stdio_ParseDecimal(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_IntmaxType *value);
_Bool              _Stdio_ParseUnsigned(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_UintmaxType *value);
_Bool              _Stdio_ParseOctal(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_UintmaxType *value);
_Bool              _Stdio_ParseHex(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_UintmaxType *value, _Stdio_Case *letters);
_Bool              _Stdio_ParsePointer(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, const void **ptr);
_Bool              _Stdio_ParseCount(const char *format, _Stdio_VaList *list, size_t *len, int *length, void **dest);
_Bool              _Stdio_ParseFixed(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Stdio_Case *letters);
_Bool              _Stdio_ParseExponential(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Stdio_Case *letters);
_Bool              _Stdio_ParseGeneral(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Stdio_Case *letters);
_Bool              _Stdio_ParseHexReal(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, double *value, _Stdio_Case *letters);
_Bool              _Stdio_ParseHexLongReal(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Stdio_Case *letters);

// Formatted output
size_t      _Stdio_Digits(char *end, _Stdio_UintmaxType value, int base, const char *digits);
void        _Stdio_StoreCount(void *dest, int length, size_t count);
_Stdio_Real _Stdio_Unpack(long double value, _Stdio_Sign *sign, unsigned long long *mant, int *exp);
void        _Stdio_Exact(long double value, _Stdio_Numeral *num);
void        _Stdio_Round(_Stdio_Numeral *num, long keep);
int         _Stdio_End(_Stdio_Sink *sink);

// Emitting
void   _Stdio_Emit(_Stdio_Sink *sink, const char *data, size_t len);
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
int    _Stdio_EmitFormat(_Stdio_Sink *sink, const char *format, _Stdio_VaList args);

// Positioning
long _Stdio_Ahead(FILE *stream);
int  _Stdio_Seek(FILE *stream, long offset, int whence);
void _Stdio_Unread(FILE *stream);

// Temporary names
unsigned long _Stdio_TempSeed(void);
void          _Stdio_TempName(char *str);

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
int printf(const char *restrict format, ...);
int snprintf(char *restrict str, size_t n, const char *restrict format, ...);
int sprintf(char *restrict str, const char *restrict format, ...);
int vfprintf(FILE *restrict stream, const char *restrict format, _Stdio_VaList arg);
int vprintf(const char *restrict format, _Stdio_VaList arg);
int vsnprintf(char *restrict str, size_t n, const char *restrict format, _Stdio_VaList arg);
int vsprintf(char *restrict str, const char *restrict format, _Stdio_VaList arg);

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
