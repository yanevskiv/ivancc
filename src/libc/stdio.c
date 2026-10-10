/*
 * C source file for input/output.
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

// Module header.
#include <stdio.h>

// The error a failed write reports.
#include <errno.h>

// Copying into a buffer, finding its new-lines and measuring a string.
#include <string.h>

// The buffers a stream allocates.
#include <stdlib.h>

// The hook exit flushes the streams by.
#include <_crt.h>

// The largest int puts returns and the largest size fwrite writes.
#include <limits.h>
#include <stdint.h>

// The writes, the terminal's settings and the standard descriptors.
#include <_sys.h>

// Standard input, read-only, its buffering picked at its first use.
FILE _Stdio_Stdin = {
    .sf_fd    = _SYS_STDIN_FILENO,
    .sf_flags = _STDIO_READ,
    .sf_mode  = _STDIO_AUTO,
    .sf_next  = &_Stdio_Stdout
};

// Standard output, write-only, its buffering picked at its first use.
FILE _Stdio_Stdout = {
    .sf_fd    = _SYS_STDOUT_FILENO,
    .sf_flags = _STDIO_WRITE,
    .sf_mode  = _STDIO_AUTO,
    .sf_next  = &_Stdio_Stderr
};

// Standard error, write-only and unbuffered as glibc's, not fully (S7.19.3p7).
FILE _Stdio_Stderr = {
    .sf_fd    = _SYS_STDERR_FILENO,
    .sf_flags = _STDIO_WRITE,
    .sf_mode  = _IONBF
};

// The open streams, the standard ones at first.
FILE *_Stdio_Streams = &_Stdio_Stdin;

// Pick the stream's buffering and buffer at its first use, none without memory.
void _Stdio_Setup(FILE *stream)
{
    size_t size = stream->sf_size != 0 ? stream->sf_size : BUFSIZ;

    if (stream->sf_flags & _STDIO_SETUP) {
        return;
    }
    if (stream->sf_mode == _STDIO_AUTO) {
        stream->sf_mode = _Stdio_IsTerminal(stream->sf_fd) ? _IOLBF : _IOFBF;
    }
    if (stream->sf_mode != _IONBF && stream->sf_buf == NULL) {
        stream->sf_buf = malloc(size);
        stream->sf_size = size;
        stream->sf_flags |= _STDIO_OWNBUF;
        if (stream->sf_buf == NULL) {
            stream->sf_mode = _IONBF;
            stream->sf_flags &= ~_STDIO_OWNBUF;
        }
    }
    if (stream->sf_mode == _IONBF) {
        stream->sf_buf = &stream->sf_byte;
        stream->sf_size = 1;
    }
    stream->sf_flags |= _STDIO_SETUP;
}

// Return whether the descriptor fd is a terminal, which answers TCGETS.
_Bool _Stdio_IsTerminal(int fd)
{
    struct _Sys_Termios term;

    return _Sys_Ioctl(fd, _SYS_TCGETS, &term) == 0;
}

// Ready the stream for output, or set its error indicator if it is read-only.
int _Stdio_ToWrite(FILE *stream)
{
    if (! (stream->sf_flags & _STDIO_WRITE)) {
        stream->sf_flags |= _STDIO_ERROR;
        errno = _SYS_EBADF;
        return EOF;
    }
    _Stdio_Setup(stream);
    stream->sf_state = _STDIO_WRITING;
    return 0;
}

// Buffer len bytes of data for the stream, and return how many it took.
size_t _Stdio_Put(FILE *stream, const void *data, size_t len)
{
    const unsigned char *bytes = data;
    size_t done = 0;

    if (_Stdio_ToWrite(stream) != 0) {
        return 0;
    }
    _Crt_Flush = _Stdio_FlushAll;
    while (done < len) {
        size_t part = len - done;

        if (stream->sf_tail == 0 && part >= stream->sf_size) {
            return _Stdio_WriteAll(stream, bytes + done, part) == 0 ? len : done;
        }
        if (part > stream->sf_size - stream->sf_tail) {
            part = stream->sf_size - stream->sf_tail;
        }
        memcpy(stream->sf_buf + stream->sf_tail, bytes + done, part);
        stream->sf_tail += part;
        done += part;
        if (stream->sf_tail == stream->sf_size && _Stdio_Flush(stream) != 0) {
            return done - part;
        }
    }
    if (stream->sf_mode == _IOLBF && memchr(data, '\n', len) != NULL && _Stdio_Flush(stream) != 0) {
        return 0;
    }
    return len;
}

// Write len bytes of data to the stream's descriptor, however many writes.
int _Stdio_WriteAll(FILE *stream, const unsigned char *data, size_t len)
{
    while (len > 0) {
        long done = _Sys_Write(stream->sf_fd, data, len);

        if (done <= 0) {
            errno = done < 0 ? (int) -done : _SYS_EIO;
            stream->sf_flags |= _STDIO_ERROR;
            return EOF;
        }
        data += done;
        len -= (size_t) done;
    }
    return 0;
}

// Write what waits in the stream's buffer, dropping it if the write fails.
int _Stdio_Flush(FILE *stream)
{
    size_t len = stream->sf_tail;

    if (stream->sf_state != _STDIO_WRITING || len == 0) {
        return 0;
    }
    stream->sf_tail = 0;
    return _Stdio_WriteAll(stream, stream->sf_buf, len);
}

// Flush every open stream, and return EOF if any failed.
int _Stdio_FlushAll(void)
{
    FILE *stream;
    int result = 0;

    for (stream = _Stdio_Streams; stream != NULL; stream = stream->sf_next) {
        if (_Stdio_Flush(stream) != 0) {
            result = EOF;
        }
    }
    return result;
}

// Write what waits in the stream's buffer, or in every stream's for NULL.
int fflush(FILE *stream)
{
    return stream == NULL ? _Stdio_FlushAll() : _Stdio_Flush(stream);
}

// Give the stream the buffer buf of BUFSIZ bytes, or none for NULL.
void setbuf(FILE *restrict stream, char *restrict buf)
{
    setvbuf(stream, buf, buf != NULL ? _IOFBF : _IONBF, BUFSIZ);
}

// Give the stream the buffering mode, and buf's size bytes or its own for NULL.
int setvbuf(FILE *restrict stream, char *restrict buf, int mode, size_t size)
{
    if (mode != _IOFBF && mode != _IOLBF && mode != _IONBF) {
        return EOF;
    }
    if ((buf != NULL && size == 0) || _Stdio_Flush(stream) != 0) {
        return EOF;
    }
    if (stream->sf_flags & _STDIO_OWNBUF) {
        free(stream->sf_buf);
    }
    stream->sf_flags &= ~(_STDIO_OWNBUF | _STDIO_SETUP);
    stream->sf_mode = mode;
    stream->sf_buf = mode != _IONBF ? (unsigned char *) buf : NULL;
    stream->sf_size = size;
    stream->sf_tail = 0;
    stream->sf_state = _STDIO_IDLE;
    return 0;
}

// Write the character ch, as an unsigned char, to the stream.
int fputc(int ch, FILE *stream)
{
    unsigned char byte = (unsigned char) ch;

    return _Stdio_Put(stream, &byte, 1) == 1 ? byte : EOF;
}

// Write the string str to the stream, and return 1 as glibc's does.
int fputs(const char *restrict str, FILE *restrict stream)
{
    size_t len = strlen(str);

    return _Stdio_Put(stream, str, len) == len ? 1 : EOF;
}

// Write the character ch to the stream, as fputc does.
int putc(int ch, FILE *stream)
{
    return fputc(ch, stream);
}

// Write the character ch to standard output.
int putchar(int ch)
{
    return fputc(ch, stdout);
}

// Write the string str and a new-line to standard output, as glibc's counts.
int puts(const char *str)
{
    size_t len = strlen(str);

    if (_Stdio_Put(stdout, str, len) != len || _Stdio_Put(stdout, "\n", 1) != 1) {
        return EOF;
    }
    return len < INT_MAX ? (int) len + 1 : INT_MAX;
}

// Write nmemb elements of size bytes from ptr, and return how many went.
size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream)
{
    if (size == 0 || nmemb == 0) {
        return 0;
    }
    if (nmemb > SIZE_MAX / size) {
        nmemb = SIZE_MAX / size;
    }
    return _Stdio_Put(stream, ptr, size * nmemb) / size;
}
