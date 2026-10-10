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
#define _STDIO_READ   0x1
#define _STDIO_WRITE  0x2
#define _STDIO_ERROR  0x4
#define _STDIO_OWNBUF 0x8
#define _STDIO_SETUP  0x10

// What a stream did last.
#define _STDIO_IDLE    0
#define _STDIO_WRITING 1

// The buffering a stream's first use picks: by line on a terminal, else full.
#define _STDIO_AUTO (-1)

// (S7.19.1) Introduction
#define NULL ((void *) 0)

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

#define BUFSIZ 8192

#define EOF (-1)

#define stderr (&_Stdio_Stderr)
#define stdin  (&_Stdio_Stdin)
#define stdout (&_Stdio_Stdout)

#ifndef __SIZE_T__
#define __SIZE_T__
typedef unsigned long size_t;
#endif

// A stream: its descriptor, its buffer, what it may do and the stream after it.
struct _Stdio_File {
    int sf_fd;
    int sf_flags;                // _STDIO_READ, _STDIO_WRITE and what happened
    int sf_state;                // _STDIO_IDLE or _STDIO_WRITING
    int sf_mode;                 // _IOFBF, _IOLBF, _IONBF or _STDIO_AUTO
    unsigned char *sf_buf;
    size_t sf_size;
    size_t sf_tail;              // the bytes that wait to be written
    unsigned char sf_byte;       // an unbuffered stream's buffer
    struct _Stdio_File *sf_next;
};

// (S7.19.1) Introduction
typedef struct _Stdio_File FILE;

// The standard streams.
extern FILE _Stdio_Stdin;
extern FILE _Stdio_Stdout;
extern FILE _Stdio_Stderr;

// The open streams, newest first.
extern FILE *_Stdio_Streams;

// Buffering
void  _Stdio_Setup(FILE *stream);
_Bool _Stdio_IsTerminal(int fd);

// Output
int    _Stdio_ToWrite(FILE *stream);
size_t _Stdio_Put(FILE *stream, const void *data, size_t len);
int    _Stdio_WriteAll(FILE *stream, const unsigned char *data, size_t len);
int    _Stdio_Flush(FILE *stream);
int    _Stdio_FlushAll(void);

// (S7.19.5) File access functions
int  fflush(FILE *stream);
void setbuf(FILE *restrict stream, char *restrict buf);
int  setvbuf(FILE *restrict stream, char *restrict buf, int mode, size_t size);

// (S7.19.7) Character input/output functions
int fputc(int ch, FILE *stream);
int fputs(const char *restrict str, FILE *restrict stream);
int putc(int ch, FILE *stream);
int putchar(int ch);
int puts(const char *str);

// (S7.19.8) Direct input/output functions
size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream);

#endif // _STDIO_H
