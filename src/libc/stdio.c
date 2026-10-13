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

// The files' calls and the origins a stream seeks from.
#include <_sys.h>

// The engine that converts an output format's directives.
#include <_print.h>

// The engine that converts an input format's directives.
#include <_scan.h>

// The error a failed open, read, write or close reports.
#include <errno.h>

// The length of a string to write and the text of an error.
#include <string.h>

// The buffer a closed stream gives back.
#include <stdlib.h>

// The largest int puts returns, and the limits the engines' are held to.
#include <limits.h>

// The largest size fread and fwrite move, and the integers %jd and %p name.
#include <stdint.h>

// The arguments a format converts.
#include <stdarg.h>

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
    FILE *stream = _File_MakeStream();
    int flags = _SYS_O_RDWR | _SYS_O_CREAT | _SYS_O_EXCL;

    if (stream == NULL) {
        return NULL;
    }
    for (tries = 0; tries < TMP_MAX && fd == -_SYS_EEXIST; tries++) {
        _File_WriteTempName(name);
        fd = _Sys_InvokeOpen(name, flags, _SYS_S_IRUSR | _SYS_S_IWUSR);
    }
    if (fd < 0) {
        errno = (int) -fd;
        _File_FreeStream(stream);
        return NULL;
    }
    _Sys_InvokeUnlink(name);
    _File_AttachDescriptor(stream, (int) fd, flags);
    return stream;
}

// Write a name no file has into str, or into an array of its own for NULL.
char *tmpnam(char *str)
{
    int tries;
    _Sys_Stat info;

    if (str == NULL) {
        str = _File_TempBuffer;
    }
    for (tries = 0; tries < TMP_MAX; tries++) {
        _File_WriteTempName(str);
        if (_Sys_InvokeLstat(str, &info) == -_SYS_ENOENT) {
            return str;
        }
    }
    return NULL;
}

// Flush the stream, close its file and end the stream.
int fclose(FILE *stream)
{
    int result = _File_ResetStream(stream);
    long status = _Sys_InvokeClose(stream->fs_fd);

    if (status < 0) {
        errno = (int) -status;
        result = EOF;
    }
    _File_FreeStream(stream);
    return result;
}

// Write what the stream has to write, every stream's for NULL, or unread it.
int fflush(FILE *stream)
{
    if (stream == NULL) {
        return _File_FlushStreams();
    }
    _File_DiscardReadAhead(stream);
    return _File_FlushBuffer(stream);
}

// Open the file filename as the mode says, and return a stream for it.
FILE *fopen(const char *restrict filename, const char *restrict mode)
{
    FILE *stream = _File_MakeStream();

    if (stream != NULL && _File_OpenFile(stream, filename, mode) != 0) {
        _File_FreeStream(stream);
        return NULL;
    }
    return stream;
}

