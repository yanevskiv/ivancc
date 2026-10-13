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

// The hook exit flushes the streams by.
#include <_crt.h>

// The files' calls, the terminal's settings and the standard descriptors.
#include <_sys.h>

// The texts a null string or pointer, an infinity and a NaN are written as.
#include <_err.h>

// The integer a floating value's digits are those of.
#include <_bigint.h>

// The error a failed open, read, write or close reports.
#include <errno.h>

// Copying to and from a buffer, finding its new-lines and matching a mode.
#include <string.h>

// The streams fopen allocates and the buffers they do.
#include <stdlib.h>

// The largest int puts returns and a width or a precision holds.
#include <limits.h>

// The largest size fread and fwrite move, and the integers %jd and %p name.
#include <stdint.h>

// The arguments a format converts.
#include <stdarg.h>

// The difference of two pointers, which %td takes.
#include <stddef.h>

// The digits and the exponents of a long double.
#include <float.h>

// The character a number's fraction begins after.
#include <locale.h>

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

// Check the header's own widest integers against stdint.h's.
typedef char _Stdio_CheckIntmax[sizeof(_Stdio_IntmaxType) == sizeof(intmax_t) && (_Stdio_IntmaxType) -1 < 0 ? 1 : -1];
typedef char _Stdio_CheckUintmax[sizeof(_Stdio_UintmaxType) == sizeof(uintmax_t) && (_Stdio_UintmaxType) -1 > 0 ? 1 : -1];

// Check that a size and a pointer difference are one width, each %z and %t.
typedef char _Stdio_CheckPtrdiff[sizeof(size_t) == sizeof(ptrdiff_t) ? 1 : -1];

// Check that an integer's digits in octal, three bits each, fit their array.
typedef char _Stdio_CheckDigits[sizeof(uintmax_t) * CHAR_BIT <= _STDIO_DIGITS_MAX * 3 ? 1 : -1];

// Check that a long double's digits fill a word under a field of its exponent.
typedef char _Stdio_CheckLongDouble[sizeof(long double) >= sizeof(unsigned long long) + sizeof(unsigned short) ? 1 : -1];
typedef char _Stdio_CheckMantissa[LDBL_MANT_DIG == sizeof(unsigned long long) * CHAR_BIT ? 1 : -1];

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

// Parse a directive's percent sign, flags, width, precision and length.
size_t _Stdio_ParseHead(const char *format, int *flags, int *width, int *precision, int *length)
{
    int digit;
    size_t i = 0;

    if (format[i] != '%') {
        return 0;
    }

    // Phase: parse the flags
    *flags = 0;
    for (i++; ; i++) {
        if (format[i] == '-') {
            *flags |= _STDIO_FLAG_LEFT;
        } else if (format[i] == '+') {
            *flags |= _STDIO_FLAG_SIGN;
        } else if (format[i] == ' ') {
            *flags |= _STDIO_FLAG_SPACE;
        } else if (format[i] == '#') {
            *flags |= _STDIO_FLAG_ALT;
        } else if (format[i] == '0') {
            *flags |= _STDIO_FLAG_ZERO;
        } else {
            break;
        }
    }

    // Phase: parse the width
    *width = 0;
    if (format[i] == '*') {
        *width = _STDIO_COUNT_ARGUMENT;
        i++;
    } else {
        for (; format[i] >= '0' && format[i] <= '9'; i++) {
            digit = format[i] - '0';
            *width = *width > (INT_MAX - digit) / _STDIO_COUNT_BASE ? INT_MAX : *width * _STDIO_COUNT_BASE + digit;
        }
    }

    // Phase: parse the precision
    *precision = _STDIO_PRECISION_NONE;
    if (format[i] == '.' && format[i + 1] == '*') {
        *precision = _STDIO_COUNT_ARGUMENT;
        i += 2;
    } else if (format[i] == '.') {
        *precision = 0;
        for (i++; format[i] >= '0' && format[i] <= '9'; i++) {
            digit = format[i] - '0';
            *precision = *precision > (INT_MAX - digit) / _STDIO_COUNT_BASE ? INT_MAX : *precision * _STDIO_COUNT_BASE + digit;
        }
    }

    // Phase: parse the length
    *length = _STDIO_LENGTH_NONE;
    if (format[i] == 'h' && format[i + 1] == 'h') {
        *length = _STDIO_LENGTH_CHAR;
        i += 2;
    } else if (format[i] == 'h') {
        *length = _STDIO_LENGTH_SHORT;
        i++;
    } else if (format[i] == 'l' && format[i + 1] == 'l') {
        *length = _STDIO_LENGTH_LLONG;
        i += 2;
    } else if (format[i] == 'l') {
        *length = _STDIO_LENGTH_LONG;
        i++;
    } else if (format[i] == 'j') {
        *length = _STDIO_LENGTH_INTMAX;
        i++;
    } else if (format[i] == 'z') {
        *length = _STDIO_LENGTH_SIZE;
        i++;
    } else if (format[i] == 't') {
        *length = _STDIO_LENGTH_PTRDIFF;
        i++;
    } else if (format[i] == 'L') {
        *length = _STDIO_LENGTH_LDOUBLE;
        i++;
    }
    return i;
}

