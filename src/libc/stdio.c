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

// The arguments a format converts.
#include <stdarg.h>

// The text a null string is written as.
#include <_err.h>

// A temporary name: where it is, and the characters and length of its end.
#define _STDIO_TEMP_PREFIX  "/tmp/tmp."
#define _STDIO_TEMP_LETTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
#define _STDIO_TEMP_RADIX   62
#define _STDIO_TEMP_DIGITS  8

// How many temporary names there are, and a step that shares no factor with it.
#define _STDIO_TEMP_NAMES 218340105584896UL
#define _STDIO_TEMP_STEP  134941606358707UL

// Check the header's origins against lseek's.
typedef char _Stdio_CheckSeek[SEEK_SET == _SYS_SEEK_SET && SEEK_CUR == _SYS_SEEK_CUR && SEEK_END == _SYS_SEEK_END ? 1 : -1];

// Check that a temporary name's characters are its radix and it fits L_tmpnam.
typedef char _Stdio_CheckTempRadix[sizeof(_STDIO_TEMP_LETTERS) - 1 == _STDIO_TEMP_RADIX ? 1 : -1];
typedef char _Stdio_CheckTempName[sizeof(_STDIO_TEMP_PREFIX) + _STDIO_TEMP_DIGITS <= L_tmpnam ? 1 : -1];

// Check the header's own argument list against stdarg.h's.
typedef char _Stdio_CheckVaList[sizeof(_Stdio_VaList) == sizeof(va_list) ? 1 : -1];

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

// The number of the temporary name made last, once a seed has begun them.
unsigned long _Stdio_TempNumber;
_Bool _Stdio_TempSeeded;

// The name tmpnam returns when it is given no array.
char _Stdio_TempBuffer[L_tmpnam];

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

// Give the stream the descriptor fd, to read or write as its open flags say.
void _Stdio_Attach(FILE *stream, int fd, int flags)
{
    stream->sf_fd = fd;
    if ((flags & _SYS_O_ACCMODE) != _SYS_O_WRONLY) {
        stream->sf_flags |= _STDIO_READ;
    }
    if ((flags & _SYS_O_ACCMODE) != _SYS_O_RDONLY) {
        stream->sf_flags |= _STDIO_WRITE;
    }
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
    _Stdio_Attach(stream, (int) fd, flags);
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
    _Sys_Termios term;

    return _Sys_Ioctl(fd, _SYS_TCGETS, &term) == 0;
}

// Ready the stream for output, and lend an unbuffered one buf for a call.
int _Stdio_Lend(FILE *stream, unsigned char *buf, size_t size)
{
    if (_Stdio_ToWrite(stream) != 0) {
        return EOF;
    }
    if (stream->sf_mode == _IONBF) {
        stream->sf_buf = buf;
        stream->sf_size = size;
    }
    return 0;
}