// Open the file filename as the mode says for the stream, on its descriptor.
FILE *freopen(const char *restrict filename, const char *restrict mode, FILE *restrict stream)
{
    int fd = stream->fs_fd;

    if (filename == NULL) {
        errno = _SYS_EINVAL;
        return NULL;
    }
    _File_ResetStream(stream);
    if (_File_OpenFile(stream, filename, mode) != 0) {
        _Sys_InvokeClose(fd);
        _File_FreeStream(stream);
        return NULL;
    }
    if (_Sys_InvokeDup2(stream->fs_fd, fd) == fd) {
        _Sys_InvokeClose(stream->fs_fd);
        stream->fs_fd = fd;
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
    if ((buf != NULL && size == 0) || _File_FlushBuffer(stream) != 0) {
        return EOF;
    }
    if (stream->fs_flags & _FILE_OWNBUF) {
        free(stream->fs_buf);
    }
    stream->fs_flags &= ~(_FILE_OWNBUF | _FILE_SETUP);
    stream->fs_mode = mode;
    stream->fs_buf = mode != _IONBF ? (unsigned char *) buf : NULL;
    stream->fs_size = size;
    stream->fs_head = 0;
    stream->fs_tail = 0;
    stream->fs_state = _FILE_IDLE;
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
    _Print_Sink sink = {
        .ps_stream = stream
    };

    if (_File_LendBuffer(stream, lent, sizeof(lent)) != 0) {
        return EOF;
    }
    result = _Print_WriteFormat(&sink, format, arg);
    return _File_ReclaimBuffer(stream) == 0 ? result : EOF;
}

// Read the stream as the format says, storing at the list's arguments.
int vfscanf(FILE *restrict stream, const char *restrict format, _Stdio_VaList arg)
{
    _Scan_Source source = {
        .ss_stream = stream
    };

    if (_File_PrepareInput(stream) != 0) {
        return EOF;
    }
    return _Scan_ReadFormat(&source, format, arg);
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
    _Print_Sink sink = {
        .ps_str  = str,
        .ps_size = n
    };

    return _Print_WriteFormat(&sink, format, arg);
}

// Write the list's arguments into str, however long, as the format says.
int vsprintf(char *restrict str, const char *restrict format, _Stdio_VaList arg)
{
    return vsnprintf(str, SIZE_MAX, format, arg);
}

// Read the string str as the format says, storing at the list's arguments.
int vsscanf(const char *restrict str, const char *restrict format, _Stdio_VaList arg)
{
    _Scan_Source source = {
        .ss_str = str
    };

    return _Scan_ReadFormat(&source, format, arg);
}

// Read the stream's next character, as an unsigned char, or EOF at its end.
int fgetc(FILE *stream)
{
    return _File_GetChar(stream);
}

// Read a line of at most n - 1 characters, its new-line kept, into str.
char *fgets(char *restrict str, int n, FILE *restrict stream)
{
    return n > 0 ? _File_GetLine(stream, str, (size_t) n - 1) : NULL;
}

// Write the character ch, as an unsigned char, to the stream.
int fputc(int ch, FILE *stream)
{
    unsigned char byte = (unsigned char) ch;

    return _File_PutBytes(stream, &byte, 1) == 1 ? byte : EOF;
}

// Write the string str to the stream, and return 1 as glibc's does.
int fputs(const char *restrict str, FILE *restrict stream)
{
    size_t len = strlen(str);

    return _File_PutBytes(stream, str, len) == len ? 1 : EOF;
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

    if (_File_GetLine(stdin, str, SIZE_MAX) == NULL) {
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

    if (_File_PutBytes(stdout, str, len) != len || _File_PutBytes(stdout, "\n", 1) != 1) {
        return EOF;
    }
    return len < INT_MAX ? (int) len + 1 : INT_MAX;
}

// Push the character ch back for the stream's next read.
int ungetc(int ch, FILE *stream)
{
    return _File_UngetChar(stream, ch);
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
    return _File_GetBytes(stream, ptr, size * nmemb) / size;
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
    return _File_PutBytes(stream, ptr, size * nmemb) / size;
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
    if (_File_SeekStream(stream, offset, whence) != 0) {
        return EOF;
    }
    stream->fs_flags &= ~_FILE_EOF;
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

    if (_File_FlushBuffer(stream) != 0) {
        return EOF;
    }
    pos = _Sys_InvokeLseek(stream->fs_fd, 0, _SYS_SEEK_CUR);
    if (pos < 0) {
        errno = (int) -pos;
        return EOF;
    }
    return pos - _File_CountReadAhead(stream);
}

// Move the stream to the start of its file, and clear its indicators.
void rewind(FILE *stream)
{
    fseek(stream, 0, SEEK_SET);
    stream->fs_flags &= ~_FILE_ERROR;
}

// Clear the stream's end-of-file and error indicators.
void clearerr(FILE *stream)
{
    stream->fs_flags &= ~(_FILE_EOF | _FILE_ERROR);
}

// Return nonzero if the stream's end-of-file indicator is set.
int feof(FILE *stream)
{
    return (stream->fs_flags & _FILE_EOF) != 0;
}

// Return nonzero if the stream's error indicator is set.
int ferror(FILE *stream)
{
    return (stream->fs_flags & _FILE_ERROR) != 0;
}

// Write errno's text and a new-line to standard error, after str if it has any.
void perror(const char *str)
{
    const char *text = strerror(errno);
    unsigned char lent[BUFSIZ];

    _File_LendBuffer(stderr, lent, sizeof(lent));
    if (str != NULL && str[0] != '\0') {
        fputs(str, stderr);
        fputs(": ", stderr);
    }
    fputs(text, stderr);
    fputs("\n", stderr);
    _File_ReclaimBuffer(stderr);
}
