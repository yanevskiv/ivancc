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

// (S7.19.1) Introduction
#define NULL ((void *) 0)

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

#define BUFSIZ 8192

#define EOF (-1)

#define FOPEN_MAX 16

#define FILENAME_MAX 4096

#define L_tmpnam 20

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0

#define TMP_MAX 10000

#define stderr (&_Stdio_Stderr)
#define stdin  (&_Stdio_Stdin)
#define stdout (&_Stdio_Stdout)

// The argument list, which stdio.h may not name.
typedef __builtin_va_list _Stdio_VaList;

#ifndef __SIZE_T__
#define __SIZE_T__
typedef unsigned long size_t;
#endif

typedef long fpos_t;

// A stream: its descriptor, its buffer, what it may do and the stream after it.
typedef struct _Stdio_File _Stdio_File;
struct _Stdio_File {
    int sf_fd;
    int sf_flags;                // _STDIO_READ, _STDIO_WRITE and what happened
    int sf_state;                // _STDIO_IDLE, _STDIO_READING, _STDIO_WRITING
    int sf_mode;                 // _IOFBF, _IOLBF, _IONBF or _STDIO_AUTO
    unsigned char *sf_buf;
    size_t sf_size;
    size_t sf_head;              // the bytes of the buffer already read
    size_t sf_tail;              // the bytes the buffer holds, read or to write
    unsigned char sf_byte;       // an unbuffered stream's buffer
    unsigned char sf_back;       // the character ungetc pushed back
    _Stdio_File *sf_next;
};

// Where formatted output goes: a stream, or else a string of a size.
typedef struct _Stdio_Sink _Stdio_Sink;
struct _Stdio_Sink {
    _Stdio_File *ss_stream;
    char *ss_str;
    size_t ss_size;              // the string's size, its null counted
    size_t ss_count;             // the characters given so far, kept or not
    _Bool ss_failed;
};

// A conversion specification: its flags, width, precision, length and letter.
typedef struct _Stdio_Directive _Stdio_Directive;
struct _Stdio_Directive {
    int sd_flags;                // _STDIO_FLAG_LEFT and the rest
    int sd_width;
    int sd_precision;            // _STDIO_PRECISION_NONE if it gives none
    int sd_length;               // _STDIO_LENGTH_NONE and the rest
    char sd_conv;
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

// Formatted output
void _Stdio_Emit(_Stdio_Sink *sink, const char *data, size_t len);
void _Stdio_Pad(_Stdio_Sink *sink, int ch, size_t len);
void _Stdio_Field(_Stdio_Sink *sink, const _Stdio_Directive *dir, const char *data, size_t len);
void _Stdio_String(_Stdio_Sink *sink, const _Stdio_Directive *dir, const char *str);
int  _Stdio_Format(_Stdio_Sink *sink, const char *format, _Stdio_VaList args);
int  _Stdio_End(_Stdio_Sink *sink);

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
