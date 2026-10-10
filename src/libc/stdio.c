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

// The error a failed open, read, write or close reports.
#include <errno.h>

// Copying to and from a buffer, finding its new-lines and matching a mode.
#include <string.h>

// The streams fopen allocates and the buffers they do.
#include <stdlib.h>

// The hook exit flushes the streams by.
#include <_crt.h>

// The largest int puts returns and the largest size fread and fwrite move.
#include <limits.h>
#include <stdint.h>

// The files' calls, the terminal's settings and the standard descriptors.
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

// Make a stream with no file, first among the open ones.
FILE *_Stdio_New(void)
{
    FILE *stream = calloc(1, sizeof(FILE));

    if (stream != NULL) {
        stream->sf_fd = _STDIO_NOFILE;
        stream->sf_flags = _STDIO_OWNFILE;
        stream->sf_mode = _STDIO_AUTO;
        stream->sf_next = _Stdio_Streams;
        _Stdio_Streams = stream;
    }
    return stream;
}

// Return whether mode is one of the fifteen fopen takes.
_Bool _Stdio_IsMode(const char *mode)
{
    const char *rest = mode + 1;

    if (mode[0] != 'r' && mode[0] != 'w' && mode[0] != 'a') {
        return 0;
    }
    return rest[0] == '\0' || strcmp(rest, "b") == 0 || strcmp(rest, "+") == 0 || strcmp(rest, "b+") == 0 || strcmp(rest, "+b") == 0;
}

// Turn fopen's mode into the flags open takes.
int _Stdio_OpenFlags(const char *mode)
{
    int flags = mode[0] == 'r' ? _SYS_O_RDONLY : _SYS_O_WRONLY | _SYS_O_CREAT;

    if (strchr(mode, '+') != NULL) {
        flags = (flags & ~_SYS_O_ACCMODE) | _SYS_O_RDWR;
    }
    if (mode[0] == 'w') {
        flags |= _SYS_O_TRUNC;
    }
    if (mode[0] == 'a') {
        flags |= _SYS_O_APPEND;
    }
    return flags;
}

// Open the file filename for the stream, to read or write as the mode says.
int _Stdio_Open(FILE *stream, const char *filename, const char *mode)
{
    long fd;
    int flags;

    if (! _Stdio_IsMode(mode)) {
        errno = _SYS_EINVAL;
        return EOF;
    }
    flags = _Stdio_OpenFlags(mode);
    fd = _Sys_Open(filename, flags, _SYS_DEFFILEMODE);
    if (fd < 0) {
        errno = (int) -fd;
        return EOF;
    }
    stream->sf_fd = (int) fd;
    if ((flags & _SYS_O_ACCMODE) != _SYS_O_WRONLY) {
        stream->sf_flags |= _STDIO_READ;
    }
    if ((flags & _SYS_O_ACCMODE) != _SYS_O_RDONLY) {
        stream->sf_flags |= _STDIO_WRITE;
    }
    return 0;
}

// Flush the stream and take its buffer, indicators and access from it.
int _Stdio_Reset(FILE *stream)
{
    int result = _Stdio_Flush(stream);

    if (stream->sf_flags & _STDIO_OWNBUF) {
        free(stream->sf_buf);
    }
    stream->sf_flags &= _STDIO_OWNFILE;
    stream->sf_state = _STDIO_IDLE;
    stream->sf_mode = _STDIO_AUTO;
    stream->sf_buf = NULL;
    stream->sf_size = 0;
    stream->sf_head = 0;
    stream->sf_tail = 0;
    return result;
}

// Take the stream off the open ones and free one fopen made.
void _Stdio_Free(FILE *stream)
{
    FILE **iter = &_Stdio_Streams;

    while (*iter != NULL && *iter != stream) {
        iter = &(*iter)->sf_next;
    }
    if (*iter == stream) {
        *iter = stream->sf_next;
    }
    stream->sf_fd = _STDIO_NOFILE;
    stream->sf_next = NULL;
    if (stream->sf_flags & _STDIO_OWNFILE) {
        free(stream);
    }
}

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

