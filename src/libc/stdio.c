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

// The streams fopen allocates, their buffers and a wide character's bytes.
#include <stdlib.h>

// The largest int puts returns and a width holds, and a character's most bytes.
#include <limits.h>

// The largest size fread and fwrite move, and the integers %jd and %p name.
#include <stdint.h>

// The arguments a format converts.
#include <stdarg.h>

// The difference of two pointers, which %td takes, and a wide character.
#include <stddef.h>

// The digits and the exponents of a double and a long double.
#include <float.h>

// The character a number's fraction begins after.
#include <locale.h>

// The white space an input directive skips.
#include <ctype.h>

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

// Check the header's own wide character against stddef.h's, and an argument's.
typedef char _Stdio_CheckWchar[sizeof(_Stdio_WcharType) == sizeof(wchar_t) && ((_Stdio_WcharType) -1 < 0) == ((wchar_t) -1 < 0) ? 1 : -1];
typedef char _Stdio_CheckWint[sizeof(_Stdio_WintType) == sizeof(wchar_t) && sizeof(_Stdio_WintType) >= sizeof(int) ? 1 : -1];

// Check that a size and a pointer difference are one width, each %z and %t.
typedef char _Stdio_CheckPtrdiff[sizeof(size_t) == sizeof(ptrdiff_t) ? 1 : -1];

// Check that an integer's digits in octal, three bits each, fit their array.
typedef char _Stdio_CheckDigits[sizeof(uintmax_t) * CHAR_BIT <= _STDIO_DIGITS_MAX * 3 ? 1 : -1];

// Check that a long double's digits fill a word under a field of its exponent.
typedef char _Stdio_CheckLongDouble[sizeof(long double) >= sizeof(unsigned long long) + sizeof(unsigned short) ? 1 : -1];
typedef char _Stdio_CheckMantissa[LDBL_MANT_DIG == sizeof(unsigned long long) * CHAR_BIT ? 1 : -1];

// Check that a mantissa is its hex digits, a double's fitting under the first.
typedef char _Stdio_CheckHex[LDBL_MANT_DIG == _STDIO_HEX_BITS * _STDIO_HEX_DIGITS ? 1 : -1];
typedef char _Stdio_CheckDouble[DBL_MANT_DIG - 1 <= _STDIO_HEX_BITS * (_STDIO_HEX_DIGITS - 1) ? 1 : -1];

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
FILE *_Stdio_MakeStream(void)
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

// Convert fopen's mode into the flags open takes.
int _Stdio_ConvertMode(const char *mode)
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

// Attach the descriptor fd to the stream, to read or write as its flags say.
void _Stdio_AttachDescriptor(FILE *stream, int fd, int flags)
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
int _Stdio_OpenFile(FILE *stream, const char *filename, const char *mode)
{
    long fd;
    int flags;

    if (! _Stdio_IsMode(mode)) {
        errno = _SYS_EINVAL;
        return EOF;
    }
    flags = _Stdio_ConvertMode(mode);
    fd = _Sys_InvokeOpen(filename, flags, _SYS_DEFFILEMODE);
    if (fd < 0) {
        errno = (int) -fd;
        return EOF;
    }
    _Stdio_AttachDescriptor(stream, (int) fd, flags);
    return 0;
}

// Reset the stream: flush it and take its buffer, indicators and access.
int _Stdio_ResetStream(FILE *stream)
{
    int result = _Stdio_FlushBuffer(stream);

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

// Free the stream if fopen made it, and take it off the open ones.
void _Stdio_FreeStream(FILE *stream)
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

// Set up the stream's buffering and buffer at first use, none without memory.
void _Stdio_SetupBuffer(FILE *stream)
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

    return _Sys_InvokeIoctl(fd, _SYS_TCGETS, &term) == 0;
}

// Lend an unbuffered stream buf for a call, the stream readied for output.
int _Stdio_LendBuffer(FILE *stream, unsigned char *buf, size_t size)
{
    if (_Stdio_PrepareOutput(stream) != 0) {
        return EOF;
    }
    if (stream->sf_mode == _IONBF) {
        stream->sf_buf = buf;
        stream->sf_size = size;
    }
    return 0;
}

// Reclaim a lent buffer: write what it holds, and give the stream its byte.
int _Stdio_ReclaimBuffer(FILE *stream)
{
    int result = 0;

    if (stream->sf_mode == _IONBF && stream->sf_state == _STDIO_WRITING) {
        result = _Stdio_FlushBuffer(stream);
        stream->sf_buf = &stream->sf_byte;
        stream->sf_size = 1;
    }
    return result;
}

// Prepare the stream for input, or set its error indicator if it is write-only.
int _Stdio_PrepareInput(FILE *stream)
{
    if (! (stream->sf_flags & _STDIO_READ)) {
        stream->sf_flags |= _STDIO_ERROR;
        errno = _SYS_EBADF;
        return EOF;
    }
    if (_Stdio_FlushBuffer(stream) != 0) {
        return EOF;
    }
    _Stdio_SetupBuffer(stream);
    stream->sf_state = _STDIO_READING;
    return 0;
}

// Get up to len bytes from the stream into data, and return how many came.
size_t _Stdio_GetBytes(FILE *stream, void *data, size_t len)
{
    size_t done = 0;
    unsigned char *bytes = data;

    if (_Stdio_PrepareInput(stream) != 0) {
        return 0;
    }
    if (len > 0 && (stream->sf_flags & _STDIO_BACK)) {
        stream->sf_flags &= ~_STDIO_BACK;
        bytes[done++] = stream->sf_back;
    }
    while (done < len) {
        size_t part = len - done;

        if (stream->sf_head == stream->sf_tail && part >= stream->sf_size) {
            part = _Stdio_ReadDescriptor(stream, bytes + done, part);
        } else {
            part = _Stdio_ReadBuffer(stream, bytes + done, part);
        }
        if (part == 0) {
            return done;
        }
        done += part;
    }
    return done;
}

// Read up to len bytes of the stream's buffer into data, refilled if empty.
size_t _Stdio_ReadBuffer(FILE *stream, unsigned char *data, size_t len)
{
    if (stream->sf_head == stream->sf_tail) {
        stream->sf_head = 0;
        stream->sf_tail = _Stdio_ReadDescriptor(stream, stream->sf_buf, stream->sf_size);
    }
    if (len > stream->sf_tail - stream->sf_head) {
        len = stream->sf_tail - stream->sf_head;
    }
    memcpy(data, stream->sf_buf + stream->sf_head, len);
    stream->sf_head += len;
    return len;
}