// Take the width and precision a directive left to its arguments.
void _Stdio_TakeHead(_Stdio_VaList *list, int *flags, int *width, int *precision)
{
    if (*width == _STDIO_COUNT_ARGUMENT) {
        *width = va_arg(*list, int);
    }
    if (*width < 0) {
        *flags |= _STDIO_FLAG_LEFT;
        *width = *width == INT_MIN ? INT_MAX : -*width;
    }
    if (*precision == _STDIO_COUNT_ARGUMENT) {
        *precision = va_arg(*list, int);
    }
    if (*precision < 0) {
        *precision = _STDIO_PRECISION_NONE;
    }
}

// Take an unsigned integer argument of the type a length names.
_Stdio_UintmaxType _Stdio_TakeUnsigned(_Stdio_VaList *list, int length)
{
    _Stdio_UintmaxType value;

    switch (length) {
        case _STDIO_LENGTH_CHAR: {
            value = (unsigned char) va_arg(*list, unsigned);
        } break;
        case _STDIO_LENGTH_SHORT: {
            value = (unsigned short) va_arg(*list, unsigned);
        } break;
        case _STDIO_LENGTH_LONG: {
            value = va_arg(*list, unsigned long);
        } break;
        case _STDIO_LENGTH_LLONG: {
            value = va_arg(*list, unsigned long long);
        } break;
        case _STDIO_LENGTH_INTMAX: {
            value = va_arg(*list, uintmax_t);
        } break;
        case _STDIO_LENGTH_SIZE:
        case _STDIO_LENGTH_PTRDIFF: {
            value = va_arg(*list, size_t);
        } break;
        default: {
            value = va_arg(*list, unsigned);
        } break;
    }
    return value;
}

// Parse the text before a directive.
_Bool _Stdio_ParseText(const char *format, size_t *len)
{
    *len = strcspn(format, "%");
    return *len > 0;
}

// Parse a %% directive.
_Bool _Stdio_ParsePercent(const char *format, _Stdio_VaList *list, size_t *len)
{
    int flags;
    int width;
    int length;
    int precision;
    size_t at = _Stdio_ParseHead(format, &flags, &width, &precision, &length);

    if (at == 0 || length != _STDIO_LENGTH_NONE || format[at] != '%') {
        return 0;
    }
    _Stdio_TakeHead(list, &flags, &width, &precision);
    *len = at + 1;
    return 1;
}

// Parse a %c directive with its argument.
_Bool _Stdio_ParseChar(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, char *ch)
{
    int length;
    int precision;
    size_t at = _Stdio_ParseHead(format, flags, width, &precision, &length);

    if (at == 0 || length != _STDIO_LENGTH_NONE || format[at] != 'c') {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, &precision);
    *ch = (char) va_arg(*list, int);
    *len = at + 1;
    return 1;
}