// Ready the stream for input, or set its error indicator if it is write-only.
int _Stdio_ToRead(FILE *stream)
{
    if (! (stream->sf_flags & _STDIO_READ)) {
        stream->sf_flags |= _STDIO_ERROR;
        errno = _SYS_EBADF;
        return EOF;
    }
    if (_Stdio_Flush(stream) != 0) {
        return EOF;
    }
    _Stdio_Setup(stream);
    stream->sf_state = _STDIO_READING;
    return 0;
}

// Take up to len bytes from the stream into data, and return how many came.
size_t _Stdio_Get(FILE *stream, void *data, size_t len)
{
    size_t done = 0;
    unsigned char *bytes = data;

    if (_Stdio_ToRead(stream) != 0) {
        return 0;
    }
    if (len > 0 && (stream->sf_flags & _STDIO_BACK)) {
        stream->sf_flags &= ~_STDIO_BACK;
        bytes[done++] = stream->sf_back;
    }
    while (done < len) {
        size_t part = len - done;

        if (stream->sf_head == stream->sf_tail && part >= stream->sf_size) {
            part = _Stdio_Read(stream, bytes + done, part);
        } else {
            part = _Stdio_Take(stream, bytes + done, part);
        }
        if (part == 0) {
            return done;
        }
        done += part;
    }
    return done;
}

// Copy up to len bytes of the stream's buffer into data, refilling it if empty.
size_t _Stdio_Take(FILE *stream, unsigned char *data, size_t len)
{
    if (stream->sf_head == stream->sf_tail) {
        stream->sf_head = 0;
        stream->sf_tail = _Stdio_Read(stream, stream->sf_buf, stream->sf_size);
    }
    if (len > stream->sf_tail - stream->sf_head) {
        len = stream->sf_tail - stream->sf_head;
    }
    memcpy(data, stream->sf_buf + stream->sf_head, len);
    stream->sf_head += len;
    return len;
}

// Read up to len bytes from the stream's descriptor, none once at its end.
size_t _Stdio_Read(FILE *stream, unsigned char *data, size_t len)
{
    long done;

    if (stream->sf_flags & _STDIO_EOF) {
        return 0;
    }
    if (stream->sf_mode != _IOFBF) {
        _Stdio_FlushLines();
    }
    done = _Sys_Read(stream->sf_fd, data, len);
    if (done < 0) {
        errno = (int) -done;
        stream->sf_flags |= _STDIO_ERROR;
        return 0;
    }
    if (done == 0) {
        stream->sf_flags |= _STDIO_EOF;
    }
    return (size_t) done;
}

// Read a line of at most max characters, its new-line among them, into str.
char *_Stdio_GetLine(FILE *stream, char *str, size_t max)
{
    int ch = 0;
    size_t len = 0;

    while (len < max && ch != '\n' && (ch = fgetc(stream)) != EOF) {
        str[len++] = (char) ch;
    }
    if (ch == EOF && (len == 0 || ! (stream->sf_flags & _STDIO_EOF))) {
        return NULL;
    }
    str[len] = '\0';
    return str;
}

// Move the stream's file back over what it read ahead, and empty its buffer.
void _Stdio_Unread(FILE *stream)
{
    long ahead = (long) (stream->sf_tail - stream->sf_head);

    if (stream->sf_flags & _STDIO_BACK) {
        ahead++;
    }
    if (ahead > 0) {
        _Sys_Lseek(stream->sf_fd, -ahead, _SYS_SEEK_CUR);
    }
    stream->sf_flags &= ~_STDIO_BACK;
    stream->sf_head = 0;
    stream->sf_tail = 0;
}

// Ready the stream for output, or set its error indicator if it is read-only.
int _Stdio_ToWrite(FILE *stream)
{
    if (! (stream->sf_flags & _STDIO_WRITE)) {
        stream->sf_flags |= _STDIO_ERROR;
        errno = _SYS_EBADF;
        return EOF;
    }
    if (stream->sf_state == _STDIO_READING) {
        _Stdio_Unread(stream);
    }
    _Stdio_Setup(stream);
    stream->sf_state = _STDIO_WRITING;
    return 0;
}