// Read up to len bytes from the stream's descriptor, none once at its end.
size_t _Stdio_ReadDescriptor(FILE *stream, unsigned char *data, size_t len)
{
    long done;

    if (stream->sf_flags & _STDIO_EOF) {
        return 0;
    }
    if (stream->sf_mode != _IOFBF) {
        _Stdio_FlushLines();
    }
    done = _Sys_InvokeRead(stream->sf_fd, data, len);
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

// Get a line of at most max characters, its new-line among them, into str.
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

// Prepare the stream for output, or set its error indicator if it is read-only.
int _Stdio_PrepareOutput(FILE *stream)
{
    if (! (stream->sf_flags & _STDIO_WRITE)) {
        stream->sf_flags |= _STDIO_ERROR;
        errno = _SYS_EBADF;
        return EOF;
    }
    if (stream->sf_state == _STDIO_READING) {
        _Stdio_DiscardReadAhead(stream);
        stream->sf_flags &= ~_STDIO_BACK;
        stream->sf_head = 0;
        stream->sf_tail = 0;
    }
    _Stdio_SetupBuffer(stream);
    stream->sf_state = _STDIO_WRITING;
    return 0;
}

// Put len bytes of data in the stream's buffer, and return how many it took.
size_t _Stdio_PutBytes(FILE *stream, const void *data, size_t len)
{
    size_t done = 0;
    const unsigned char *bytes = data;

    if (_Stdio_PrepareOutput(stream) != 0) {
        return 0;
    }
    _Crt_FlushStreams = _Stdio_FlushStreams;
    while (done < len) {
        size_t part = len - done;

        if (stream->sf_tail == 0 && part >= stream->sf_size) {
            return _Stdio_WriteDescriptor(stream, bytes + done, part) == 0 ? len : done;
        }
        if (part > stream->sf_size - stream->sf_tail) {
            part = stream->sf_size - stream->sf_tail;
        }
        memcpy(stream->sf_buf + stream->sf_tail, bytes + done, part);
        stream->sf_tail += part;
        done += part;
        if (stream->sf_tail == stream->sf_size && _Stdio_FlushBuffer(stream) != 0) {
            return done - part;
        }
    }
    if (stream->sf_mode == _IOLBF && memchr(data, '\n', len) != NULL && _Stdio_FlushBuffer(stream) != 0) {
        return 0;
    }
    return len;
}

// Write len bytes of data to the stream's descriptor, however many writes.
int _Stdio_WriteDescriptor(FILE *stream, const unsigned char *data, size_t len)
{
    while (len > 0) {
        long done = _Sys_InvokeWrite(stream->sf_fd, data, len);

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

// Flush what waits in the stream's buffer, dropping it if the write fails.
int _Stdio_FlushBuffer(FILE *stream)
{
    size_t len = stream->sf_tail;

    if (stream->sf_state != _STDIO_WRITING || len == 0) {
        return 0;
    }
    stream->sf_tail = 0;
    return _Stdio_WriteDescriptor(stream, stream->sf_buf, len);
}

// Flush every open stream, and return EOF if any failed.
int _Stdio_FlushStreams(void)
{
    FILE *stream;
    int result = 0;

    for (stream = _Stdio_Streams; stream != NULL; stream = stream->sf_next) {
        if (_Stdio_FlushBuffer(stream) != 0) {
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
            _Stdio_FlushBuffer(stream);
        }
    }
}

// Match a directive's percent sign, flags, width, precision and length.
size_t _Stdio_MatchPrintHead(const char *format, int *flags, int *width, int *precision, int *length)
{
    int digit;
    size_t i = 0;

    if (format[i] != '%') {
        return 0;
    }

    // Phase: match the flags
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

    // Phase: match the width
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

    // Phase: match the precision
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

    // Phase: match the length
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

// Match the text before a directive.
_Bool _Stdio_MatchPrintText(const char *format, size_t *len)
{
    *len = strcspn(format, "%");
    return *len > 0;
}

// Match a %% directive.
_Bool _Stdio_MatchPrintPercent(const char *format, _Stdio_VaList *list, size_t *len)
{
    int flags;
    int width;
    int length;
    int precision;
    size_t at = _Stdio_MatchPrintHead(format, &flags, &width, &precision, &length);

    if (at == 0 || length != _STDIO_LENGTH_NONE || format[at] != '%') {
        return 0;
    }
    _Stdio_TakeHead(list, &flags, &width, &precision);
    *len = at + 1;
    return 1;
}

// Match a %c directive with its argument.
_Bool _Stdio_MatchPrintChar(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, char *ch)
{
    int length;
    int precision;
    size_t at = _Stdio_MatchPrintHead(format, flags, width, &precision, &length);

    if (at == 0 || length != _STDIO_LENGTH_NONE || format[at] != 'c') {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, &precision);
    *ch = (char) va_arg(*list, int);
    *len = at + 1;
    return 1;
}

// Match a %s directive with its argument.
_Bool _Stdio_MatchPrintString(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, const char **str)
{
    int length;
    size_t at = _Stdio_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length != _STDIO_LENGTH_NONE || format[at] != 's') {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *str = va_arg(*list, const char *);
    *len = at + 1;
    return 1;
}

// Match a %d or %i directive with its argument.
_Bool _Stdio_MatchPrintDecimal(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_IntmaxType *value)
{
    int length;
    size_t at = _Stdio_MatchPrintHead(format, flags, width, precision, &length);

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

// Match a %u directive with its argument.
_Bool _Stdio_MatchPrintUnsigned(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_UintmaxType *value)
{
    int length;
    size_t at = _Stdio_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length == _STDIO_LENGTH_LDOUBLE || format[at] != 'u') {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *value = _Stdio_TakeUnsigned(list, length);
    *len = at + 1;
    return 1;
}

// Match a %o directive with its argument.
_Bool _Stdio_MatchPrintOctal(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_UintmaxType *value)
{
    int length;
    size_t at = _Stdio_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length == _STDIO_LENGTH_LDOUBLE || format[at] != 'o') {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *value = _Stdio_TakeUnsigned(list, length);
    *len = at + 1;
    return 1;
}

// Match a %x or %X directive with its argument.
_Bool _Stdio_MatchPrintHex(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, _Stdio_UintmaxType *value, _Stdio_Case *letters)
{
    int length;
    size_t at = _Stdio_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length == _STDIO_LENGTH_LDOUBLE || (format[at] != 'x' && format[at] != 'X')) {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *value = _Stdio_TakeUnsigned(list, length);
    *letters = format[at] == 'X' ? _STDIO_CASE_UPPER : _STDIO_CASE_LOWER;
    *len = at + 1;
    return 1;
}

// Match a %p directive with its argument.
_Bool _Stdio_MatchPrintPointer(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, const void **ptr)
{
    int length;
    size_t at = _Stdio_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length != _STDIO_LENGTH_NONE || format[at] != 'p') {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *ptr = va_arg(*list, const void *);
    *len = at + 1;
    return 1;
}

// Match a %n directive with its argument.
_Bool _Stdio_MatchPrintCount(const char *format, _Stdio_VaList *list, size_t *len, int *length, void **dest)
{
    int flags;
    int width;
    int precision;
    size_t at = _Stdio_MatchPrintHead(format, &flags, &width, &precision, length);

    if (at == 0 || *length == _STDIO_LENGTH_LDOUBLE || format[at] != 'n') {
        return 0;
    }
    _Stdio_TakeHead(list, &flags, &width, &precision);
    *dest = va_arg(*list, void *);
    *len = at + 1;
    return 1;
}

// Match a %f or %F directive with its argument.
_Bool _Stdio_MatchPrintFixed(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Stdio_Case *letters)
{
    int length;
    size_t at = _Stdio_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || (format[at] != 'f' && format[at] != 'F')) {
        return 0;
    }
    if (length != _STDIO_LENGTH_NONE && length != _STDIO_LENGTH_LONG && length != _STDIO_LENGTH_LDOUBLE) {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *value = length == _STDIO_LENGTH_LDOUBLE ? va_arg(*list, long double) : va_arg(*list, double);
    *letters = format[at] == 'F' ? _STDIO_CASE_UPPER : _STDIO_CASE_LOWER;
    *len = at + 1;
    return 1;
}

// Match a %e or %E directive with its argument.
_Bool _Stdio_MatchPrintExponential(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Stdio_Case *letters)
{
    int length;
    size_t at = _Stdio_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || (format[at] != 'e' && format[at] != 'E')) {
        return 0;
    }
    if (length != _STDIO_LENGTH_NONE && length != _STDIO_LENGTH_LONG && length != _STDIO_LENGTH_LDOUBLE) {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *value = length == _STDIO_LENGTH_LDOUBLE ? va_arg(*list, long double) : va_arg(*list, double);
    *letters = format[at] == 'E' ? _STDIO_CASE_UPPER : _STDIO_CASE_LOWER;
    *len = at + 1;
    return 1;
}

// Match a %g or %G directive with its argument.
_Bool _Stdio_MatchPrintGeneral(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Stdio_Case *letters)
{
    int length;
    size_t at = _Stdio_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || (format[at] != 'g' && format[at] != 'G')) {
        return 0;
    }
    if (length != _STDIO_LENGTH_NONE && length != _STDIO_LENGTH_LONG && length != _STDIO_LENGTH_LDOUBLE) {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *value = length == _STDIO_LENGTH_LDOUBLE ? va_arg(*list, long double) : va_arg(*list, double);
    *letters = format[at] == 'G' ? _STDIO_CASE_UPPER : _STDIO_CASE_LOWER;
    *len = at + 1;
    return 1;
}

// Match a %a or %A directive with its argument, a double.
_Bool _Stdio_MatchPrintHexReal(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, double *value, _Stdio_Case *letters)
{
    int length;
    size_t at = _Stdio_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || (format[at] != 'a' && format[at] != 'A')) {
        return 0;
    }
    if (length != _STDIO_LENGTH_NONE && length != _STDIO_LENGTH_LONG) {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *value = va_arg(*list, double);
    *letters = format[at] == 'A' ? _STDIO_CASE_UPPER : _STDIO_CASE_LOWER;
    *len = at + 1;
    return 1;
}

// Match a %La or %LA directive with its argument, a long double.
_Bool _Stdio_MatchPrintHexLongReal(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, long double *value, _Stdio_Case *letters)
{
    int length;
    size_t at = _Stdio_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length != _STDIO_LENGTH_LDOUBLE || (format[at] != 'a' && format[at] != 'A')) {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *value = va_arg(*list, long double);
    *letters = format[at] == 'A' ? _STDIO_CASE_UPPER : _STDIO_CASE_LOWER;
    *len = at + 1;
    return 1;
}

// Match a %lc directive with its argument, a wide character.
_Bool _Stdio_MatchPrintWideChar(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, _Stdio_WcharType *wc)
{
    int length;
    int precision;
    size_t at = _Stdio_MatchPrintHead(format, flags, width, &precision, &length);

    if (at == 0 || length != _STDIO_LENGTH_LONG || format[at] != 'c') {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, &precision);
    *wc = (wchar_t) va_arg(*list, _Stdio_WintType);
    *len = at + 1;
    return 1;
}

// Match a %ls directive with its argument, a wide string.
_Bool _Stdio_MatchPrintWideString(const char *format, _Stdio_VaList *list, size_t *len, int *flags, int *width, int *precision, const _Stdio_WcharType **wstr)
{
    int length;
    size_t at = _Stdio_MatchPrintHead(format, flags, width, precision, &length);

    if (at == 0 || length != _STDIO_LENGTH_LONG || format[at] != 's') {
        return 0;
    }
    _Stdio_TakeHead(list, flags, width, precision);
    *wstr = va_arg(*list, const wchar_t *);
    *len = at + 1;
    return 1;
}

// Write a value's digits in a base to end at end, none for zero.
size_t _Stdio_WriteDigits(char *end, _Stdio_UintmaxType value, int base, const char *digits)
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

// Unpack a floating value into a sign and its bits by a power of two.
_Stdio_Real _Stdio_UnpackReal(long double value, _Stdio_Sign *sign, unsigned long long *mant, int *exp)
{
    unsigned short top;
    unsigned int biased;
    _Stdio_Real kind = _STDIO_REAL_FINITE;
    unsigned int ones = (unsigned int) LDBL_MAX_EXP * 2 - 1;
    unsigned long long lead = 1ULL << (LDBL_MANT_DIG - 1);

    memcpy(mant, &value, sizeof(*mant));
    memcpy(&top, (char *) &value + sizeof(*mant), sizeof(top));
    biased = top & ones;
    *exp = (int) (biased != 0 ? biased : 1) - (LDBL_MAX_EXP - 1) - (LDBL_MANT_DIG - 1);
    *sign = top > ones ? _STDIO_SIGN_MINUS : _STDIO_SIGN_PLUS;
    if (biased == ones) {
        kind = (*mant & (lead - 1)) != 0 ? _STDIO_REAL_NAN : _STDIO_REAL_INFINITE;
    }
    if (biased != 0 && ! (*mant & lead)) {
        kind = _STDIO_REAL_NAN;
    }
    return kind;
}

// Write a floating value's sign, what it is and its every decimal digit.
void _Stdio_WriteNumeral(long double value, _Stdio_Numeral *num)
{
    int i;
    int exp;
    int step;
    int below;
    int place = 0;
    unsigned int chunk;
    _Bigint_Number big;
    unsigned long long part;
    unsigned long long mant;
    char run[_BIGINT_POW5_STEP];
    char *end = num->sn_digit + sizeof(num->sn_digit);
    char *ptr = end;

    // Phase: take the value apart
    num->sn_kind = _Stdio_UnpackReal(value, &num->sn_sign, &mant, &exp);
    num->sn_len = 0;
    num->sn_exp = 0;
    if (num->sn_kind != _STDIO_REAL_FINITE) {
        return;
    }

    // Phase: write the whole part's digits
    _Bigint_SetValue(&big, mant);
    if (exp > 0) {
        _Bigint_ShiftBitsLeft(&big, exp);
    } else {
        _Bigint_ShiftBitsRight(&big, -exp);
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
    _Bigint_SetValue(&big, mant);
    _Bigint_SplitBits(&big, below);
    while (big.bn_len > 0) {
        step = below < _BIGINT_POW5_STEP ? below : _BIGINT_POW5_STEP;
        _Bigint_MulPow5(&big, step);
        below -= step;
        part = _Bigint_SplitBits(&big, below);
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
void _Stdio_RoundNumeral(_Stdio_Numeral *num, long keep)
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
int _Stdio_EndSink(_Stdio_Sink *sink)
{
    if (sink->ss_stream == NULL && sink->ss_size > 0) {
        size_t last = sink->ss_size - 1;

        sink->ss_str[sink->ss_count < last ? sink->ss_count : last] = '\0';
    }
    return sink->ss_failed ? EOF : (int) sink->ss_count;
}

// Emit len characters of data, counted though the sink's string be full.
void _Stdio_EmitText(_Stdio_Sink *sink, const char *data, size_t len)
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
        sink->ss_failed = _Stdio_PutBytes(sink->ss_stream, data, len) != len;
    } else if (sink->ss_count + 1 < sink->ss_size) {
        room = sink->ss_size - 1 - sink->ss_count;
        memcpy(sink->ss_str + sink->ss_count, data, len < room ? len : room);
    }
    sink->ss_count += len;
}

// Emit len copies of the character ch.
void _Stdio_EmitPad(_Stdio_Sink *sink, int ch, size_t len)
{
    char chunk[_STDIO_PAD_CHUNK];

    memset(chunk, ch, sizeof(chunk));
    while (len > 0 && ! sink->ss_failed) {
        size_t part = len < sizeof(chunk) ? len : sizeof(chunk);

        _Stdio_EmitText(sink, chunk, part);
        len -= part;
    }
}

// Emit len characters of data in a width.
void _Stdio_EmitField(_Stdio_Sink *sink, int flags, int width, const char *data, size_t len)
{
    size_t pad = (size_t) width > len ? (size_t) width - len : 0;

    if (! (flags & _STDIO_FLAG_LEFT)) {
        _Stdio_EmitPad(sink, ' ', pad);
    }
    _Stdio_EmitText(sink, data, len);
    if (flags & _STDIO_FLAG_LEFT) {
        _Stdio_EmitPad(sink, ' ', pad);
    }
}

// Emit what leads a number, and return the spaces to follow it.
size_t _Stdio_EmitLead(_Stdio_Sink *sink, int flags, int width, _Stdio_Sign sign, const char *prefix, size_t len)
{
    size_t pad;
    size_t lead;
    const char *text = "";

    if (sign == _STDIO_SIGN_MINUS) {
        text = "-";
    } else if (flags & _STDIO_FLAG_SIGN) {
        text = "+";
    } else if (flags & _STDIO_FLAG_SPACE) {
        text = " ";
    }
    if (flags & _STDIO_FLAG_LEFT) {
        flags &= ~_STDIO_FLAG_ZERO;
    }
    lead = strlen(text) + strlen(prefix);
    pad = (size_t) width > lead + len ? (size_t) width - lead - len : 0;
    if (! (flags & (_STDIO_FLAG_LEFT | _STDIO_FLAG_ZERO))) {
        _Stdio_EmitPad(sink, ' ', pad);
    }
    _Stdio_EmitText(sink, text, strlen(text));
    _Stdio_EmitText(sink, prefix, strlen(prefix));
    if (flags & _STDIO_FLAG_ZERO) {
        _Stdio_EmitPad(sink, '0', pad);
    }
    return flags & _STDIO_FLAG_LEFT ? pad : 0;
}

// Emit an integer's digits in a width, its precision the fewest.
void _Stdio_EmitInteger(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_Sign sign, const char *prefix, const char *digits, size_t len)
{
    size_t rest;
    size_t zeros;

    if (precision == _STDIO_PRECISION_NONE) {
        precision = _STDIO_INTEGER_PRECISION;
    } else {
        flags &= ~_STDIO_FLAG_ZERO;
    }
    zeros = (size_t) precision > len ? (size_t) precision - len : 0;
    rest = _Stdio_EmitLead(sink, flags, width, sign, prefix, zeros + len);
    _Stdio_EmitPad(sink, '0', zeros);
    _Stdio_EmitText(sink, digits, len);
    _Stdio_EmitPad(sink, ' ', rest);
}

// Emit count digits of num from the one worth 10^high, zeros for none.
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
    _Stdio_EmitText(sink, num->sn_digit + skip, real);
    _Stdio_EmitPad(sink, '0', count - real);
}

// Emit a rounded numeral in a width, frac digits after its point.
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
    size_t count = _Stdio_WriteDigits(text + sizeof(text), power, _STDIO_BASE_DECIMAL, _STDIO_DIGITS_LOWER);
    size_t zeros = count < _STDIO_EXPONENT_MIN ? _STDIO_EXPONENT_MIN - count : 0;
    size_t tail = powered ? strlen(mark) + 1 + zeros + count : 0;

    rest = _Stdio_EmitLead(sink, flags, width, num->sn_sign, "", whole + dot + frac + tail);
    _Stdio_EmitSpan(sink, num, high, whole);
    _Stdio_EmitText(sink, &point, dot);
    _Stdio_EmitSpan(sink, num, high - (long) whole, frac);
    if (powered) {
        _Stdio_EmitText(sink, mark, strlen(mark));
        _Stdio_EmitText(sink, num->sn_exp < 0 ? "-" : "+", 1);
        _Stdio_EmitPad(sink, '0', zeros);
        _Stdio_EmitText(sink, text + sizeof(text) - count, count);
    }
    _Stdio_EmitPad(sink, ' ', rest);
}

// Emit an infinity or a NaN in a width, a sign and a text, no zeros.
void _Stdio_EmitSpecial(_Stdio_Sink *sink, int flags, int width, _Stdio_Sign sign, _Stdio_Real kind, _Stdio_Case letters)
{
    size_t rest;
    const char *inf = letters == _STDIO_CASE_UPPER ? _ERR_STDIO_INF_UPPER : _ERR_STDIO_INF;
    const char *nan = letters == _STDIO_CASE_UPPER ? _ERR_STDIO_NAN_UPPER : _ERR_STDIO_NAN;
    const char *text = kind == _STDIO_REAL_NAN ? nan : inf;

    flags &= ~_STDIO_FLAG_ZERO;
    rest = _Stdio_EmitLead(sink, flags, width, sign, "", strlen(text));
    _Stdio_EmitText(sink, text, strlen(text));
    _Stdio_EmitPad(sink, ' ', rest);
}

// Emit a mantissa in hex, its top four bits first, by a power of two.
void _Stdio_EmitMantissa(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_Sign sign, unsigned long long mant, int exp, _Stdio_Case letters)
{
    size_t dot;
    size_t kept;
    size_t rest;
    size_t zeros;
    size_t count;
    unsigned long long unit;
    unsigned long long below;
    char text[_STDIO_DIGITS_MAX];
    char digits[_STDIO_HEX_DIGITS];
    int last = _STDIO_HEX_DIGITS - 1;
    _Bool upper = letters == _STDIO_CASE_UPPER;
    char point = localeconv()->decimal_point[0];

    // Phase: round to the precision, or find the least that is exact
    if (precision == _STDIO_PRECISION_NONE) {
        for (precision = last; precision > 0 && (mant >> (_STDIO_HEX_BITS * (last - precision))) % _STDIO_BASE_HEX == 0; precision--) {
            // empty
        }
    }
    if (precision < last) {
        unit = 1ULL << (_STDIO_HEX_BITS * (last - precision));
        below = mant & (unit - 1);
        mant -= below;
        if (below > unit / 2 || (below == unit / 2 && (mant & unit) != 0)) {
            mant += unit;
            if (mant < unit) {
                mant = 1ULL << (_STDIO_HEX_BITS * last);
                exp += _STDIO_HEX_BITS;
            }
        }
    }

    // Phase: write its digits and its power
    dot = precision > 0 || (flags & _STDIO_FLAG_ALT) ? 1 : 0;
    memset(digits, '0', sizeof(digits));
    _Stdio_WriteDigits(digits + sizeof(digits), mant, _STDIO_BASE_HEX, upper ? _STDIO_DIGITS_UPPER : _STDIO_DIGITS_LOWER);
    count = _Stdio_WriteDigits(text + sizeof(text), (_Stdio_UintmaxType) (exp < 0 ? -exp : exp), _STDIO_BASE_DECIMAL, _STDIO_DIGITS_LOWER);
    zeros = count < _STDIO_POWER_MIN ? _STDIO_POWER_MIN - count : 0;
    kept = precision < last ? (size_t) precision : (size_t) last;
    rest = _Stdio_EmitLead(sink, flags, width, sign, upper ? "0X" : "0x", 1 + dot + (size_t) precision + 2 + zeros + count);
    _Stdio_EmitText(sink, digits, 1);
    _Stdio_EmitText(sink, &point, dot);
    _Stdio_EmitText(sink, digits + 1, kept);
    _Stdio_EmitPad(sink, '0', (size_t) precision - kept);
    _Stdio_EmitText(sink, upper ? "P" : "p", 1);
    _Stdio_EmitText(sink, exp < 0 ? "-" : "+", 1);
    _Stdio_EmitPad(sink, '0', zeros);
    _Stdio_EmitText(sink, text + sizeof(text) - count, count);
    _Stdio_EmitPad(sink, ' ', rest);
}

// Emit a character as %c converts it.
void _Stdio_EmitChar(_Stdio_Sink *sink, int flags, int width, char ch)
{
    _Stdio_EmitField(sink, flags, width, &ch, 1);
}

// Emit a string as %s converts it, a null one as glibc does.
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

// Emit an integer as %d and %i convert it.
void _Stdio_EmitDecimal(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_IntmaxType value)
{
    char text[_STDIO_DIGITS_MAX];
    _Stdio_UintmaxType magnitude = value < 0 ? -(_Stdio_UintmaxType) value : (_Stdio_UintmaxType) value;
    size_t len = _Stdio_WriteDigits(text + sizeof(text), magnitude, _STDIO_BASE_DECIMAL, _STDIO_DIGITS_LOWER);

    _Stdio_EmitInteger(sink, flags, width, precision, value < 0 ? _STDIO_SIGN_MINUS : _STDIO_SIGN_PLUS, "", text + sizeof(text) - len, len);
}

// Emit an unsigned integer as %u converts it.
void _Stdio_EmitUnsigned(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_UintmaxType value)
{
    char text[_STDIO_DIGITS_MAX];
    size_t len = _Stdio_WriteDigits(text + sizeof(text), value, _STDIO_BASE_DECIMAL, _STDIO_DIGITS_LOWER);

    flags &= ~(_STDIO_FLAG_SIGN | _STDIO_FLAG_SPACE);
    _Stdio_EmitInteger(sink, flags, width, precision, _STDIO_SIGN_PLUS, "", text + sizeof(text) - len, len);
}

// Emit an unsigned integer as %o converts it.
void _Stdio_EmitOctal(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_UintmaxType value)
{
    char text[_STDIO_DIGITS_MAX];
    const char *prefix = "";
    size_t least = precision == _STDIO_PRECISION_NONE ? _STDIO_INTEGER_PRECISION : (size_t) precision;
    size_t len = _Stdio_WriteDigits(text + sizeof(text), value, _STDIO_BASE_OCTAL, _STDIO_DIGITS_LOWER);

    if ((flags & _STDIO_FLAG_ALT) && len >= least) {
        prefix = "0";
    }
    flags &= ~(_STDIO_FLAG_SIGN | _STDIO_FLAG_SPACE);
    _Stdio_EmitInteger(sink, flags, width, precision, _STDIO_SIGN_PLUS, prefix, text + sizeof(text) - len, len);
}

// Emit an unsigned integer as %x and %X convert it.
void _Stdio_EmitHex(_Stdio_Sink *sink, int flags, int width, int precision, _Stdio_UintmaxType value, _Stdio_Case letters)
{
    char text[_STDIO_DIGITS_MAX];
    const char *prefix = "";
    const char *digits = letters == _STDIO_CASE_UPPER ? _STDIO_DIGITS_UPPER : _STDIO_DIGITS_LOWER;
    size_t len = _Stdio_WriteDigits(text + sizeof(text), value, _STDIO_BASE_HEX, digits);

    if ((flags & _STDIO_FLAG_ALT) && value != 0) {
        prefix = letters == _STDIO_CASE_UPPER ? "0X" : "0x";
    }
    flags &= ~(_STDIO_FLAG_SIGN | _STDIO_FLAG_SPACE);
    _Stdio_EmitInteger(sink, flags, width, precision, _STDIO_SIGN_PLUS, prefix, text + sizeof(text) - len, len);
}

// Emit a pointer as glibc does: a null one a text, another by %#lx.
void _Stdio_EmitPointer(_Stdio_Sink *sink, int flags, int width, int precision, const void *ptr)
{
    char text[_STDIO_DIGITS_MAX];
    size_t len = _Stdio_WriteDigits(text + sizeof(text), (uintptr_t) ptr, _STDIO_BASE_HEX, _STDIO_DIGITS_LOWER);

    if (ptr == NULL) {
        _Stdio_EmitField(sink, flags, width, _ERR_STDIO_NIL, sizeof(_ERR_STDIO_NIL) - 1);
        return;
    }
    _Stdio_EmitInteger(sink, flags, width, precision, _STDIO_SIGN_PLUS, "0x", text + sizeof(text) - len, len);
}

// Emit a floating value as %f and %F convert it.
void _Stdio_EmitFixed(_Stdio_Sink *sink, int flags, int width, int precision, long double value, _Stdio_Case letters)
{
    _Stdio_Numeral num;

    _Stdio_WriteNumeral(value, &num);
    if (num.sn_kind != _STDIO_REAL_FINITE) {
        _Stdio_EmitSpecial(sink, flags, width, num.sn_sign, num.sn_kind, letters);
        return;
    }
    if (precision == _STDIO_PRECISION_NONE) {
        precision = _STDIO_REAL_PRECISION;
    }
    _Stdio_RoundNumeral(&num, (long) num.sn_exp + 1 + precision);
    _Stdio_EmitNumeral(sink, flags, width, &num, (size_t) precision, "");
}

// Emit a floating value as %e and %E convert it.
void _Stdio_EmitExponential(_Stdio_Sink *sink, int flags, int width, int precision, long double value, _Stdio_Case letters)
{
    _Stdio_Numeral num;

    _Stdio_WriteNumeral(value, &num);
    if (num.sn_kind != _STDIO_REAL_FINITE) {
        _Stdio_EmitSpecial(sink, flags, width, num.sn_sign, num.sn_kind, letters);
        return;
    }
    if (precision == _STDIO_PRECISION_NONE) {
        precision = _STDIO_REAL_PRECISION;
    }
    _Stdio_RoundNumeral(&num, (long) precision + 1);
    _Stdio_EmitNumeral(sink, flags, width, &num, (size_t) precision, letters == _STDIO_CASE_UPPER ? "E" : "e");
}

// Emit a floating value as %g and %G convert it.
void _Stdio_EmitGeneral(_Stdio_Sink *sink, int flags, int width, int precision, long double value, _Stdio_Case letters)
{
    long frac;
    _Bool powered;
    _Stdio_Numeral num;
    const char *mark = "";

    _Stdio_WriteNumeral(value, &num);
    if (num.sn_kind != _STDIO_REAL_FINITE) {
        _Stdio_EmitSpecial(sink, flags, width, num.sn_sign, num.sn_kind, letters);
        return;
    }
    if (precision == _STDIO_PRECISION_NONE) {
        precision = _STDIO_REAL_PRECISION;
    } else if (precision == 0) {
        precision = _STDIO_REAL_LEAST;
    }
    _Stdio_RoundNumeral(&num, precision);
    powered = num.sn_exp < _STDIO_GENERAL_MIN || num.sn_exp >= precision;
    frac = (flags & _STDIO_FLAG_ALT ? precision : (long) num.sn_len) - 1;
    if (powered) {
        mark = letters == _STDIO_CASE_UPPER ? "E" : "e";
    } else {
        frac -= num.sn_exp;
    }
    _Stdio_EmitNumeral(sink, flags, width, &num, frac > 0 ? (size_t) frac : 0, mark);
}

// Emit a double as %a and %A convert it, its first digit a 1 or a 0.
void _Stdio_EmitHexReal(_Stdio_Sink *sink, int flags, int width, int precision, double value, _Stdio_Case letters)
{
    int exp;
    int under;
    _Stdio_Sign sign;
    unsigned long long mant;
    _Stdio_Real kind = _Stdio_UnpackReal(value, &sign, &mant, &exp);

    if (kind != _STDIO_REAL_FINITE) {
        _Stdio_EmitSpecial(sink, flags, width, sign, kind, letters);
        return;
    }
    exp = mant != 0 ? exp + LDBL_MANT_DIG - 1 : 0;
    under = exp < DBL_MIN_EXP - 1 ? DBL_MIN_EXP - 1 - exp : 0;
    _Stdio_EmitMantissa(sink, flags, width, precision, sign, mant >> (_STDIO_HEX_BITS - 1) >> under, exp + under, letters);
}

// Emit a long double as %La and %LA convert it, its top bits first.
void _Stdio_EmitHexLongReal(_Stdio_Sink *sink, int flags, int width, int precision, long double value, _Stdio_Case letters)
{
    int exp;
    _Stdio_Sign sign;
    unsigned long long mant;
    _Stdio_Real kind = _Stdio_UnpackReal(value, &sign, &mant, &exp);

    if (kind != _STDIO_REAL_FINITE) {
        _Stdio_EmitSpecial(sink, flags, width, sign, kind, letters);
        return;
    }
    exp = mant != 0 ? exp + LDBL_MANT_DIG - _STDIO_HEX_BITS : 0;
    _Stdio_EmitMantissa(sink, flags, width, precision, sign, mant, exp, letters);
}

// Emit a wide character as %lc converts it, a wide string of it alone.
void _Stdio_EmitWideChar(_Stdio_Sink *sink, int flags, int width, _Stdio_WcharType wc)
{
    _Stdio_WcharType pair[2] = {wc, L'\0'};

    _Stdio_EmitWideString(sink, flags, width, _STDIO_PRECISION_NONE, pair);
}

// Emit a wide string as %ls converts it, in the locale's bytes.
void _Stdio_EmitWideString(_Stdio_Sink *sink, int flags, int width, int precision, const _Stdio_WcharType *wstr)
{
    int n;
    size_t i;
    size_t pad;
    size_t len = 0;
    size_t count = 0;
    char bytes[MB_LEN_MAX];
    size_t max = precision == _STDIO_PRECISION_NONE ? SIZE_MAX : (size_t) precision;

    if (wstr == NULL) {
        _Stdio_EmitString(sink, flags, width, precision, NULL);
        return;
    }

    // Phase: count the characters whose bytes the precision holds
    for (; len < max && wstr[count] != L'\0'; count++) {
        n = wctomb(bytes, wstr[count]);
        if (n < 0) {
            sink->ss_failed = 1;
            return;
        }
        if ((size_t) n > max - len) {
            break;
        }
        len += (size_t) n;
    }

    // Phase: write them in the width
    pad = (size_t) width > len ? (size_t) width - len : 0;
    if (! (flags & _STDIO_FLAG_LEFT)) {
        _Stdio_EmitPad(sink, ' ', pad);
    }
    for (i = 0; i < count; i++) {
        n = wctomb(bytes, wstr[i]);
        _Stdio_EmitText(sink, bytes, (size_t) n);
    }
    if (flags & _STDIO_FLAG_LEFT) {
        _Stdio_EmitPad(sink, ' ', pad);
    }
}

// Emit the format's text, each directive converting its arguments.
int _Stdio_EmitFormat(_Stdio_Sink *sink, const char *format, _Stdio_VaList args)
{
    char ch;
    int flags;
    int width;
    int length;
    size_t len;
    void *dest;
    double dval;
    va_list list;
    int precision;
    const char *str;
    const void *ptr;
    long double fval;
    _Stdio_Case letters;
    _Stdio_WcharType wc;
    _Stdio_IntmaxType sval;
    _Stdio_UintmaxType uval;
    const _Stdio_WcharType *wstr;

    va_copy(list, args);
    for (size_t i = 0; format[i] != '\0'; i += len) {
        if (_Stdio_MatchPrintText(format + i, &len)) {
            _Stdio_EmitText(sink, format + i, len);
        } else if (_Stdio_MatchPrintPercent(format + i, &list, &len)) {
            _Stdio_EmitText(sink, "%", 1);
        } else if (_Stdio_MatchPrintChar(format + i, &list, &len, &flags, &width, &ch)) {
            _Stdio_EmitChar(sink, flags, width, ch);
        } else if (_Stdio_MatchPrintString(format + i, &list, &len, &flags, &width, &precision, &str)) {
            _Stdio_EmitString(sink, flags, width, precision, str);
        } else if (_Stdio_MatchPrintDecimal(format + i, &list, &len, &flags, &width, &precision, &sval)) {
            _Stdio_EmitDecimal(sink, flags, width, precision, sval);
        } else if (_Stdio_MatchPrintUnsigned(format + i, &list, &len, &flags, &width, &precision, &uval)) {
            _Stdio_EmitUnsigned(sink, flags, width, precision, uval);
        } else if (_Stdio_MatchPrintOctal(format + i, &list, &len, &flags, &width, &precision, &uval)) {
            _Stdio_EmitOctal(sink, flags, width, precision, uval);
        } else if (_Stdio_MatchPrintHex(format + i, &list, &len, &flags, &width, &precision, &uval, &letters)) {
            _Stdio_EmitHex(sink, flags, width, precision, uval, letters);
        } else if (_Stdio_MatchPrintPointer(format + i, &list, &len, &flags, &width, &precision, &ptr)) {
            _Stdio_EmitPointer(sink, flags, width, precision, ptr);
        } else if (_Stdio_MatchPrintCount(format + i, &list, &len, &length, &dest)) {
            _Stdio_StoreCount(dest, length, sink->ss_count);
        } else if (_Stdio_MatchPrintFixed(format + i, &list, &len, &flags, &width, &precision, &fval, &letters)) {
            _Stdio_EmitFixed(sink, flags, width, precision, fval, letters);
        } else if (_Stdio_MatchPrintExponential(format + i, &list, &len, &flags, &width, &precision, &fval, &letters)) {
            _Stdio_EmitExponential(sink, flags, width, precision, fval, letters);
        } else if (_Stdio_MatchPrintGeneral(format + i, &list, &len, &flags, &width, &precision, &fval, &letters)) {
            _Stdio_EmitGeneral(sink, flags, width, precision, fval, letters);
        } else if (_Stdio_MatchPrintHexReal(format + i, &list, &len, &flags, &width, &precision, &dval, &letters)) {
            _Stdio_EmitHexReal(sink, flags, width, precision, dval, letters);
        } else if (_Stdio_MatchPrintHexLongReal(format + i, &list, &len, &flags, &width, &precision, &fval, &letters)) {
            _Stdio_EmitHexLongReal(sink, flags, width, precision, fval, letters);
        } else if (_Stdio_MatchPrintWideChar(format + i, &list, &len, &flags, &width, &wc)) {
            _Stdio_EmitWideChar(sink, flags, width, wc);
        } else if (_Stdio_MatchPrintWideString(format + i, &list, &len, &flags, &width, &precision, &wstr)) {
            _Stdio_EmitWideString(sink, flags, width, precision, wstr);
        } else {
            len = 1;
            _Stdio_EmitText(sink, format + i, len);
        }
    }
    va_end(list);
    return _Stdio_EndSink(sink);
}

// Match an input directive's percent sign, asterisk, width and length.
size_t _Stdio_MatchScanHead(const char *format, _Stdio_Assign *assign, int *width, int *length)
{
    int digit;
    size_t from;
    size_t i = 0;

    if (format[i] != '%') {
        return 0;
    }
    i++;

    // Phase: match the asterisk
    *assign = _STDIO_ASSIGN_STORE;
    if (format[i] == '*') {
        *assign = _STDIO_ASSIGN_SUPPRESS;
        i++;
    }

    // Phase: match the width
    *width = _STDIO_SCAN_WIDTH_NONE;
    for (from = i; format[i] >= '0' && format[i] <= '9'; i++) {
        digit = format[i] - '0';
        *width = *width > (INT_MAX - digit) / _STDIO_COUNT_BASE ? INT_MAX : *width * _STDIO_COUNT_BASE + digit;
    }
    if (i > from && *width == 0) {
        return 0;
    }

    // Phase: match the length
    *length = _STDIO_SCAN_LENGTH_NONE;
    if (format[i] == 'h' && format[i + 1] == 'h') {
        *length = _STDIO_SCAN_LENGTH_CHAR;
        i += 2;
    } else if (format[i] == 'h') {
        *length = _STDIO_SCAN_LENGTH_SHORT;
        i++;
    } else if (format[i] == 'l' && format[i + 1] == 'l') {
        *length = _STDIO_SCAN_LENGTH_LLONG;
        i += 2;
    } else if (format[i] == 'l') {
        *length = _STDIO_SCAN_LENGTH_LONG;
        i++;
    } else if (format[i] == 'j') {
        *length = _STDIO_SCAN_LENGTH_INTMAX;
        i++;
    } else if (format[i] == 'z') {
        *length = _STDIO_SCAN_LENGTH_SIZE;
        i++;
    } else if (format[i] == 't') {
        *length = _STDIO_SCAN_LENGTH_PTRDIFF;
        i++;
    } else if (format[i] == 'L') {
        *length = _STDIO_SCAN_LENGTH_LDOUBLE;
        i++;
    }
    return i;
}

// Match a directive of white space, however much.
_Bool _Stdio_MatchScanSpace(const char *format, size_t *len)
{
    size_t i = 0;

    while (isspace((unsigned char) format[i])) {
        i++;
    }
    *len = i;
    return i > 0;
}

// Match a directive of ordinary characters, up to white space or a directive.
_Bool _Stdio_MatchScanOrdinary(const char *format, size_t *len)
{
    size_t i = 0;

    while (format[i] != '\0' && format[i] != '%' && ! isspace((unsigned char) format[i])) {
        i++;
    }
    *len = i;
    return i > 0;
}

// Match a %% directive, which is those two characters and no more.
_Bool _Stdio_MatchScanPercent(const char *format, size_t *len)
{
    if (format[0] != '%' || format[1] != '%') {
        return 0;
    }
    *len = 2;
    return 1;
}

// Match a %c directive with where it stores, null if it stores nothing.
_Bool _Stdio_MatchScanChar(const char *format, _Stdio_VaList *list, size_t *len, int *width, char **dest)
{
    int length;
    _Stdio_Assign assign;
    size_t at = _Stdio_MatchScanHead(format, &assign, width, &length);

    if (at == 0 || length != _STDIO_SCAN_LENGTH_NONE || format[at] != 'c') {
        return 0;
    }
    *dest = assign == _STDIO_ASSIGN_STORE ? va_arg(*list, char *) : NULL;
    *len = at + 1;
    return 1;
}

// Match a %s directive with where it stores, null if it stores nothing.
_Bool _Stdio_MatchScanString(const char *format, _Stdio_VaList *list, size_t *len, int *width, char **dest)
{
    int length;
    _Stdio_Assign assign;
    size_t at = _Stdio_MatchScanHead(format, &assign, width, &length);

    if (at == 0 || length != _STDIO_SCAN_LENGTH_NONE || format[at] != 's') {
        return 0;
    }
    *dest = assign == _STDIO_ASSIGN_STORE ? va_arg(*list, char *) : NULL;
    *len = at + 1;
    return 1;
}

// Match a %[ directive with its scanset and where it stores, null if nowhere.
_Bool _Stdio_MatchScanSet(const char *format, _Stdio_VaList *list, size_t *len, int *width, char **dest, const char **set, size_t *count, _Stdio_Accept *accept)
{
    int length;
    size_t end;
    size_t from;
    _Stdio_Assign assign;
    size_t at = _Stdio_MatchScanHead(format, &assign, width, &length);

    if (at == 0 || length != _STDIO_SCAN_LENGTH_NONE || format[at] != '[') {
        return 0;
    }

    // Phase: match the scanset, a right bracket first being one of it
    *accept = _STDIO_ACCEPT_LISTED;
    from = at + 1;
    if (format[from] == '^') {
        *accept = _STDIO_ACCEPT_UNLISTED;
        from++;
    }
    end = from;
    if (format[end] == ']') {
        end++;
    }
    while (format[end] != '\0' && format[end] != ']') {
        end++;
    }
    if (format[end] != ']') {
        return 0;
    }
    *set = format + from;
    *count = end - from;
    *dest = assign == _STDIO_ASSIGN_STORE ? va_arg(*list, char *) : NULL;
    *len = end + 1;
    return 1;
}

// Match a %n directive with its length and where it stores.
_Bool _Stdio_MatchScanCount(const char *format, _Stdio_VaList *list, size_t *len, int *length, void **dest)
{
    int width;
    _Stdio_Assign assign;
    size_t at = _Stdio_MatchScanHead(format, &assign, &width, length);

    if (at == 0 || *length == _STDIO_SCAN_LENGTH_LDOUBLE || format[at] != 'n') {
        return 0;
    }
    if (assign != _STDIO_ASSIGN_STORE || width != _STDIO_SCAN_WIDTH_NONE) {
        return 0;
    }
    *dest = va_arg(*list, void *);
    *len = at + 1;
    return 1;
}

// Match a %d directive with its length and where it stores, null if nowhere.
_Bool _Stdio_MatchScanDecimal(const char *format, _Stdio_VaList *list, size_t *len, int *width, int *length, void **dest)
{
    _Stdio_Assign assign;
    size_t at = _Stdio_MatchScanHead(format, &assign, width, length);

    if (at == 0 || *length == _STDIO_SCAN_LENGTH_LDOUBLE || format[at] != 'd') {
        return 0;
    }
    *dest = assign == _STDIO_ASSIGN_STORE ? va_arg(*list, void *) : NULL;
    *len = at + 1;
    return 1;
}

// Match a %i directive with its length and where it stores, null if nowhere.
_Bool _Stdio_MatchScanInteger(const char *format, _Stdio_VaList *list, size_t *len, int *width, int *length, void **dest)
{
    _Stdio_Assign assign;
    size_t at = _Stdio_MatchScanHead(format, &assign, width, length);

    if (at == 0 || *length == _STDIO_SCAN_LENGTH_LDOUBLE || format[at] != 'i') {
        return 0;
    }
    *dest = assign == _STDIO_ASSIGN_STORE ? va_arg(*list, void *) : NULL;
    *len = at + 1;
    return 1;
}

// Match a %o directive with its length and where it stores, null if nowhere.
_Bool _Stdio_MatchScanOctal(const char *format, _Stdio_VaList *list, size_t *len, int *width, int *length, void **dest)
{
    _Stdio_Assign assign;
    size_t at = _Stdio_MatchScanHead(format, &assign, width, length);

    if (at == 0 || *length == _STDIO_SCAN_LENGTH_LDOUBLE || format[at] != 'o') {
        return 0;
    }
    *dest = assign == _STDIO_ASSIGN_STORE ? va_arg(*list, void *) : NULL;
    *len = at + 1;
    return 1;
}

// Match a %u directive with its length and where it stores, null if nowhere.
_Bool _Stdio_MatchScanUnsigned(const char *format, _Stdio_VaList *list, size_t *len, int *width, int *length, void **dest)
{
    _Stdio_Assign assign;
    size_t at = _Stdio_MatchScanHead(format, &assign, width, length);

    if (at == 0 || *length == _STDIO_SCAN_LENGTH_LDOUBLE || format[at] != 'u') {
        return 0;
    }
    *dest = assign == _STDIO_ASSIGN_STORE ? va_arg(*list, void *) : NULL;
    *len = at + 1;
    return 1;
}

// Match a %x or %X directive with its length and where it stores, if anywhere.
_Bool _Stdio_MatchScanHex(const char *format, _Stdio_VaList *list, size_t *len, int *width, int *length, void **dest)
{
    _Stdio_Assign assign;
    size_t at = _Stdio_MatchScanHead(format, &assign, width, length);

    if (at == 0 || *length == _STDIO_SCAN_LENGTH_LDOUBLE || (format[at] != 'x' && format[at] != 'X')) {
        return 0;
    }
    *dest = assign == _STDIO_ASSIGN_STORE ? va_arg(*list, void *) : NULL;
    *len = at + 1;
    return 1;
}

// Match a %p directive with where it stores, null if it stores nothing.
_Bool _Stdio_MatchScanPointer(const char *format, _Stdio_VaList *list, size_t *len, int *width, void **dest)
{
    int length;
    _Stdio_Assign assign;
    size_t at = _Stdio_MatchScanHead(format, &assign, width, &length);

    if (at == 0 || length != _STDIO_SCAN_LENGTH_NONE || format[at] != 'p') {
        return 0;
    }
    *dest = assign == _STDIO_ASSIGN_STORE ? va_arg(*list, void *) : NULL;
    *len = at + 1;
    return 1;
}

// Match a %a, %e, %f or %g directive, of either case, with its length, l or L.
_Bool _Stdio_MatchScanReal(const char *format, _Stdio_VaList *list, size_t *len, int *width, int *length, void **dest)
{
    _Stdio_Assign assign;
    size_t at = _Stdio_MatchScanHead(format, &assign, width, length);

    if (at == 0 || format[at] == '\0' || strchr(_STDIO_SCAN_REALS, format[at]) == NULL) {
        return 0;
    }
    if (*length != _STDIO_SCAN_LENGTH_NONE && *length != _STDIO_SCAN_LENGTH_LONG && *length != _STDIO_SCAN_LENGTH_LDOUBLE) {
        return 0;
    }
    *dest = assign == _STDIO_ASSIGN_STORE ? va_arg(*list, void *) : NULL;
    *len = at + 1;
    return 1;
}

// Match a %lc directive with where it stores, null if it stores nothing.
_Bool _Stdio_MatchScanWideChar(const char *format, _Stdio_VaList *list, size_t *len, int *width, _Stdio_WcharType **dest)
{
    int length;
    _Stdio_Assign assign;
    size_t at = _Stdio_MatchScanHead(format, &assign, width, &length);

    if (at == 0 || length != _STDIO_SCAN_LENGTH_LONG || format[at] != 'c') {
        return 0;
    }
    *dest = assign == _STDIO_ASSIGN_STORE ? va_arg(*list, _Stdio_WcharType *) : NULL;
    *len = at + 1;
    return 1;
}

// Match a %ls directive with where it stores, null if it stores nothing.
_Bool _Stdio_MatchScanWideString(const char *format, _Stdio_VaList *list, size_t *len, int *width, _Stdio_WcharType **dest)
{
    int length;
    _Stdio_Assign assign;
    size_t at = _Stdio_MatchScanHead(format, &assign, width, &length);

    if (at == 0 || length != _STDIO_SCAN_LENGTH_LONG || format[at] != 's') {
        return 0;
    }
    *dest = assign == _STDIO_ASSIGN_STORE ? va_arg(*list, _Stdio_WcharType *) : NULL;
    *len = at + 1;
    return 1;
}

// Match a %l[ directive with its scanset and where it stores, null if nowhere.
_Bool _Stdio_MatchScanWideSet(const char *format, _Stdio_VaList *list, size_t *len, int *width, _Stdio_WcharType **dest, const char **set, size_t *count, _Stdio_Accept *accept)
{
    int length;
    size_t end;
    size_t from;
    _Stdio_Assign assign;
    size_t at = _Stdio_MatchScanHead(format, &assign, width, &length);

    if (at == 0 || length != _STDIO_SCAN_LENGTH_LONG || format[at] != '[') {
        return 0;
    }

    // Phase: match the scanset, a right bracket first being one of it
    *accept = _STDIO_ACCEPT_LISTED;
    from = at + 1;
    if (format[from] == '^') {
        *accept = _STDIO_ACCEPT_UNLISTED;
        from++;
    }
    end = from;
    if (format[end] == ']') {
        end++;
    }
    while (format[end] != '\0' && format[end] != ']') {
        end++;
    }
    if (format[end] != ']') {
        return 0;
    }
    *set = format + from;
    *count = end - from;
    *dest = assign == _STDIO_ASSIGN_STORE ? va_arg(*list, _Stdio_WcharType *) : NULL;
    *len = end + 1;
    return 1;
}

// Peek at the source's next character, left unread, or EOF if there is none.
int _Stdio_PeekSource(_Stdio_Source *source)
{
    char ch;
    FILE *stream = source->so_stream;

    if (stream == NULL) {
        ch = source->so_str[source->so_count];
        return ch != '\0' ? (unsigned char) ch : EOF;
    }
    if (stream->sf_flags & _STDIO_BACK) {
        return stream->sf_back;
    }
    if (stream->sf_head == stream->sf_tail) {
        stream->sf_head = 0;
        stream->sf_tail = _Stdio_ReadDescriptor(stream, stream->sf_buf, stream->sf_size);
    }
    return stream->sf_head < stream->sf_tail ? stream->sf_buf[stream->sf_head] : EOF;
}

// Advance the source past the character peeked at, which counts as read.
void _Stdio_AdvanceSource(_Stdio_Source *source)
{
    FILE *stream = source->so_stream;

    if (stream != NULL && (stream->sf_flags & _STDIO_BACK)) {
        stream->sf_flags &= ~_STDIO_BACK;
    } else if (stream != NULL) {
        stream->sf_head++;
    }
    source->so_count++;
}

// End the source's input, and return its assignments, or EOF if it had none.
int _Stdio_EndSource(_Stdio_Source *source)
{
    if (source->so_failure == _STDIO_FAILURE_INPUT && source->so_stored == 0) {
        return EOF;
    }
    return source->so_stored;
}

// Return whether ch is in a scanset's count characters, a-z being a range.
_Bool _Stdio_IsInSet(const char *set, size_t count, int ch)
{
    const unsigned char *list = (const unsigned char *) set;

    for (size_t i = 0; i < count; i++) {
        if (i > 0 && i + 1 < count && list[i] == '-' && list[i - 1] <= list[i + 1]) {
            if (ch >= list[i - 1] && ch < list[i + 1]) {
                return 1;
            }
        } else if (ch == list[i]) {
            return 1;
        }
    }
    return 0;
}

// Read the longest input that is or begins an integer of a base into item.
_Bool _Stdio_ReadInteger(_Stdio_Source *source, int width, int base, char *item)
{
    int ch;
    int digit;
    size_t first;
    size_t len = 0;
    size_t read = 0;
    size_t digits = 0;
    size_t max = width == _STDIO_SCAN_WIDTH_NONE ? SIZE_MAX : (size_t) width;

    // Phase: read the sign
    ch = _Stdio_PeekSource(source);
    if (ch == '+' || ch == '-') {
        item[len++] = (char) ch;
        _Stdio_AdvanceSource(source);
        read++;
    }

    // Phase: read the prefix, which settles the base if the directive did not
    first = len;
    if (read < max && _Stdio_PeekSource(source) == '0') {
        item[len++] = '0';
        _Stdio_AdvanceSource(source);
        read++;
        digits++;
        ch = _Stdio_PeekSource(source);
        if (read < max && (ch == 'x' || ch == 'X') && (base == _STDIO_SCAN_BASE_PREFIX || base == _STDIO_SCAN_BASE_HEX)) {
            item[len++] = (char) ch;
            _Stdio_AdvanceSource(source);
            read++;
            first = len;
            digits = 0;
            base = _STDIO_SCAN_BASE_HEX;
        } else if (base == _STDIO_SCAN_BASE_PREFIX) {
            base = _STDIO_SCAN_BASE_OCTAL;
        }
    } else if (base == _STDIO_SCAN_BASE_PREFIX) {
        base = _STDIO_SCAN_BASE_DECIMAL;
    }

    // Phase: read the digits, keeping one leading zero and what item holds
    for (; read < max; read++) {
        ch = _Stdio_PeekSource(source);
        digit = isdigit(ch) ? ch - '0' : isxdigit(ch) ? tolower(ch) - 'a' + _STDIO_SCAN_BASE_DECIMAL : base;
        if (digit >= base) {
            break;
        }
        if (len < _STDIO_SCAN_ITEM - 1 && (ch != '0' || len != first + 1 || item[first] != '0')) {
            item[len++] = (char) ch;
        }
        _Stdio_AdvanceSource(source);
        digits++;
    }
    item[len] = '\0';
    if (digits == 0) {
        source->so_failure = read == 0 && ch == EOF ? _STDIO_FAILURE_INPUT : _STDIO_FAILURE_MATCHING;
    }
    return digits > 0;
}

// Store value at dest as an integer of a length's type, signed or not.
void _Stdio_StoreInteger(void *dest, int length, unsigned long long value)
{
    switch (length) {
        case _STDIO_SCAN_LENGTH_CHAR: {
            *(unsigned char *) dest = (unsigned char) value;
        } break;
        case _STDIO_SCAN_LENGTH_SHORT: {
            *(unsigned short *) dest = (unsigned short) value;
        } break;
        case _STDIO_SCAN_LENGTH_LONG: {
            *(unsigned long *) dest = (unsigned long) value;
        } break;
        case _STDIO_SCAN_LENGTH_LLONG: {
            *(unsigned long long *) dest = value;
        } break;
        case _STDIO_SCAN_LENGTH_INTMAX: {
            *(uintmax_t *) dest = (uintmax_t) value;
        } break;
        case _STDIO_SCAN_LENGTH_SIZE: {
            *(size_t *) dest = (size_t) value;
        } break;
        case _STDIO_SCAN_LENGTH_PTRDIFF: {
            *(ptrdiff_t *) dest = (ptrdiff_t) value;
        } break;
        default: {
            *(unsigned int *) dest = (unsigned int) value;
        } break;
    }
}

// Take the source's next characters into item while set has them, max at most.
size_t _Stdio_TakeChars(_Stdio_Source *source, _Stdio_Item *item, const char *set, size_t max)
{
    int ch;
    char *text;
    size_t size;
    size_t count = 0;

    while (count < max && item->si_room > 0) {
        ch = _Stdio_PeekSource(source);
        if (ch == EOF || ch == '\0' || strchr(set, tolower(ch)) == NULL) {
            break;
        }

        // Phase: grow the item to hold one character more and its null
        if (item->si_len + 2 > item->si_size) {
            size = item->si_size == 0 ? _STDIO_SCAN_ITEM : 2 * item->si_size;
            text = realloc(item->si_text, size);
            if (text == NULL) {
                source->so_failure = _STDIO_FAILURE_INPUT;
                item->si_room = 0;
                break;
            }
            item->si_text = text;
            item->si_size = size;
        }
        item->si_text[item->si_len++] = (char) ch;
        item->si_text[item->si_len] = '\0';
        _Stdio_AdvanceSource(source);
        item->si_room--;
        count++;
    }
    return count;
}

// Take the source's next characters into item while they spell word, any case.
size_t _Stdio_TakeWord(_Stdio_Source *source, _Stdio_Item *item, const char *word)
{
    size_t count = 0;
    char set[2] = {'\0', '\0'};

    for (; word[count] != '\0'; count++) {
        set[0] = word[count];
        if (_Stdio_TakeChars(source, item, set, 1) == 0) {
            break;
        }
    }
    return count;
}

// Read the longest input that is or begins a floating number into a new item.
void _Stdio_ReadReal(_Stdio_Source *source, int width, _Stdio_Item *item)
{
    int ch;
    size_t digits;
    const char *mark = _STDIO_SCAN_EXPONENT;
    const char *set = _STDIO_SCAN_DECIMALS;

    item->si_text = NULL;
    item->si_len = 0;
    item->si_size = 0;
    item->si_room = width == _STDIO_SCAN_WIDTH_NONE ? SIZE_MAX : (size_t) width;

    // Phase: read the sign
    _Stdio_TakeChars(source, item, _STDIO_SCAN_SIGNS, 1);
    ch = item->si_room > 0 ? tolower(_Stdio_PeekSource(source)) : EOF;

    // Phase: read an infinity, or a NaN and what its parentheses hold
    if (ch == _STDIO_SCAN_INFINITY[0]) {
        _Stdio_TakeWord(source, item, _STDIO_SCAN_INFINITY);
        return;
    }
    if (ch == _STDIO_SCAN_NAN[0]) {
        if (_Stdio_TakeWord(source, item, _STDIO_SCAN_NAN) == sizeof(_STDIO_SCAN_NAN) - 1 && _Stdio_TakeChars(source, item, "(", 1) > 0) {
            _Stdio_TakeChars(source, item, _STDIO_SCAN_NCHARS, SIZE_MAX);
            _Stdio_TakeChars(source, item, ")", 1);
        }
        return;
    }

    // Phase: read 0x, which makes the digits hex and the exponent a power of 2
    digits = _Stdio_TakeChars(source, item, "0", 1);
    if (digits > 0 && _Stdio_TakeChars(source, item, "x", 1) > 0) {
        digits = 0;
        set = _STDIO_SCAN_HEXES;
        mark = _STDIO_SCAN_POWER;
    }

    // Phase: read the digits and the point among them
    digits += _Stdio_TakeChars(source, item, set, SIZE_MAX);
    _Stdio_TakeChars(source, item, _STDIO_SCAN_POINT, 1);
    digits += _Stdio_TakeChars(source, item, set, SIZE_MAX);

    // Phase: read the exponent, which only a number with a digit has
    if (digits > 0 && _Stdio_TakeChars(source, item, mark, 1) > 0) {
        _Stdio_TakeChars(source, item, _STDIO_SCAN_SIGNS, 1);
        _Stdio_TakeChars(source, item, _STDIO_SCAN_DECIMALS, SIZE_MAX);
    }
}

// Convert all of str to a length's floating type, stored at dest if not null.
_Bool _Stdio_ConvertReal(const char *str, int length, void *dest)
{
    char *end;
    float single = 0;
    double real = 0;
    long double extended = 0;

    if (length == _STDIO_SCAN_LENGTH_LONG) {
        real = strtod(str, &end);
    } else if (length == _STDIO_SCAN_LENGTH_LDOUBLE) {
        extended = strtold(str, &end);
    } else {
        single = strtof(str, &end);
    }
    if (end == str || *end != '\0') {
        return 0;
    }
    if (dest != NULL && length == _STDIO_SCAN_LENGTH_LONG) {
        *(double *) dest = real;
    } else if (dest != NULL && length == _STDIO_SCAN_LENGTH_LDOUBLE) {
        *(long double *) dest = extended;
    } else if (dest != NULL) {
        *(float *) dest = single;
    }
    return 1;
}

// Decode ch as a character's next byte, storing the character if ch ends it.
void _Stdio_DecodeByte(_Stdio_Source *source, _Stdio_Wide *wide, int ch)
{
    _Stdio_WcharType wc;

    wide->sw_byte[wide->sw_count++] = (char) ch;
    if (mbtowc(&wc, wide->sw_byte, wide->sw_count) >= 0) {
        if (wide->sw_dest != NULL) {
            *wide->sw_dest++ = wc;
        }
        wide->sw_count = 0;
    } else if (wide->sw_count >= MB_CUR_MAX) {
        source->so_failure = _STDIO_FAILURE_INPUT;
    }
}

// End a directive's wide characters, and return whether every one was whole.
_Bool _Stdio_EndWide(_Stdio_Source *source, const _Stdio_Wide *wide)
{
    if (source->so_failure == _STDIO_FAILURE_NONE && wide->sw_count > 0) {
        source->so_failure = _STDIO_FAILURE_INPUT;
        errno = EILSEQ;
    }
    return source->so_failure == _STDIO_FAILURE_NONE;
}

// Scan the white space the source is at, none being enough.
void _Stdio_ScanSpace(_Stdio_Source *source)
{
    while (isspace(_Stdio_PeekSource(source))) {
        _Stdio_AdvanceSource(source);
    }
}

// Scan len characters that should be text's, stopping at one that is not.
void _Stdio_ScanOrdinary(_Stdio_Source *source, const char *text, size_t len)
{
    int ch;

    for (size_t i = 0; i < len; i++) {
        ch = _Stdio_PeekSource(source);
        if (ch != (unsigned char) text[i]) {
            source->so_failure = ch == EOF ? _STDIO_FAILURE_INPUT : _STDIO_FAILURE_MATCHING;
            return;
        }
        _Stdio_AdvanceSource(source);
    }
}

// Scan a percent sign, after any white space.
void _Stdio_ScanPercent(_Stdio_Source *source)
{
    _Stdio_ScanSpace(source);
    _Stdio_ScanOrdinary(source, "%", 1);
}

// Scan width characters into dest, one if no width, with no null after them.
void _Stdio_ScanChar(_Stdio_Source *source, int width, char *dest)
{
    int ch;
    size_t count = width == _STDIO_SCAN_WIDTH_NONE ? 1 : (size_t) width;

    for (size_t i = 0; i < count; i++) {
        ch = _Stdio_PeekSource(source);
        if (ch == EOF) {
            source->so_failure = _STDIO_FAILURE_INPUT;
            return;
        }
        if (dest != NULL) {
            dest[i] = (char) ch;
        }
        _Stdio_AdvanceSource(source);
    }
    if (dest != NULL) {
        source->so_stored++;
    }
}

// Scan a word, after any white space, into dest as a string, width its bound.
void _Stdio_ScanString(_Stdio_Source *source, int width, char *dest)
{
    int ch;
    size_t len = 0;
    size_t max = width == _STDIO_SCAN_WIDTH_NONE ? SIZE_MAX : (size_t) width;

    _Stdio_ScanSpace(source);
    while (len < max && (ch = _Stdio_PeekSource(source)) != EOF && ! isspace(ch)) {
        if (dest != NULL) {
            dest[len] = (char) ch;
        }
        _Stdio_AdvanceSource(source);
        len++;
    }
    if (len == 0) {
        source->so_failure = _STDIO_FAILURE_INPUT;
        return;
    }
    if (dest != NULL) {
        dest[len] = '\0';
        source->so_stored++;
    }
}

// Scan the characters a scanset takes into dest as a string, width its bound.
void _Stdio_ScanSet(_Stdio_Source *source, int width, char *dest, const char *set, size_t count, _Stdio_Accept accept)
{
    int ch = 0;
    size_t len = 0;
    size_t max = width == _STDIO_SCAN_WIDTH_NONE ? SIZE_MAX : (size_t) width;

    while (len < max && (ch = _Stdio_PeekSource(source)) != EOF && _Stdio_IsInSet(set, count, ch) == (accept == _STDIO_ACCEPT_LISTED)) {
        if (dest != NULL) {
            dest[len] = (char) ch;
        }
        _Stdio_AdvanceSource(source);
        len++;
    }
    if (len == 0) {
        source->so_failure = ch == EOF ? _STDIO_FAILURE_INPUT : _STDIO_FAILURE_MATCHING;
        return;
    }
    if (dest != NULL) {
        dest[len] = '\0';
        source->so_stored++;
    }
}

// Scan nothing: store the characters read so far at dest, as a length's type.
void _Stdio_ScanCount(_Stdio_Source *source, int length, void *dest)
{
    _Stdio_StoreInteger(dest, length, source->so_count);
}

// Scan a decimal integer, after any white space, into dest as a length's type.
void _Stdio_ScanDecimal(_Stdio_Source *source, int width, int length, void *dest)
{
    char item[_STDIO_SCAN_ITEM];

    _Stdio_ScanSpace(source);
    if (_Stdio_ReadInteger(source, width, _STDIO_SCAN_BASE_DECIMAL, item) && dest != NULL) {
        _Stdio_StoreInteger(dest, length, (unsigned long long) strtoll(item, NULL, _STDIO_SCAN_BASE_DECIMAL));
        source->so_stored++;
    }
}

// Scan an integer of the base its prefix names, after any white space.
void _Stdio_ScanInteger(_Stdio_Source *source, int width, int length, void *dest)
{
    char item[_STDIO_SCAN_ITEM];

    _Stdio_ScanSpace(source);
    if (_Stdio_ReadInteger(source, width, _STDIO_SCAN_BASE_PREFIX, item) && dest != NULL) {
        _Stdio_StoreInteger(dest, length, (unsigned long long) strtoll(item, NULL, _STDIO_SCAN_BASE_PREFIX));
        source->so_stored++;
    }
}

// Scan an octal integer, after any white space, into dest as a length's type.
void _Stdio_ScanOctal(_Stdio_Source *source, int width, int length, void *dest)
{
    char item[_STDIO_SCAN_ITEM];

    _Stdio_ScanSpace(source);
    if (_Stdio_ReadInteger(source, width, _STDIO_SCAN_BASE_OCTAL, item) && dest != NULL) {
        _Stdio_StoreInteger(dest, length, strtoull(item, NULL, _STDIO_SCAN_BASE_OCTAL));
        source->so_stored++;
    }
}

// Scan a decimal integer, after any white space, into dest as unsigned.
void _Stdio_ScanUnsigned(_Stdio_Source *source, int width, int length, void *dest)
{
    char item[_STDIO_SCAN_ITEM];

    _Stdio_ScanSpace(source);
    if (_Stdio_ReadInteger(source, width, _STDIO_SCAN_BASE_DECIMAL, item) && dest != NULL) {
        _Stdio_StoreInteger(dest, length, strtoull(item, NULL, _STDIO_SCAN_BASE_DECIMAL));
        source->so_stored++;
    }
}

// Scan a hex integer, 0x before it or not, after any white space, into dest.
void _Stdio_ScanHex(_Stdio_Source *source, int width, int length, void *dest)
{
    char item[_STDIO_SCAN_ITEM];

    _Stdio_ScanSpace(source);
    if (_Stdio_ReadInteger(source, width, _STDIO_SCAN_BASE_HEX, item) && dest != NULL) {
        _Stdio_StoreInteger(dest, length, strtoull(item, NULL, _STDIO_SCAN_BASE_HEX));
        source->so_stored++;
    }
}

// Scan a pointer as %p writes one, (nil) or hex, after any white space.
void _Stdio_ScanPointer(_Stdio_Source *source, int width, void *dest)
{
    char item[_STDIO_SCAN_ITEM];
    const char *nil = _STDIO_SCAN_NIL;
    unsigned long long value = 0;

    _Stdio_ScanSpace(source);
    if (_Stdio_PeekSource(source) == nil[0] && (width == _STDIO_SCAN_WIDTH_NONE || width >= _STDIO_SCAN_NIL_WIDTH)) {
        // Phase: scan a null pointer, in letters of either case
        for (size_t i = 0; nil[i] != '\0'; i++) {
            if (tolower(_Stdio_PeekSource(source)) != nil[i]) {
                source->so_failure = _STDIO_FAILURE_MATCHING;
                return;
            }
            _Stdio_AdvanceSource(source);
        }
    } else {
        // Phase: scan an address
        if (! _Stdio_ReadInteger(source, width, _STDIO_SCAN_BASE_HEX, item)) {
            return;
        }
        value = strtoull(item, NULL, _STDIO_SCAN_BASE_HEX);
    }
    if (dest != NULL) {
        *(void **) dest = (void *) (uintptr_t) value;
        source->so_stored++;
    }
}

// Scan a floating number, after any white space, into dest as a length's type.
void _Stdio_ScanReal(_Stdio_Source *source, int width, int length, void *dest)
{
    _Stdio_Item item;

    _Stdio_ScanSpace(source);
    _Stdio_ReadReal(source, width, &item);
    if (source->so_failure == _STDIO_FAILURE_NONE && item.si_len == 0) {
        source->so_failure = _Stdio_PeekSource(source) == EOF ? _STDIO_FAILURE_INPUT : _STDIO_FAILURE_MATCHING;
    } else if (source->so_failure == _STDIO_FAILURE_NONE && ! _Stdio_ConvertReal(item.si_text, length, dest)) {
        source->so_failure = _STDIO_FAILURE_MATCHING;
    } else if (source->so_failure == _STDIO_FAILURE_NONE && dest != NULL) {
        source->so_stored++;
    }
    free(item.si_text);
}

// Scan width bytes, one if no width, into dest as wide characters, no null.
void _Stdio_ScanWideChar(_Stdio_Source *source, int width, _Stdio_WcharType *dest)
{
    int ch;
    _Stdio_Wide wide;
    size_t count = width == _STDIO_SCAN_WIDTH_NONE ? 1 : (size_t) width;

    wide.sw_dest = dest;
    wide.sw_count = 0;
    for (size_t i = 0; i < count && source->so_failure == _STDIO_FAILURE_NONE; i++) {
        ch = _Stdio_PeekSource(source);
        if (ch == EOF) {
            source->so_failure = _STDIO_FAILURE_INPUT;
            return;
        }
        _Stdio_DecodeByte(source, &wide, ch);
        _Stdio_AdvanceSource(source);
    }
    if (_Stdio_EndWide(source, &wide) && dest != NULL) {
        source->so_stored++;
    }
}

// Scan a word, after any white space, into dest as a wide string, width bytes.
void _Stdio_ScanWideString(_Stdio_Source *source, int width, _Stdio_WcharType *dest)
{
    int ch;
    _Stdio_Wide wide;
    size_t len = 0;
    size_t max = width == _STDIO_SCAN_WIDTH_NONE ? SIZE_MAX : (size_t) width;

    wide.sw_dest = dest;
    wide.sw_count = 0;
    _Stdio_ScanSpace(source);
    while (len < max && source->so_failure == _STDIO_FAILURE_NONE && (ch = _Stdio_PeekSource(source)) != EOF && ! isspace(ch)) {
        _Stdio_DecodeByte(source, &wide, ch);
        _Stdio_AdvanceSource(source);
        len++;
    }
    if (len == 0) {
        source->so_failure = _STDIO_FAILURE_INPUT;
        return;
    }
    if (_Stdio_EndWide(source, &wide) && dest != NULL) {
        *wide.sw_dest = 0;
        source->so_stored++;
    }
}

// Scan the bytes a scanset takes into dest as a wide string, width bytes.
void _Stdio_ScanWideSet(_Stdio_Source *source, int width, _Stdio_WcharType *dest, const char *set, size_t count, _Stdio_Accept accept)
{
    int ch = 0;
    _Stdio_Wide wide;
    size_t len = 0;
    size_t max = width == _STDIO_SCAN_WIDTH_NONE ? SIZE_MAX : (size_t) width;

    wide.sw_dest = dest;
    wide.sw_count = 0;
    while (len < max && source->so_failure == _STDIO_FAILURE_NONE && (ch = _Stdio_PeekSource(source)) != EOF && _Stdio_IsInSet(set, count, ch) == (accept == _STDIO_ACCEPT_LISTED)) {
        _Stdio_DecodeByte(source, &wide, ch);
        _Stdio_AdvanceSource(source);
        len++;
    }
    if (len == 0) {
        source->so_failure = ch == EOF ? _STDIO_FAILURE_INPUT : _STDIO_FAILURE_MATCHING;
        return;
    }
    if (_Stdio_EndWide(source, &wide) && dest != NULL) {
        *wide.sw_dest = 0;
        source->so_stored++;
    }
}

// Scan the source as the format says, each directive storing at its argument.
int _Stdio_ScanFormat(_Stdio_Source *source, const char *format, _Stdio_VaList args)
{
    int width;
    char *str;
    int length;
    size_t len;
    void *dest;
    size_t count;
    va_list list;
    const char *set;
    _Stdio_Accept accept;
    _Stdio_WcharType *wstr;

    va_copy(list, args);
    for (size_t i = 0; format[i] != '\0' && source->so_failure == _STDIO_FAILURE_NONE; i += len) {
        if (_Stdio_MatchScanSpace(format + i, &len)) {
            _Stdio_ScanSpace(source);
        } else if (_Stdio_MatchScanOrdinary(format + i, &len)) {
            _Stdio_ScanOrdinary(source, format + i, len);
        } else if (_Stdio_MatchScanPercent(format + i, &len)) {
            _Stdio_ScanPercent(source);
        } else if (_Stdio_MatchScanChar(format + i, &list, &len, &width, &str)) {
            _Stdio_ScanChar(source, width, str);
        } else if (_Stdio_MatchScanString(format + i, &list, &len, &width, &str)) {
            _Stdio_ScanString(source, width, str);
        } else if (_Stdio_MatchScanSet(format + i, &list, &len, &width, &str, &set, &count, &accept)) {
            _Stdio_ScanSet(source, width, str, set, count, accept);
        } else if (_Stdio_MatchScanCount(format + i, &list, &len, &length, &dest)) {
            _Stdio_ScanCount(source, length, dest);
        } else if (_Stdio_MatchScanDecimal(format + i, &list, &len, &width, &length, &dest)) {
            _Stdio_ScanDecimal(source, width, length, dest);
        } else if (_Stdio_MatchScanInteger(format + i, &list, &len, &width, &length, &dest)) {
            _Stdio_ScanInteger(source, width, length, dest);
        } else if (_Stdio_MatchScanOctal(format + i, &list, &len, &width, &length, &dest)) {
            _Stdio_ScanOctal(source, width, length, dest);
        } else if (_Stdio_MatchScanUnsigned(format + i, &list, &len, &width, &length, &dest)) {
            _Stdio_ScanUnsigned(source, width, length, dest);
        } else if (_Stdio_MatchScanHex(format + i, &list, &len, &width, &length, &dest)) {
            _Stdio_ScanHex(source, width, length, dest);
        } else if (_Stdio_MatchScanPointer(format + i, &list, &len, &width, &dest)) {
            _Stdio_ScanPointer(source, width, dest);
        } else if (_Stdio_MatchScanReal(format + i, &list, &len, &width, &length, &dest)) {
            _Stdio_ScanReal(source, width, length, dest);
        } else if (_Stdio_MatchScanWideChar(format + i, &list, &len, &width, &wstr)) {
            _Stdio_ScanWideChar(source, width, wstr);
        } else if (_Stdio_MatchScanWideString(format + i, &list, &len, &width, &wstr)) {
            _Stdio_ScanWideString(source, width, wstr);
        } else if (_Stdio_MatchScanWideSet(format + i, &list, &len, &width, &wstr, &set, &count, &accept)) {
            _Stdio_ScanWideSet(source, width, wstr, set, count, accept);
        } else {
            len = 0;
            source->so_failure = _STDIO_FAILURE_MATCHING;
        }
    }
    va_end(list);
    return _Stdio_EndSource(source);
}

// Count the bytes the stream took from its file and has yet to give out.
long _Stdio_CountReadAhead(FILE *stream)
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

// Seek the stream to offset from the origin whence, and empty its buffer.
int _Stdio_SeekStream(FILE *stream, long offset, int whence)
{
    long pos;

    if (_Stdio_FlushBuffer(stream) != 0) {
        return EOF;
    }
    if (whence == _SYS_SEEK_CUR) {
        offset -= _Stdio_CountReadAhead(stream);
    }
    pos = _Sys_InvokeLseek(stream->sf_fd, offset, whence);
    if (pos < 0) {
        errno = (int) -pos;
        return EOF;
    }
    stream->sf_flags &= ~_STDIO_BACK;
    stream->sf_head = 0;
    stream->sf_tail = 0;
    return 0;
}

// Discard what the stream read ahead, moving its file back over it.
void _Stdio_DiscardReadAhead(FILE *stream)
{
    if (_Stdio_CountReadAhead(stream) > 0) {
        _Stdio_SeekStream(stream, 0, _SYS_SEEK_CUR);
    }
}

// Make a number that differs from run to run, by the clock and the stack.
unsigned long _Stdio_MakeTempSeed(void)
{
    _Sys_Timespec now = {0, 0};

    _Sys_InvokeClockGettime(_SYS_CLOCK_REALTIME, &now);
    return (unsigned long) now.tv_sec ^ (unsigned long) now.tv_nsec ^ (unsigned long) &now;
}

// Write the next temporary name, one no call before wrote, into str.
void _Stdio_WriteTempName(char *str)
{
    int k;
    unsigned long number;
    size_t len = sizeof(_STDIO_TEMP_PREFIX) - 1;

    if (! _Stdio_TempSeeded) {
        _Stdio_TempNumber = _Stdio_MakeTempSeed() % _STDIO_TEMP_NAMES;
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
    long status = _Sys_InvokeUnlink(filename);

    if (status == -_SYS_EISDIR) {
        status = _Sys_InvokeRmdir(filename);
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
    long status = _Sys_InvokeRename(old, new);

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
    FILE *stream = _Stdio_MakeStream();
    int flags = _SYS_O_RDWR | _SYS_O_CREAT | _SYS_O_EXCL;

    if (stream == NULL) {
        return NULL;
    }
    for (tries = 0; tries < TMP_MAX && fd == -_SYS_EEXIST; tries++) {
        _Stdio_WriteTempName(name);
        fd = _Sys_InvokeOpen(name, flags, _SYS_S_IRUSR | _SYS_S_IWUSR);
    }
    if (fd < 0) {
        errno = (int) -fd;
        _Stdio_FreeStream(stream);
        return NULL;
    }
    _Sys_InvokeUnlink(name);
    _Stdio_AttachDescriptor(stream, (int) fd, flags);
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
        _Stdio_WriteTempName(str);
        if (_Sys_InvokeLstat(str, &info) == -_SYS_ENOENT) {
            return str;
        }
    }
    return NULL;
}

// Flush the stream, close its file and end the stream.
int fclose(FILE *stream)
{
    int result = _Stdio_ResetStream(stream);
    long status = _Sys_InvokeClose(stream->sf_fd);

    if (status < 0) {
        errno = (int) -status;
        result = EOF;
    }
    _Stdio_FreeStream(stream);
    return result;
}

// Write what the stream has to write, every stream's for NULL, or unread it.
int fflush(FILE *stream)
{
    if (stream == NULL) {
        return _Stdio_FlushStreams();
    }
    _Stdio_DiscardReadAhead(stream);
    return _Stdio_FlushBuffer(stream);
}

// Open the file filename as the mode says, and return a stream for it.
FILE *fopen(const char *restrict filename, const char *restrict mode)
{
    FILE *stream = _Stdio_MakeStream();

    if (stream != NULL && _Stdio_OpenFile(stream, filename, mode) != 0) {
        _Stdio_FreeStream(stream);
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
    _Stdio_ResetStream(stream);
    if (_Stdio_OpenFile(stream, filename, mode) != 0) {
        _Sys_InvokeClose(fd);
        _Stdio_FreeStream(stream);
        return NULL;
    }
    if (_Sys_InvokeDup2(stream->sf_fd, fd) == fd) {
        _Sys_InvokeClose(stream->sf_fd);
        stream->sf_fd = fd;
    } else {
        _Sys_InvokeClose(fd);
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
    if ((buf != NULL && size == 0) || _Stdio_FlushBuffer(stream) != 0) {
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

// Read the stream as the format says, storing at the arguments.
int fscanf(FILE *restrict stream, const char *restrict format, ...)
{
    int result;
    va_list args;

    va_start(args, format);
    result = vfscanf(stream, format, args);
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

// Read standard input as the format says, storing at the arguments.
int scanf(const char *restrict format, ...)
{
    int result;
    va_list args;

    va_start(args, format);
    result = vfscanf(stdin, format, args);
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

// Read the string str as the format says, storing at the arguments.
int sscanf(const char *restrict str, const char *restrict format, ...)
{
    int result;
    va_list args;

    va_start(args, format);
    result = vsscanf(str, format, args);
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

    if (_Stdio_LendBuffer(stream, lent, sizeof(lent)) != 0) {
        return EOF;
    }
    result = _Stdio_EmitFormat(&sink, format, arg);
    return _Stdio_ReclaimBuffer(stream) == 0 ? result : EOF;
}

// Read the stream as the format says, storing at the list's arguments.
int vfscanf(FILE *restrict stream, const char *restrict format, _Stdio_VaList arg)
{
    _Stdio_Source source = {
        .so_stream = stream
    };

    if (_Stdio_PrepareInput(stream) != 0) {
        return EOF;
    }
    return _Stdio_ScanFormat(&source, format, arg);
}

// Write the list's arguments to standard output as the format says.
int vprintf(const char *restrict format, _Stdio_VaList arg)
{
    return vfprintf(stdout, format, arg);
}

// Read standard input as the format says, storing at the list's arguments.
int vscanf(const char *restrict format, _Stdio_VaList arg)
{
    return vfscanf(stdin, format, arg);
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

// Read the string str as the format says, storing at the list's arguments.
int vsscanf(const char *restrict str, const char *restrict format, _Stdio_VaList arg)
{
    _Stdio_Source source = {
        .so_str = str
    };

    return _Stdio_ScanFormat(&source, format, arg);
}

// Read the stream's next character, as an unsigned char, or EOF at its end.
int fgetc(FILE *stream)
{
    unsigned char byte;

    return _Stdio_GetBytes(stream, &byte, 1) == 1 ? byte : EOF;
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

    return _Stdio_PutBytes(stream, &byte, 1) == 1 ? byte : EOF;
}

// Write the string str to the stream, and return 1 as glibc's does.
int fputs(const char *restrict str, FILE *restrict stream)
{
    size_t len = strlen(str);

    return _Stdio_PutBytes(stream, str, len) == len ? 1 : EOF;
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

    if (_Stdio_PutBytes(stdout, str, len) != len || _Stdio_PutBytes(stdout, "\n", 1) != 1) {
        return EOF;
    }
    return len < INT_MAX ? (int) len + 1 : INT_MAX;
}

// Push the character ch back for the stream's next read, one at a time.
int ungetc(int ch, FILE *stream)
{
    if (ch == EOF || (stream->sf_flags & _STDIO_BACK) || _Stdio_PrepareInput(stream) != 0) {
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
    return _Stdio_GetBytes(stream, ptr, size * nmemb) / size;
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
    return _Stdio_PutBytes(stream, ptr, size * nmemb) / size;
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
    if (_Stdio_SeekStream(stream, offset, whence) != 0) {
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

    if (_Stdio_FlushBuffer(stream) != 0) {
        return EOF;
    }
    pos = _Sys_InvokeLseek(stream->sf_fd, 0, _SYS_SEEK_CUR);
    if (pos < 0) {
        errno = (int) -pos;
        return EOF;
    }
    return pos - _Stdio_CountReadAhead(stream);
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

    _Stdio_LendBuffer(stderr, lent, sizeof(lent));
    if (str != NULL && str[0] != '\0') {
        fputs(str, stderr);
        fputs(": ", stderr);
    }
    fputs(text, stderr);
    fputs("\n", stderr);
    _Stdio_ReclaimBuffer(stderr);
}
