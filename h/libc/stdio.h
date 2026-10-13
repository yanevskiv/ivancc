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

// The streams, their buffers and the files under them.
#include <_file.h>

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

// A wide character, which stdio.h may not name.
typedef int _Stdio_WcharType;

#ifndef __SIZE_T__
#define __SIZE_T__
typedef unsigned long size_t;
#endif

// (S7.19.1) Introduction
typedef _File_Stream FILE;

typedef long fpos_t;

// What an input directive does with what it converts.
typedef enum _Stdio_Assign _Stdio_Assign;
enum _Stdio_Assign {
    _STDIO_ASSIGN_STORE,   // store it at the next argument
    _STDIO_ASSIGN_SUPPRESS // store nothing, the directive having an asterisk
};

// The characters a scanset takes: those it lists, or all the others.
typedef enum _Stdio_Accept _Stdio_Accept;
enum _Stdio_Accept {
    _STDIO_ACCEPT_LISTED,  // the characters between its brackets
    _STDIO_ACCEPT_UNLISTED // every other, the scanset beginning with ^
};

// What stopped formatted input: nothing, no input or input that does not fit.
typedef enum _Stdio_Failure _Stdio_Failure;
enum _Stdio_Failure {
    _STDIO_FAILURE_NONE,    // nothing has
    _STDIO_FAILURE_INPUT,   // the input ended, could not be read or decoded
    _STDIO_FAILURE_MATCHING // the input is not what a directive takes
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
