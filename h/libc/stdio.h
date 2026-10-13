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

#ifndef __SIZE_T__
#define __SIZE_T__
typedef unsigned long size_t;
#endif

// (S7.19.1) Introduction
typedef _File_Stream FILE;

typedef long fpos_t;

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