// Write what a lent buffer holds, and give the stream its own byte back.
int _Stdio_Reclaim(FILE *stream)
{
    int result = 0;

    if (stream->sf_mode == _IONBF && stream->sf_state == _STDIO_WRITING) {
        result = _Stdio_Flush(stream);
        stream->sf_buf = &stream->sf_byte;
        stream->sf_size = 1;
    }
    return result;
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
        stream->sf_flags &= ~_STDIO_BACK;
        stream->sf_head = 0;
        stream->sf_tail = 0;
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

// Give the sink len characters of data, counted though its string be full.
void _Stdio_Emit(_Stdio_Sink *sink, const char *data, size_t len)
{
    size_t room;

    if (sink->ss_failed) {
        return;
    }
    if (len > (size_t) INT_MAX - sink->ss_count) {
        errno = _SYS_EOVERFLOW;
        sink->ss_failed = 1;
        return;
    }
    if (sink->ss_stream != NULL) {
        sink->ss_failed = _Stdio_Put(sink->ss_stream, data, len) != len;
    } else if (sink->ss_count + 1 < sink->ss_size) {
        room = sink->ss_size - 1 - sink->ss_count;
        memcpy(sink->ss_str + sink->ss_count, data, len < room ? len : room);
    }
    sink->ss_count += len;
}

// Give the sink len copies of the character ch.
void _Stdio_Pad(_Stdio_Sink *sink, int ch, size_t len)
{
    char chunk[_STDIO_PAD_CHUNK];

    memset(chunk, ch, sizeof(chunk));
    while (len > 0 && ! sink->ss_failed) {
        size_t part = len < sizeof(chunk) ? len : sizeof(chunk);

        _Stdio_Emit(sink, chunk, part);
        len -= part;
    }
}

// Give the sink len characters of data in the directive's width, on its side.
void _Stdio_Field(_Stdio_Sink *sink, const _Stdio_Directive *dir, const char *data, size_t len)
{
    size_t width = (size_t) dir->sd_width;
    size_t pad = width > len ? width - len : 0;

    if (! (dir->sd_flags & _STDIO_FLAG_LEFT)) {
        _Stdio_Pad(sink, ' ', pad);
    }
    _Stdio_Emit(sink, data, len);
    if (dir->sd_flags & _STDIO_FLAG_LEFT) {
        _Stdio_Pad(sink, ' ', pad);
    }
}

// Convert the string str by the directive, a null one as glibc does.
void _Stdio_String(_Stdio_Sink *sink, const _Stdio_Directive *dir, const char *str)
{
    size_t len;
    const char *end;
    int limit = dir->sd_precision;

    if (str == NULL) {
        str = limit == _STDIO_PRECISION_NONE || limit >= (int) sizeof(_ERR_STDIO_NULL) - 1 ? _ERR_STDIO_NULL : "";
    }
    if (limit == _STDIO_PRECISION_NONE) {
        len = strlen(str);
    } else {
        end = memchr(str, '\0', (size_t) limit);
        len = end != NULL ? (size_t) (end - str) : (size_t) limit;
    }
    _Stdio_Field(sink, dir, str, len);
}

// Give the sink the format's text, each directive converting an argument.
int _Stdio_Format(_Stdio_Sink *sink, const char *format, _Stdio_VaList args)
{
    char ch;
    int digit;
    size_t start;
    _Stdio_Directive dir;

    for (size_t i = 0; format[i] != '\0'; i++) {
        if (format[i] != '%') {
            _Stdio_Emit(sink, format + i, 1);
            continue;
        }

        // Phase: parse the directive
        start = i;
        dir.sd_flags = 0;
        for (i++; ; i++) {
            if (format[i] == '-') {
                dir.sd_flags |= _STDIO_FLAG_LEFT;
            } else if (format[i] == '+') {
                dir.sd_flags |= _STDIO_FLAG_SIGN;
            } else if (format[i] == ' ') {
                dir.sd_flags |= _STDIO_FLAG_SPACE;
            } else if (format[i] == '#') {
                dir.sd_flags |= _STDIO_FLAG_ALT;
            } else if (format[i] == '0') {
                dir.sd_flags |= _STDIO_FLAG_ZERO;
            } else {
                break;
            }
        }
        dir.sd_width = 0;
        if (format[i] == '*') {
            dir.sd_width = va_arg(args, int);
            i++;
        } else {
            for (; format[i] >= '0' && format[i] <= '9'; i++) {
                digit = format[i] - '0';
                dir.sd_width = dir.sd_width > (INT_MAX - digit) / _STDIO_COUNT_BASE ? INT_MAX : dir.sd_width * _STDIO_COUNT_BASE + digit;
            }
        }
        if (dir.sd_width < 0) {
            dir.sd_flags |= _STDIO_FLAG_LEFT;
            dir.sd_width = dir.sd_width == INT_MIN ? INT_MAX : -dir.sd_width;
        }
        dir.sd_precision = _STDIO_PRECISION_NONE;
        if (format[i] == '.' && format[i + 1] == '*') {
            dir.sd_precision = va_arg(args, int);
            i += 2;
        } else if (format[i] == '.') {
            dir.sd_precision = 0;
            for (i++; format[i] >= '0' && format[i] <= '9'; i++) {
                digit = format[i] - '0';
                dir.sd_precision = dir.sd_precision > (INT_MAX - digit) / _STDIO_COUNT_BASE ? INT_MAX : dir.sd_precision * _STDIO_COUNT_BASE + digit;
            }
        }
        if (dir.sd_precision < 0) {
            dir.sd_precision = _STDIO_PRECISION_NONE;
        }
        dir.sd_length = _STDIO_LENGTH_NONE;
        if (format[i] == 'h' && format[i + 1] == 'h') {
            dir.sd_length = _STDIO_LENGTH_CHAR;
            i += 2;
        } else if (format[i] == 'h') {
            dir.sd_length = _STDIO_LENGTH_SHORT;
            i++;
        } else if (format[i] == 'l' && format[i + 1] == 'l') {
            dir.sd_length = _STDIO_LENGTH_LLONG;
            i += 2;
        } else if (format[i] == 'l') {
            dir.sd_length = _STDIO_LENGTH_LONG;
            i++;
        } else if (format[i] == 'j') {
            dir.sd_length = _STDIO_LENGTH_INTMAX;
            i++;
        } else if (format[i] == 'z') {
            dir.sd_length = _STDIO_LENGTH_SIZE;
            i++;
        } else if (format[i] == 't') {
            dir.sd_length = _STDIO_LENGTH_PTRDIFF;
            i++;
        } else if (format[i] == 'L') {
            dir.sd_length = _STDIO_LENGTH_LDOUBLE;
            i++;
        }
        dir.sd_conv = format[i];

        // Phase: emit the conversion
        switch (dir.sd_length == _STDIO_LENGTH_NONE ? dir.sd_conv : '\0') {
            case '%': {
                _Stdio_Emit(sink, "%", 1);
            } break;
            case 'c': {
                ch = (char) va_arg(args, int);
                _Stdio_Field(sink, &dir, &ch, 1);
            } break;
            case 's': {
                _Stdio_String(sink, &dir, va_arg(args, const char *));
            } break;
            default: {
                _Stdio_Emit(sink, "%", 1);
                i = start;
            } break;
        }
    }
    return _Stdio_End(sink);
}

// End the sink's string, and return its count, or EOF if it failed.
int _Stdio_End(_Stdio_Sink *sink)
{
    if (sink->ss_stream == NULL && sink->ss_size > 0) {
        size_t last = sink->ss_size - 1;

        sink->ss_str[sink->ss_count < last ? sink->ss_count : last] = '\0';
    }
    return sink->ss_failed ? EOF : (int) sink->ss_count;
}

// Return how many bytes the stream took from its file and has yet to give out.
long _Stdio_Ahead(FILE *stream)
{
    long ahead = 0;

    if (stream->sf_state == _STDIO_READING) {
        ahead = (long) (stream->sf_tail - stream->sf_head);
    }
    if (stream->sf_flags & _STDIO_BACK) {
        ahead++;
    }
    return ahead;
}

// Move the stream to offset from the origin whence, and empty its buffer.
int _Stdio_Seek(FILE *stream, long offset, int whence)
{
    long pos;

    if (_Stdio_Flush(stream) != 0) {
        return EOF;
    }
    if (whence == _SYS_SEEK_CUR) {
        offset -= _Stdio_Ahead(stream);
    }
    pos = _Sys_Lseek(stream->sf_fd, offset, whence);
    if (pos < 0) {
        errno = (int) -pos;
        return EOF;
    }
    stream->sf_flags &= ~_STDIO_BACK;
    stream->sf_head = 0;
    stream->sf_tail = 0;
    return 0;
}

// Move the stream's file back over what the stream read ahead.
void _Stdio_Unread(FILE *stream)
{
    if (_Stdio_Ahead(stream) > 0) {
        _Stdio_Seek(stream, 0, _SYS_SEEK_CUR);
    }
}

// Return a number that differs from run to run, by the clock and the stack.
unsigned long _Stdio_TempSeed(void)
{
    _Sys_Timespec now = {0, 0};

    _Sys_ClockGettime(_SYS_CLOCK_REALTIME, &now);
    return (unsigned long) now.tv_sec ^ (unsigned long) now.tv_nsec ^ (unsigned long) &now;
}

// Write the next temporary name, one no call before wrote, into str.
void _Stdio_TempName(char *str)
{
    int k;
    unsigned long number;
    size_t len = sizeof(_STDIO_TEMP_PREFIX) - 1;

    if (! _Stdio_TempSeeded) {
        _Stdio_TempNumber = _Stdio_TempSeed() % _STDIO_TEMP_NAMES;
        _Stdio_TempSeeded = 1;
    }
    _Stdio_TempNumber = (_Stdio_TempNumber + _STDIO_TEMP_STEP) % _STDIO_TEMP_NAMES;
    number = _Stdio_TempNumber;
    memcpy(str, _STDIO_TEMP_PREFIX, len);
    for (k = 0; k < _STDIO_TEMP_DIGITS; k++) {
        str[len++] = _STDIO_TEMP_LETTERS[number % _STDIO_TEMP_RADIX];
        number /= _STDIO_TEMP_RADIX;
    }
    str[len] = '\0';
}

// Remove the file filename, or the empty directory of that name.
int remove(const char *filename)
{
    long status = _Sys_Unlink(filename);

    if (status == -_SYS_EISDIR) {
        status = _Sys_Rmdir(filename);
    }
    if (status < 0) {
        errno = (int) -status;
        return EOF;
    }
    return 0;
}

// Give the file old the name new, replacing a file that has it as Linux does.
int rename(const char *old, const char *new)
{
    long status = _Sys_Rename(old, new);

    if (status < 0) {
        errno = (int) -status;
        return EOF;
    }
    return 0;
}

// Open a file of no name for update, which goes when its stream is closed.
FILE *tmpfile(void)
{
    int tries;
    char name[L_tmpnam];
    long fd = -_SYS_EEXIST;
    FILE *stream = _Stdio_New();
    int flags = _SYS_O_RDWR | _SYS_O_CREAT | _SYS_O_EXCL;

    if (stream == NULL) {
        return NULL;
    }
    for (tries = 0; tries < TMP_MAX && fd == -_SYS_EEXIST; tries++) {
        _Stdio_TempName(name);
        fd = _Sys_Open(name, flags, _SYS_S_IRUSR | _SYS_S_IWUSR);
    }
    if (fd < 0) {
        errno = (int) -fd;
        _Stdio_Free(stream);
        return NULL;
    }
    _Sys_Unlink(name);
    _Stdio_Attach(stream, (int) fd, flags);
    return stream;
}

// Write a name no file has into str, or into an array of its own for NULL.
char *tmpnam(char *str)
{
    int tries;
    _Sys_Stat info;

    if (str == NULL) {
        str = _Stdio_TempBuffer;
    }
    for (tries = 0; tries < TMP_MAX; tries++) {
        _Stdio_TempName(str);
        if (_Sys_Lstat(str, &info) == -_SYS_ENOENT) {
            return str;
        }
    }
    return NULL;
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

// Write what the stream has to write, every stream's for NULL, or unread it.
int fflush(FILE *stream)
{
    if (stream == NULL) {
        return _Stdio_FlushAll();
    }
    _Stdio_Unread(stream);
    return _Stdio_Flush(stream);
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

// Write the arguments to the stream as the format says.
int fprintf(FILE *restrict stream, const char *restrict format, ...)
{
    int result;
    va_list args;

    va_start(args, format);
    result = vfprintf(stream, format, args);
    va_end(args);
    return result;
}

// Write the arguments to standard output as the format says.
int printf(const char *restrict format, ...)
{
    int result;
    va_list args;

    va_start(args, format);
    result = vfprintf(stdout, format, args);
    va_end(args);
    return result;
}

// Write the arguments into str's n characters as the format says.
int snprintf(char *restrict str, size_t n, const char *restrict format, ...)
{
    int result;
    va_list args;

    va_start(args, format);
    result = vsnprintf(str, n, format, args);
    va_end(args);
    return result;
}

// Write the arguments into str, however long, as the format says.
int sprintf(char *restrict str, const char *restrict format, ...)
{
    int result;
    va_list args;

    va_start(args, format);
    result = vsnprintf(str, SIZE_MAX, format, args);
    va_end(args);
    return result;
}

// Write the list's arguments to the stream as the format says.
int vfprintf(FILE *restrict stream, const char *restrict format, _Stdio_VaList arg)
{
    int result;
    unsigned char lent[BUFSIZ];
    _Stdio_Sink sink = {
        .ss_stream = stream
    };

    if (_Stdio_Lend(stream, lent, sizeof(lent)) != 0) {
        return EOF;
    }
    result = _Stdio_Format(&sink, format, arg);
    return _Stdio_Reclaim(stream) == 0 ? result : EOF;
}

// Write the list's arguments to standard output as the format says.
int vprintf(const char *restrict format, _Stdio_VaList arg)
{
    return vfprintf(stdout, format, arg);
}

// Write the list's arguments into str's n characters as the format says.
int vsnprintf(char *restrict str, size_t n, const char *restrict format, _Stdio_VaList arg)
{
    _Stdio_Sink sink = {
        .ss_str  = str,
        .ss_size = n
    };

    return _Stdio_Format(&sink, format, arg);
}

// Write the list's arguments into str, however long, as the format says.
int vsprintf(char *restrict str, const char *restrict format, _Stdio_VaList arg)
{
    return vsnprintf(str, SIZE_MAX, format, arg);
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

// Store the stream's position in pos.
int fgetpos(FILE *restrict stream, fpos_t *restrict pos)
{
    long offset = ftell(stream);

    if (offset < 0) {
        return EOF;
    }
    *pos = offset;
    return 0;
}

// Move the stream to offset from the origin whence, and clear its end-of-file.
int fseek(FILE *stream, long offset, int whence)
{
    if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END) {
        errno = _SYS_EINVAL;
        return EOF;
    }
    if (_Stdio_Seek(stream, offset, whence) != 0) {
        return EOF;
    }
    stream->sf_flags &= ~_STDIO_EOF;
    return 0;
}

// Move the stream to the position fgetpos stored in pos.
int fsetpos(FILE *stream, const fpos_t *pos)
{
    return fseek(stream, *pos, SEEK_SET);
}

// Return the stream's position, the bytes from the start of its file.
long ftell(FILE *stream)
{
    long pos;

    if (_Stdio_Flush(stream) != 0) {
        return EOF;
    }
    pos = _Sys_Lseek(stream->sf_fd, 0, _SYS_SEEK_CUR);
    if (pos < 0) {
        errno = (int) -pos;
        return EOF;
    }
    return pos - _Stdio_Ahead(stream);
}

// Move the stream to the start of its file, and clear its indicators.
void rewind(FILE *stream)
{
    fseek(stream, 0, SEEK_SET);
    stream->sf_flags &= ~_STDIO_ERROR;
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

// Write errno's text and a new-line to standard error, after str if it has any.
void perror(const char *str)
{
    const char *text = strerror(errno);
    unsigned char lent[BUFSIZ];

    _Stdio_Lend(stderr, lent, sizeof(lent));
    if (str != NULL && str[0] != '\0') {
        fputs(str, stderr);
        fputs(": ", stderr);
    }
    fputs(text, stderr);
    fputs("\n", stderr);
    _Stdio_Reclaim(stderr);
}