// Buffer len bytes of data for the stream, and return how many it took.
size_t _Stdio_Put(FILE *stream, const void *data, size_t len)
{
    size_t done = 0;
    const unsigned char *bytes = data;

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

// Flush every line-buffered stream, as a read from the host calls for.
void _Stdio_FlushLines(void)
{
    FILE *stream;

    for (stream = _Stdio_Streams; stream != NULL; stream = stream->sf_next) {
        if (stream->sf_mode == _IOLBF) {
            _Stdio_Flush(stream);
        }
    }
}

// Flush the stream, close its file and end the stream.
int fclose(FILE *stream)
{
    int result = _Stdio_Reset(stream);
    long status = _Sys_Close(stream->sf_fd);

    if (status < 0) {
        errno = (int) -status;
        result = EOF;
    }
    _Stdio_Free(stream);
    return result;
}

// Write what waits in the stream's buffer, or in every stream's for NULL.
int fflush(FILE *stream)
{
    return stream == NULL ? _Stdio_FlushAll() : _Stdio_Flush(stream);
}

// Open the file filename as the mode says, and return a stream for it.
FILE *fopen(const char *restrict filename, const char *restrict mode)
{
    FILE *stream = _Stdio_New();

    if (stream != NULL && _Stdio_Open(stream, filename, mode) != 0) {
        _Stdio_Free(stream);
        return NULL;
    }
    return stream;
}

// Open the file filename as the mode says for the stream, on its descriptor.
FILE *freopen(const char *restrict filename, const char *restrict mode, FILE *restrict stream)
{
    int fd = stream->sf_fd;

    if (filename == NULL) {
        errno = _SYS_EINVAL;
        return NULL;
    }
    _Stdio_Reset(stream);
    if (_Stdio_Open(stream, filename, mode) != 0) {
        _Sys_Close(fd);
        _Stdio_Free(stream);
        return NULL;
    }
    if (_Sys_Dup2(stream->sf_fd, fd) == fd) {
        _Sys_Close(stream->sf_fd);
        stream->sf_fd = fd;
    } else {
        _Sys_Close(fd);
    }
    return stream;
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
    stream->sf_head = 0;
    stream->sf_tail = 0;
    stream->sf_state = _STDIO_IDLE;
    return 0;
}

// Read the stream's next character, as an unsigned char, or EOF at its end.
int fgetc(FILE *stream)
{
    unsigned char byte;

    return _Stdio_Get(stream, &byte, 1) == 1 ? byte : EOF;
}

// Read a line of at most n - 1 characters, its new-line kept, into str.
char *fgets(char *restrict str, int n, FILE *restrict stream)
{
    return n > 0 ? _Stdio_GetLine(stream, str, (size_t) n - 1) : NULL;
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

// Read the stream's next character, as fgetc does.
int getc(FILE *stream)
{
    return fgetc(stream);
}

// Read standard input's next character.
int getchar(void)
{
    return fgetc(stdin);
}

// Read a line of standard input, however long, into str, its new-line dropped.
char *gets(char *str)
{
    size_t len;

    if (_Stdio_GetLine(stdin, str, SIZE_MAX) == NULL) {
        return NULL;
    }
    len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
    return str;
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

// Push the character ch back for the stream's next read, one at a time.
int ungetc(int ch, FILE *stream)
{
    if (ch == EOF || (stream->sf_flags & _STDIO_BACK) || _Stdio_ToRead(stream) != 0) {
        return EOF;
    }
    stream->sf_back = (unsigned char) ch;
    stream->sf_flags |= _STDIO_BACK;
    stream->sf_flags &= ~_STDIO_EOF;
    return stream->sf_back;
}

// Read nmemb elements of size bytes into ptr, and return how many came whole.
size_t fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream)
{
    if (size == 0 || nmemb == 0) {
        return 0;
    }
    if (nmemb > SIZE_MAX / size) {
        nmemb = SIZE_MAX / size;
    }
    return _Stdio_Get(stream, ptr, size * nmemb) / size;
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

// Clear the stream's end-of-file and error indicators.
void clearerr(FILE *stream)
{
    stream->sf_flags &= ~(_STDIO_EOF | _STDIO_ERROR);
}

// Return nonzero if the stream's end-of-file indicator is set.
int feof(FILE *stream)
{
    return (stream->sf_flags & _STDIO_EOF) != 0;
}

// Return nonzero if the stream's error indicator is set.
int ferror(FILE *stream)
{
    return (stream->sf_flags & _STDIO_ERROR) != 0;
}