// Parse a %s directive with its argument.
_Bool _Stdio_ParseString(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, const char **str)
{
    int length;
    size_t at = _Stdio_ParseHead(format, flags, width, precision, &length);

    if (at == 0 || length != _STDIO_LENGTH_NONE || format[at] != 's') {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *str = va_arg(*list, const char *);
    *len = at + 1;
    return 1;
}

// Parse a %d or %i directive with its argument.
_Bool _Stdio_ParseDecimal(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_IntmaxType *value)
{
    int length;
    size_t at = _Stdio_ParseHead(format, flags, width, precision, &length);

    if (at == 0 || length == _STDIO_LENGTH_LDOUBLE || (format[at] != 'd' && format[at] != 'i')) {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    switch (length) {
        case _STDIO_LENGTH_CHAR: {
            *value = (signed char) va_arg(*list, int);
        } break;
        case _STDIO_LENGTH_SHORT: {
            *value = (short) va_arg(*list, int);
        } break;
        case _STDIO_LENGTH_LONG: {
            *value = va_arg(*list, long);
        } break;
        case _STDIO_LENGTH_LLONG: {
            *value = va_arg(*list, long long);
        } break;
        case _STDIO_LENGTH_INTMAX: {
            *value = va_arg(*list, intmax_t);
        } break;
        case _STDIO_LENGTH_SIZE:
        case _STDIO_LENGTH_PTRDIFF: {
            *value = va_arg(*list, ptrdiff_t);
        } break;
        default: {
            *value = va_arg(*list, int);
        } break;
    }
    *len = at + 1;
    return 1;
}

// Parse a %u directive with its argument.
_Bool _Stdio_ParseUnsigned(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_UintmaxType *value)
{
    int length;
    size_t at = _Stdio_ParseHead(format, flags, width, precision, &length);

    if (at == 0 || length == _STDIO_LENGTH_LDOUBLE || format[at] != 'u') {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *value = _Stdio_TakeUnsigned(list, length);
    *len = at + 1;
    return 1;
}

// Parse a %o directive with its argument.
_Bool _Stdio_ParseOctal(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_UintmaxType *value)
{
    int length;
    size_t at = _Stdio_ParseHead(format, flags, width, precision, &length);

    if (at == 0 || length == _STDIO_LENGTH_LDOUBLE || format[at] != 'o') {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *value = _Stdio_TakeUnsigned(list, length);
    *len = at + 1;
    return 1;
}

// Parse a %x or %X directive with its argument.
_Bool _Stdio_ParseHex(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_UintmaxType *value, _Bool *upper)
{
    int length;
    size_t at = _Stdio_ParseHead(format, flags, width, precision, &length);

    if (at == 0 || length == _STDIO_LENGTH_LDOUBLE || (format[at] != 'x' && format[at] != 'X')) {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *value = _Stdio_TakeUnsigned(list, length);
    *upper = format[at] == 'X';
    *len = at + 1;
    return 1;
}

// Parse a %p directive with its argument.
_Bool _Stdio_ParsePointer(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, const void **ptr)
{
    int length;
    size_t at = _Stdio_ParseHead(format, flags, width, precision, &length);

    if (at == 0 || length != _STDIO_LENGTH_NONE || format[at] != 'p') {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *ptr = va_arg(*list, const void *);
    *len = at + 1;
    return 1;
}

// Parse a %n directive with its argument.
_Bool _Stdio_ParseCount(const char *format, _Stdio_VaList *list, size_t *len, int *length, void **dest)
{
    int flags;
    int width;
    int precision;
    size_t at = _Stdio_ParseHead(format, &flags, &width, &precision, length);

    if (at == 0 || *length == _STDIO_LENGTH_LDOUBLE || format[at] != 'n') {
        return 0;
    }
    _Stdio_TakeHead(list, &flags, &width, &precision);
    *dest = va_arg(*list, void *);
    *len = at + 1;
    return 1;
}

// Parse a %f or %F directive with its argument.
_Bool _Stdio_ParseFixed(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Bool *upper)
{
    int length;
    size_t at = _Stdio_ParseHead(format, flags, width, precision, &length);

    if (at == 0 || (format[at] != 'f' && format[at] != 'F')) {
        return 0;
    }
    if (length != _STDIO_LENGTH_NONE && length != _STDIO_LENGTH_LONG && length != _STDIO_LENGTH_LDOUBLE) {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *value = length == _STDIO_LENGTH_LDOUBLE ? va_arg(*list, long double) : va_arg(*list, double);
    *upper = format[at] == 'F';
    *len = at + 1;
    return 1;
}

// Parse a %e or %E directive with its argument.
_Bool _Stdio_ParseExponential(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Bool *upper)
{
    int length;
    size_t at = _Stdio_ParseHead(format, flags, width, precision, &length);

    if (at == 0 || (format[at] != 'e' && format[at] != 'E')) {
        return 0;
    }
    if (length != _STDIO_LENGTH_NONE && length != _STDIO_LENGTH_LONG && length != _STDIO_LENGTH_LDOUBLE) {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *value = length == _STDIO_LENGTH_LDOUBLE ? va_arg(*list, long double) : va_arg(*list, double);
    *upper = format[at] == 'E';
    *len = at + 1;
    return 1;
}

// Parse a %g or %G directive with its argument.
_Bool _Stdio_ParseGeneral(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Bool *upper)
{
    int length;
    size_t at = _Stdio_ParseHead(format, flags, width, precision, &length);

    if (at == 0 || (format[at] != 'g' && format[at] != 'G')) {
        return 0;
    }
    if (length != _STDIO_LENGTH_NONE && length != _STDIO_LENGTH_LONG && length != _STDIO_LENGTH_LDOUBLE) {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *value = length == _STDIO_LENGTH_LDOUBLE ? va_arg(*list, long double) : va_arg(*list, double);
    *upper = format[at] == 'G';
    *len = at + 1;
    return 1;
}

// Write a value's digits in a base to end at end, none for zero.
size_t _Stdio_Digits(char *end, _Stdio_UintmaxType value, int base, const char *digits)
{
    char *ptr = end;

    while (value != 0) {
        ptr--;
        ptr[0] = digits[value % (_Stdio_UintmaxType) base];
        value /= (_Stdio_UintmaxType) base;
    }
    return (size_t) (end - ptr);
}

// Store a count at dest as an integer of the type a length names.
void _Stdio_StoreCount(void *dest, int length, size_t count)
{
    switch (length) {
        case _STDIO_LENGTH_CHAR: {
            *(signed char *) dest = (signed char) count;
        } break;
        case _STDIO_LENGTH_SHORT: {
            *(short *) dest = (short) count;
        } break;
        case _STDIO_LENGTH_LONG: {
            *(long *) dest = (long) count;
        } break;
        case _STDIO_LENGTH_LLONG: {
            *(long long *) dest = (long long) count;
        } break;
        case _STDIO_LENGTH_INTMAX: {
            *(intmax_t *) dest = (intmax_t) count;
        } break;
        case _STDIO_LENGTH_SIZE: {
            *(size_t *) dest = count;
        } break;
        case _STDIO_LENGTH_PTRDIFF: {
            *(ptrdiff_t *) dest = (ptrdiff_t) count;
        } break;
        default: {
            *(int *) dest = (int) count;
        } break;
    }
}

// Write a floating value's sign, what it is and its every decimal digit.
void _Stdio_Exact(long double value, _Stdio_Numeral *num)
{
    int i;
    int exp;
    int step;
    int below;
    int place = 0;
    unsigned int chunk;
    unsigned int biased;
    unsigned short top;
    _Bigint_Number big;
    unsigned long long part;
    unsigned long long mant;
    char run[_BIGINT_POW5_STEP];
    unsigned int ones = (unsigned int) LDBL_MAX_EXP * 2 - 1;
    unsigned long long lead = 1ULL << (LDBL_MANT_DIG - 1);
    char *end = num->sn_digit + sizeof(num->sn_digit);
    char *ptr = end;

    // Phase: take the value apart
    memcpy(&mant, &value, sizeof(mant));
    memcpy(&top, (char *) &value + sizeof(mant), sizeof(top));
    biased = top & ones;
    exp = (int) (biased != 0 ? biased : 1) - (LDBL_MAX_EXP - 1) - (LDBL_MANT_DIG - 1);
    num->sn_kind = _STDIO_REAL_FINITE;
    num->sn_negative = top > ones;
    num->sn_len = 0;
    num->sn_exp = 0;
    if (biased == ones) {
        num->sn_kind = (mant & (lead - 1)) != 0 ? _STDIO_REAL_NAN : _STDIO_REAL_INFINITE;
    }
    if (biased != 0 && ! (mant & lead)) {
        num->sn_kind = _STDIO_REAL_NAN;
    }
    if (num->sn_kind != _STDIO_REAL_FINITE) {
        return;
    }

    // Phase: write the whole part's digits
    _Bigint_Set(&big, mant);
    if (exp > 0) {
        _Bigint_ShiftLeft(&big, exp);
    } else {
        _Bigint_ShiftRight(&big, -exp);
    }
    while (big.bn_len > 0) {
        chunk = _Bigint_DivideWord(&big, _BIGINT_POW10_WORD);
        for (i = 0; i < _BIGINT_POW10_STEP; i++) {
            ptr--;
            ptr[0] = (char) ('0' + chunk % _STDIO_BASE_DECIMAL);
            chunk /= _STDIO_BASE_DECIMAL;
        }
    }
    while (ptr < end && ptr[0] == '0') {
        ptr++;
    }
    num->sn_len = (size_t) (end - ptr);
    num->sn_exp = num->sn_len > 0 ? (int) num->sn_len - 1 : 0;
    memmove(num->sn_digit, ptr, num->sn_len);

    // Phase: write the fraction's digits
    below = exp < 0 ? -exp : 0;
    _Bigint_Set(&big, mant);
    _Bigint_Split(&big, below);
    while (big.bn_len > 0) {
        step = below < _BIGINT_POW5_STEP ? below : _BIGINT_POW5_STEP;
        _Bigint_MulPow5(&big, step);
        below -= step;
        part = _Bigint_Split(&big, below);
        for (i = step - 1; i >= 0; i--) {
            run[i] = (char) ('0' + part % _STDIO_BASE_DECIMAL);
            part /= _STDIO_BASE_DECIMAL;
        }
        for (i = 0; i < step; i++) {
            place++;
            if (num->sn_len == 0 && run[i] != '0') {
                num->sn_exp = -place;
            }
            if (num->sn_len > 0 || run[i] != '0') {
                num->sn_digit[num->sn_len++] = run[i];
            }
        }
    }

    // Phase: drop the zeros at the end
    while (num->sn_len > 0 && num->sn_digit[num->sn_len - 1] == '0') {
        num->sn_len--;
    }
}

// Round num to its first keep digits, to nearest and a tie to even.
void _Stdio_Round(_Stdio_Numeral *num, long keep)
{
    _Bool up;
    size_t len;
    char *digit = num->sn_digit;

    if (keep >= (long) num->sn_len) {
        return;
    }
    len = keep > 0 ? (size_t) keep : 0;
    up = keep >= 0 && digit[len] >= '5';
    if (up && digit[len] == '5' && len + 1 == num->sn_len) {
        up = len > 0 && (digit[len - 1] - '0') % 2 != 0;
    }
    while (up && len > 0 && digit[len - 1] == '9') {
        len--;
    }
    if (up && len == 0) {
        digit[0] = '1';
        len = 1;
        num->sn_exp++;
    } else if (up) {
        digit[len - 1]++;
    }
    while (len > 0 && digit[len - 1] == '0') {
        len--;
    }
    num->sn_len = len;
    if (len == 0) {
        num->sn_exp = 0;
    }
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
void _Stdio_EmitPad(_Stdio_Sink *sink, int ch, size_t len)
{
    char chunk[_STDIO_PAD_CHUNK];

    memset(chunk, ch, sizeof(chunk));
    while (len > 0 && ! sink->ss_failed) {
        size_t part = len < sizeof(chunk) ? len : sizeof(chunk);

        _Stdio_Emit(sink, chunk, part);
        len -= part;
    }
}

// Give the sink len characters of data in a width.
void _Stdio_EmitField(_Stdio_Sink *sink, int flags, int width, const char *data, size_t len)
{
    size_t pad = (size_t) width > len ? (size_t) width - len : 0;

    if (! (flags & _STDIO_FLAG_LEFT)) {
        _Stdio_EmitPad(sink, ' ', pad);
    }
    _Stdio_Emit(sink, data, len);
    if (flags & _STDIO_FLAG_LEFT) {
        _Stdio_EmitPad(sink, ' ', pad);
    }
}

// Give the sink what leads a number, and return the spaces to follow it.
size_t _Stdio_EmitLead(_Stdio_Sink *sink, int flags, int width, _Bool negative, const char *prefix, size_t len)
{
    size_t pad;
    size_t lead;
    const char *sign = "";

    if (negative) {
        sign = "-";
    } else if (flags & _STDIO_FLAG_SIGN) {
        sign = "+";
    } else if (flags & _STDIO_FLAG_SPACE) {
        sign = " ";
    }
    if (flags & _STDIO_FLAG_LEFT) {
        flags &= ~_STDIO_FLAG_ZERO;
    }
    lead = strlen(sign) + strlen(prefix);
    pad = (size_t) width > lead + len ? (size_t) width - lead - len : 0;
    if (! (flags & (_STDIO_FLAG_LEFT | _STDIO_FLAG_ZERO))) {
        _Stdio_EmitPad(sink, ' ', pad);
    }
    _Stdio_Emit(sink, sign, strlen(sign));
    _Stdio_Emit(sink, prefix, strlen(prefix));
    if (flags & _STDIO_FLAG_ZERO) {
        _Stdio_EmitPad(sink, '0', pad);
    }
    return flags & _STDIO_FLAG_LEFT ? pad : 0;
}

// Give the sink an integer's digits in a width, its precision the fewest.
void _Stdio_EmitInteger(_Stdio_Sink *sink, int flags, int width, int precision, _Bool negative, const char *prefix, const char *digits, size_t len)
{
    size_t rest;
    size_t zeros;

    if (precision == _STDIO_PRECISION_NONE) {
        precision = _STDIO_INTEGER_PRECISION;
    } else {
        flags &= ~_STDIO_FLAG_ZERO;
    }
    zeros = (size_t) precision > len ? (size_t) precision - len : 0;
    rest = _Stdio_EmitLead(sink, flags, width, negative, prefix, zeros + len);
    _Stdio_EmitPad(sink, '0', zeros);
    _Stdio_Emit(sink, digits, len);
    _Stdio_EmitPad(sink, ' ', rest);
}

// Give the sink count digits of num from the one worth 10^high, zeros for none.
void _Stdio_EmitSpan(_Stdio_Sink *sink, const _Stdio_Numeral *num, long high, size_t count)
{
    size_t real;
    size_t lead = high > num->sn_exp ? (size_t) (high - num->sn_exp) : 0;
    size_t skip = high < num->sn_exp ? (size_t) (num->sn_exp - high) : 0;

    if (lead > count) {
        lead = count;
    }
    count -= lead;
    real = skip < num->sn_len ? num->sn_len - skip : 0;
    if (real > count) {
        real = count;
    }
    _Stdio_EmitPad(sink, '0', lead);
    _Stdio_Emit(sink, num->sn_digit + skip, real);
    _Stdio_EmitPad(sink, '0', count - real);
}

// Give the sink a rounded numeral in a width, frac digits after its point.
void _Stdio_EmitNumeral(_Stdio_Sink *sink, int flags, int width, const _Stdio_Numeral *num, size_t frac, const char *mark)
{
    size_t rest;
    char text[_STDIO_DIGITS_MAX];
    _Bool powered = mark[0] != '\0';
    char point = localeconv()->decimal_point[0];
    long high = powered || num->sn_exp > 0 ? num->sn_exp : 0;
    size_t whole = powered ? 1 : (size_t) high + 1;
    size_t dot = frac > 0 || (flags & _STDIO_FLAG_ALT) ? 1 : 0;
    _Stdio_UintmaxType power = (_Stdio_UintmaxType) (num->sn_exp < 0 ? -num->sn_exp : num->sn_exp);
    size_t count = _Stdio_Digits(text + sizeof(text), power, _STDIO_BASE_DECIMAL, _STDIO_DIGITS_LOWER);
    size_t zeros = count < _STDIO_EXPONENT_MIN ? _STDIO_EXPONENT_MIN - count : 0;
    size_t tail = powered ? strlen(mark) + 1 + zeros + count : 0;

    rest = _Stdio_EmitLead(sink, flags, width, num->sn_negative, "", whole + dot + frac + tail);
    _Stdio_EmitSpan(sink, num, high, whole);
    _Stdio_Emit(sink, &point, dot);
    _Stdio_EmitSpan(sink, num, high - (long) whole, frac);
    if (powered) {
        _Stdio_Emit(sink, mark, strlen(mark));
        _Stdio_Emit(sink, num->sn_exp < 0 ? "-" : "+", 1);
        _Stdio_EmitPad(sink, '0', zeros);
        _Stdio_Emit(sink, text + sizeof(text) - count, count);
    }
    _Stdio_EmitPad(sink, ' ', rest);
}

// Give the sink an infinity or a NaN in a width, a sign and a text, no zeros.
void _Stdio_EmitSpecial(_Stdio_Sink *sink, int flags, int width, const _Stdio_Numeral *num, _Bool upper)
{
    size_t rest;
    const char *inf = upper ? _ERR_STDIO_INF_UPPER : _ERR_STDIO_INF;
    const char *nan = upper ? _ERR_STDIO_NAN_UPPER : _ERR_STDIO_NAN;
    const char *text = num->sn_kind == _STDIO_REAL_NAN ? nan : inf;

    flags &= ~_STDIO_FLAG_ZERO;
    rest = _Stdio_EmitLead(sink, flags, width, num->sn_negative, "", strlen(text));
    _Stdio_Emit(sink, text, strlen(text));
    _Stdio_EmitPad(sink, ' ', rest);
}

// Give the sink a character as %c converts it.
void _Stdio_EmitChar(_Stdio_Sink *sink, int flags, int width, char ch)
{
    _Stdio_EmitField(sink, flags, width, &ch, 1);
}

// Give the sink a string as %s converts it, a null one as glibc does.
void _Stdio_EmitString(_Stdio_Sink *sink, int flags, int width, int precision, const char *str)
{
    size_t len;
    const char *end;

    if (str == NULL) {
        str = precision == _STDIO_PRECISION_NONE || precision >= (int) sizeof(_ERR_STDIO_NULL) - 1 ? _ERR_STDIO_NULL : "";
    }
    if (precision == _STDIO_PRECISION_NONE) {
        len = strlen(str);
    } else {
        end = memchr(str, '\0', (size_t) precision);
        len = end != NULL ? (size_t) (end - str) : (size_t) precision;
    }
    _Stdio_EmitField(sink, flags, width, str, len);
}

// Give the sink an integer as %d and %i convert it.
void _Stdio_EmitDecimal(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_IntmaxType value)
{
    char text[_STDIO_DIGITS_MAX];
    _Stdio_UintmaxType magnitude = value < 0 ? -(_Stdio_UintmaxType) value : (_Stdio_UintmaxType) value;
    size_t len = _Stdio_Digits(text + sizeof(text), magnitude, _STDIO_BASE_DECIMAL, _STDIO_DIGITS_LOWER);

    _Stdio_EmitInteger(sink, flags, width, precision, value < 0, "", text + sizeof(text) - len, len);
}

// Give the sink an unsigned integer as %u converts it.
void _Stdio_EmitUnsigned(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_UintmaxType value)
{
    char text[_STDIO_DIGITS_MAX];
    size_t len = _Stdio_Digits(text + sizeof(text), value, _STDIO_BASE_DECIMAL, _STDIO_DIGITS_LOWER);

    flags &= ~(_STDIO_FLAG_SIGN | _STDIO_FLAG_SPACE);
    _Stdio_EmitInteger(sink, flags, width, precision, 0, "", text + sizeof(text) - len, len);
}

// Give the sink an unsigned integer as %o converts it.
void _Stdio_EmitOctal(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_UintmaxType value)
{
    char text[_STDIO_DIGITS_MAX];
    const char *prefix = "";
    size_t least = precision == _STDIO_PRECISION_NONE ? _STDIO_INTEGER_PRECISION : (size_t) precision;
    size_t len = _Stdio_Digits(text + sizeof(text), value, _STDIO_BASE_OCTAL, _STDIO_DIGITS_LOWER);

    if ((flags & _STDIO_FLAG_ALT) && len >= least) {
        prefix = "0";
    }
    flags &= ~(_STDIO_FLAG_SIGN | _STDIO_FLAG_SPACE);
    _Stdio_EmitInteger(sink, flags, width, precision, 0, prefix, text + sizeof(text) - len, len);
}

// Give the sink an unsigned integer as %x and %X convert it.
void _Stdio_EmitHex(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_UintmaxType value, _Bool upper)
{
    char text[_STDIO_DIGITS_MAX];
    const char *prefix = "";
    size_t len = _Stdio_Digits(text + sizeof(text), value, _STDIO_BASE_HEX, upper ? _STDIO_DIGITS_UPPER : _STDIO_DIGITS_LOWER);

    if ((flags & _STDIO_FLAG_ALT) && value != 0) {
        prefix = upper ? "0X" : "0x";
    }
    flags &= ~(_STDIO_FLAG_SIGN | _STDIO_FLAG_SPACE);
    _Stdio_EmitInteger(sink, flags, width, precision, 0, prefix, text + sizeof(text) - len, len);
}

// Give the sink a pointer as glibc does: a null one a text, another by %#lx.
void _Stdio_EmitPointer(_Stdio_Sink *sink, int flags, int width, int precision, const void *ptr)
{
    char text[_STDIO_DIGITS_MAX];
    size_t len = _Stdio_Digits(text + sizeof(text), (uintptr_t) ptr, _STDIO_BASE_HEX, _STDIO_DIGITS_LOWER);

    if (ptr == NULL) {
        _Stdio_EmitField(sink, flags, width, _ERR_STDIO_NIL, sizeof(_ERR_STDIO_NIL) - 1);
        return;
    }
    _Stdio_EmitInteger(sink, flags, width, precision, 0, "0x", text + sizeof(text) - len, len);
}

// Give the sink a floating value as %f and %F convert it.
void _Stdio_EmitFixed(_Stdio_Sink *sink, int flags, int width, int precision, long double value, _Bool upper)
{
    _Stdio_Numeral num;

    _Stdio_Exact(value, &num);
    if (num.sn_kind != _STDIO_REAL_FINITE) {
        _Stdio_EmitSpecial(sink, flags, width, &num, upper);
        return;
    }
    if (precision == _STDIO_PRECISION_NONE) {
        precision = _STDIO_REAL_PRECISION;
    }
    _Stdio_Round(&num, (long) num.sn_exp + 1 + precision);
    _Stdio_EmitNumeral(sink, flags, width, &num, (size_t) precision, "");
}

// Give the sink a floating value as %e and %E convert it.
void _Stdio_EmitExponential(_Stdio_Sink *sink, int flags, int width, int precision, long double value, _Bool upper)
{
    _Stdio_Numeral num;

    _Stdio_Exact(value, &num);
    if (num.sn_kind != _STDIO_REAL_FINITE) {
        _Stdio_EmitSpecial(sink, flags, width, &num, upper);
        return;
    }
    if (precision == _STDIO_PRECISION_NONE) {
        precision = _STDIO_REAL_PRECISION;
    }
    _Stdio_Round(&num, (long) precision + 1);
    _Stdio_EmitNumeral(sink, flags, width, &num, (size_t) precision, upper ? "E" : "e");
}

// Give the sink a floating value as %g and %G convert it.
void _Stdio_EmitGeneral(_Stdio_Sink *sink, int flags, int width, int precision, long double value, _Bool upper)
{
    long frac;
    _Bool powered;
    _Stdio_Numeral num;
    const char *mark = "";

    _Stdio_Exact(value, &num);
    if (num.sn_kind != _STDIO_REAL_FINITE) {
        _Stdio_EmitSpecial(sink, flags, width, &num, upper);
        return;
    }
    if (precision == _STDIO_PRECISION_NONE) {
        precision = _STDIO_REAL_PRECISION;
    } else if (precision == 0) {
        precision = _STDIO_REAL_LEAST;
    }
    _Stdio_Round(&num, precision);
    powered = num.sn_exp < _STDIO_GENERAL_MIN || num.sn_exp >= precision;
    frac = (flags & _STDIO_FLAG_ALT ? precision : (long) num.sn_len) - 1;
    if (powered) {
        mark = upper ? "E" : "e";
    } else {
        frac -= num.sn_exp;
    }
    _Stdio_EmitNumeral(sink, flags, width, &num, frac > 0 ? (size_t) frac : 0, mark);
}

// Give the sink the format's text, each directive converting its arguments.
int _Stdio_EmitFormat(_Stdio_Sink *sink, const char *format, _Stdio_VaList args)
{
    char ch;
    int flags;
    int width;
    int length;
    size_t len;
    void *dest;
    _Bool upper;
    va_list list;
    int precision;
    const char *str;
    const void *ptr;
    long double fval;
    _Stdio_IntmaxType sval;
    _Stdio_UintmaxType uval;

    va_copy(list, args);
    for (size_t i = 0; format[i] != '\0'; i += len) {
        if (_Stdio_ParseText(format + i, &len)) {
            _Stdio_Emit(sink, format + i, len);
        } else if (_Stdio_ParsePercent(format + i, &list, &len)) {
            _Stdio_Emit(sink, "%", 1);
        } else if (_Stdio_ParseChar(format + i, &list, &len, &flags, &width, &ch)) {
            _Stdio_EmitChar(sink, flags, width, ch);
        } else if (_Stdio_ParseString(format + i, &list, &len, &flags, &width, &precision, &str)) {
            _Stdio_EmitString(sink, flags, width, precision, str);
        } else if (_Stdio_ParseDecimal(format + i, &list, &len, &flags, &width, &precision, &sval)) {
            _Stdio_EmitDecimal(sink, flags, width, precision, sval);
        } else if (_Stdio_ParseUnsigned(format + i, &list, &len, &flags, &width, &precision, &uval)) {
            _Stdio_EmitUnsigned(sink, flags, width, precision, uval);
        } else if (_Stdio_ParseOctal(format + i, &list, &len, &flags, &width, &precision, &uval)) {
            _Stdio_EmitOctal(sink, flags, width, precision, uval);
        } else if (_Stdio_ParseHex(format + i, &list, &len, &flags, &width, &precision, &uval, &upper)) {
            _Stdio_EmitHex(sink, flags, width, precision, uval, upper);
        } else if (_Stdio_ParsePointer(format + i, &list, &len, &flags, &width, &precision, &ptr)) {
            _Stdio_EmitPointer(sink, flags, width, precision, ptr);
        } else if (_Stdio_ParseCount(format + i, &list, &len, &length, &dest)) {
            _Stdio_StoreCount(dest, length, sink->ss_count);
        } else if (_Stdio_ParseFixed(format + i, &list, &len, &flags, &width, &precision, &fval, &upper)) {
            _Stdio_EmitFixed(sink, flags, width, precision, fval, upper);
        } else if (_Stdio_ParseExponential(format + i, &list, &len, &flags, &width, &precision, &fval, &upper)) {
            _Stdio_EmitExponential(sink, flags, width, precision, fval, upper);
        } else if (_Stdio_ParseGeneral(format + i, &list, &len, &flags, &width, &precision, &fval, &upper)) {
            _Stdio_EmitGeneral(sink, flags, width, precision, fval, upper);
        } else {
            len = 1;
            _Stdio_Emit(sink, format + i, len);
        }
    }
    va_end(list);
    return _Stdio_End(sink);
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
    result = _Stdio_EmitFormat(&sink, format, arg);
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

    return _Stdio_EmitFormat(&sink, format, arg);
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
