/*
 * C source file for streams and the files under them.
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
#include <_file.h>

// The files' calls, the terminal's settings and the standard descriptors.
#include <_sys.h>

// The error number of the last failed call.
#include <_err.h>

// The hook exit flushes the streams by.
#include <_crt.h>

// The streams and the buffers a program opens.
#include <_mem.h>

// Copying to and from a buffer, finding its new-lines and matching a mode.
#include <_str.h>

// Standard input, read-only, its buffering picked at its first use.
_File_Stream _File_Stdin = {
    .fs_fd    = _SYS_STDIN_FILENO,
    .fs_flags = _FILE_READ,
    .fs_mode  = _FILE_AUTO,
    .fs_next  = &_File_Stdout
};

// Standard output, write-only, its buffering picked at its first use.
_File_Stream _File_Stdout = {
    .fs_fd    = _SYS_STDOUT_FILENO,
    .fs_flags = _FILE_WRITE,
    .fs_mode  = _FILE_AUTO,
    .fs_next  = &_File_Stderr
};

// Standard error, write-only and unbuffered, as glibc's.
_File_Stream _File_Stderr = {
    .fs_fd    = _SYS_STDERR_FILENO,
    .fs_flags = _FILE_WRITE,
    .fs_mode  = _FILE_NONE
};

// The open streams, the standard ones at first.
_File_Stream *_File_Streams = &_File_Stdin;

// The number of the temporary name made last, once a seed has begun them.
unsigned long _File_TempNumber;
_Bool _File_TempSeeded;

// The name tmpnam returns when it is given no array.
char _File_TempBuffer[_FILE_TEMP_SIZE];

// Make a stream with no file, first among the open ones.
_File_Stream *_File_MakeStream(void)
{
    _File_Stream *stream = _Mem_AllocateObjects(1, sizeof(_File_Stream));

    if (stream != _FILE_NULL) {
        stream->fs_fd = _FILE_NOFILE;
        stream->fs_flags = _FILE_OWNFILE;
        stream->fs_mode = _FILE_AUTO;
        stream->fs_next = _File_Streams;
        _File_Streams = stream;
    }
    return stream;
}

// Return whether mode is one of the fifteen fopen takes.
_Bool _File_IsMode(const char *mode)
{
    const char *rest = mode + 1;

    if (mode[0] != 'r' && mode[0] != 'w' && mode[0] != 'a') {
        return 0;
    }
    return rest[0] == '\0' || _Str_CompareStrings(rest, "b") == 0 || _Str_CompareStrings(rest, "+") == 0 || _Str_CompareStrings(rest, "b+") == 0 || _Str_CompareStrings(rest, "+b") == 0;
}

// Convert fopen's mode into the flags open takes.
int _File_ToMode(const char *mode)
{
    int flags = mode[0] == 'r' ? _SYS_O_RDONLY : _SYS_O_WRONLY | _SYS_O_CREAT;

    if (_Str_FindChar(mode, '+') != _FILE_NULL) {
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
void _File_AttachDescriptor(_File_Stream *stream, int fd, int flags)
{
    stream->fs_fd = fd;
    if ((flags & _SYS_O_ACCMODE) != _SYS_O_WRONLY) {
        stream->fs_flags |= _FILE_READ;
    }
    if ((flags & _SYS_O_ACCMODE) != _SYS_O_RDONLY) {
        stream->fs_flags |= _FILE_WRITE;
    }
}

// Open the file filename for the stream, to read or write as the mode says.
int _File_OpenFile(_File_Stream *stream, const char *filename, const char *mode)
{
    long fd;
    int flags;

    if (! _File_IsMode(mode)) {
        _Err_Errno = _SYS_EINVAL;
        return _FILE_END;
    }
    flags = _File_ToMode(mode);
    fd = _Sys_InvokeOpen(filename, flags, _SYS_DEFFILEMODE);
    if (fd < 0) {
        _Err_Errno = (int) -fd;
        return _FILE_END;
    }
    _File_AttachDescriptor(stream, (int) fd, flags);
    return 0;
}

// Reset the stream: flush it and take its buffer, indicators and access.
int _File_ResetStream(_File_Stream *stream)
{
    int result = _File_FlushBuffer(stream);

    if (stream->fs_flags & _FILE_OWNBUF) {
        _Mem_FreeMemory(stream->fs_buf);
    }
    stream->fs_flags &= _FILE_OWNFILE;
    stream->fs_state = _FILE_IDLE;
    stream->fs_mode = _FILE_AUTO;
    stream->fs_buf = _FILE_NULL;
    stream->fs_size = 0;
    stream->fs_head = 0;
    stream->fs_tail = 0;
    stream->fs_backs = 0;
    stream->fs_orient = _FILE_ORIENT_NONE;
    return result;
}

// Free the stream if fopen made it, and take it off the open ones.
void _File_FreeStream(_File_Stream *stream)
{
    _File_Stream **iter = &_File_Streams;

    while (*iter != _FILE_NULL && *iter != stream) {
        iter = &(*iter)->fs_next;
    }
    if (*iter == stream) {
        *iter = stream->fs_next;
    }
    stream->fs_fd = _FILE_NOFILE;
    stream->fs_next = _FILE_NULL;
    if (stream->fs_flags & _FILE_OWNFILE) {
        _Mem_FreeMemory(stream);
    }
}

// Orient a stream that has no orientation as mode's sign says, and give its.
int _File_OrientStream(_File_Stream *stream, int mode)
{
    if (stream->fs_orient == _FILE_ORIENT_NONE && mode != 0) {
        stream->fs_orient = mode > 0 ? _FILE_ORIENT_WIDE : _FILE_ORIENT_BYTE;
    }
    return stream->fs_orient;
}

// Set up the stream's buffering and buffer at first use, none without memory.
void _File_SetupBuffer(_File_Stream *stream)
{
    unsigned long size = stream->fs_size != 0 ? stream->fs_size : _FILE_BUFFER;

    if (stream->fs_flags & _FILE_SETUP) {
        return;
    }
    if (stream->fs_mode == _FILE_AUTO) {
        stream->fs_mode = _File_IsTerminal(stream->fs_fd) ? _FILE_LINE : _FILE_FULL;
    }
    if (stream->fs_mode != _FILE_NONE && stream->fs_buf == _FILE_NULL) {
        stream->fs_buf = _Mem_AllocateMemory(size);
        stream->fs_size = size;
        stream->fs_flags |= _FILE_OWNBUF;
        if (stream->fs_buf == _FILE_NULL) {
            stream->fs_mode = _FILE_NONE;
            stream->fs_flags &= ~_FILE_OWNBUF;
        }
    }
    if (stream->fs_mode == _FILE_NONE) {
        stream->fs_buf = &stream->fs_byte;
        stream->fs_size = 1;
    }
    stream->fs_flags |= _FILE_SETUP;
}

// Return whether the descriptor fd is a terminal, which answers TCGETS.
_Bool _File_IsTerminal(int fd)
{
    _Sys_Termios term;

    return _Sys_InvokeIoctl(fd, _SYS_TCGETS, &term) == 0;
}

// Lend an unbuffered stream buf for a call, the stream readied for output.
int _File_LendBuffer(_File_Stream *stream, unsigned char *buf, unsigned long size)
{
    if (_File_PrepareOutput(stream) != 0) {
        return _FILE_END;
    }
    if (stream->fs_mode == _FILE_NONE) {
        stream->fs_buf = buf;
        stream->fs_size = size;
    }
    return 0;
}

// Reclaim a lent buffer: write what it holds, and give the stream its byte.
int _File_ReclaimBuffer(_File_Stream *stream)
{
    int result = 0;

    if (stream->fs_mode == _FILE_NONE && stream->fs_state == _FILE_WRITING) {
        result = _File_FlushBuffer(stream);
        stream->fs_buf = &stream->fs_byte;
        stream->fs_size = 1;
    }
    return result;
}

// Prepare the stream for input, or set its error indicator if it is write-only.
int _File_PrepareInput(_File_Stream *stream)
{
    _File_OrientStream(stream, _FILE_ORIENT_BYTE);
    if (! (stream->fs_flags & _FILE_READ)) {
        stream->fs_flags |= _FILE_ERROR;
        _Err_Errno = _SYS_EBADF;
        return _FILE_END;
    }
    if (_File_FlushBuffer(stream) != 0) {
        return _FILE_END;
    }
    _File_SetupBuffer(stream);
    stream->fs_state = _FILE_READING;
    return 0;
}

// Get up to len bytes from the stream into data, and return how many came.
unsigned long _File_GetBytes(_File_Stream *stream, void *data, unsigned long len)
{
    unsigned long done = 0;
    unsigned char *bytes = data;

    if (_File_PrepareInput(stream) != 0) {
        return 0;
    }
    while (done < len && stream->fs_backs > 0) {
        bytes[done++] = stream->fs_back[--stream->fs_backs];
    }
    while (done < len) {
        unsigned long part = len - done;

        if (stream->fs_head == stream->fs_tail && part >= stream->fs_size) {
            part = _File_ReadDescriptor(stream, bytes + done, part);
        } else {
            part = _File_ReadBuffer(stream, bytes + done, part);
        }
        if (part == 0) {
            return done;
        }
        done += part;
    }
    return done;
}

// Read up to len bytes of the stream's buffer into data, refilled if empty.
unsigned long _File_ReadBuffer(_File_Stream *stream, unsigned char *data, unsigned long len)
{
    if (stream->fs_head == stream->fs_tail) {
        stream->fs_head = 0;
        stream->fs_tail = _File_ReadDescriptor(stream, stream->fs_buf, stream->fs_size);
    }
    if (len > stream->fs_tail - stream->fs_head) {
        len = stream->fs_tail - stream->fs_head;
    }
    _Str_CopyBytes(data, stream->fs_buf + stream->fs_head, len);
    stream->fs_head += len;
    return len;
}

// Read up to len bytes from the stream's descriptor, none once at its end.
unsigned long _File_ReadDescriptor(_File_Stream *stream, unsigned char *data, unsigned long len)
{
    long done;

    if (stream->fs_flags & _FILE_EOF) {
        return 0;
    }
    if (stream->fs_mode != _FILE_FULL) {
        _File_FlushLines();
    }
    done = _Sys_InvokeRead(stream->fs_fd, data, len);
    if (done < 0) {
        _Err_Errno = (int) -done;
        stream->fs_flags |= _FILE_ERROR;
        return 0;
    }
    if (done == 0) {
        stream->fs_flags |= _FILE_EOF;
    }
    return (unsigned long) done;
}

// Get the stream's next character, as an unsigned char, or _FILE_END.
int _File_GetChar(_File_Stream *stream)
{
    unsigned char byte;

    return _File_GetBytes(stream, &byte, 1) == 1 ? byte : _FILE_END;
}

// Peek at the stream's next character, left unread, or _FILE_END for none.
int _File_PeekChar(_File_Stream *stream)
{
    if (stream->fs_backs > 0) {
        return stream->fs_back[stream->fs_backs - 1];
    }
    if (stream->fs_head == stream->fs_tail) {
        stream->fs_head = 0;
        stream->fs_tail = _File_ReadDescriptor(stream, stream->fs_buf, stream->fs_size);
    }
    return stream->fs_head < stream->fs_tail ? stream->fs_buf[stream->fs_head] : _FILE_END;
}

// Skip the character peeked at, which is read then.
void _File_SkipChar(_File_Stream *stream)
{
    if (stream->fs_backs > 0) {
        stream->fs_backs--;
    } else {
        stream->fs_head++;
    }
}

// Push the character ch back for the stream's next read, or give _FILE_END.
int _File_UngetChar(_File_Stream *stream, int ch)
{
    if (ch == _FILE_END || stream->fs_backs == _FILE_BACKS || _File_PrepareInput(stream) != 0) {
        return _FILE_END;
    }
    stream->fs_back[stream->fs_backs++] = (unsigned char) ch;
    stream->fs_flags &= ~_FILE_EOF;
    return (unsigned char) ch;
}

// Get a line of at most max characters, its new-line among them, into str.
char *_File_GetLine(_File_Stream *stream, char *str, unsigned long max)
{
    int ch = 0;
    unsigned long len = 0;

    while (len < max && ch != '\n' && (ch = _File_GetChar(stream)) != _FILE_END) {
        str[len++] = (char) ch;
    }
    if (ch == _FILE_END && (len == 0 || ! (stream->fs_flags & _FILE_EOF))) {
        return _FILE_NULL;
    }
    str[len] = '\0';
    return str;
}

// Prepare the stream for output, or set its error indicator if it is read-only.
int _File_PrepareOutput(_File_Stream *stream)
{
    _File_OrientStream(stream, _FILE_ORIENT_BYTE);
    if (! (stream->fs_flags & _FILE_WRITE)) {
        stream->fs_flags |= _FILE_ERROR;
        _Err_Errno = _SYS_EBADF;
        return _FILE_END;
    }
    if (stream->fs_state == _FILE_READING) {
        _File_DiscardReadAhead(stream);
        stream->fs_backs = 0;
        stream->fs_head = 0;
        stream->fs_tail = 0;
    }
    _File_SetupBuffer(stream);
    stream->fs_state = _FILE_WRITING;
    return 0;
}

// Put len bytes of data in the stream's buffer, and return how many it took.
unsigned long _File_PutBytes(_File_Stream *stream, const void *data, unsigned long len)
{
    unsigned long done = 0;
    const unsigned char *bytes = data;

    if (_File_PrepareOutput(stream) != 0) {
        return 0;
    }
    _Crt_FlushStreams = _File_FlushStreams;
    while (done < len) {
        unsigned long part = len - done;

        if (stream->fs_tail == 0 && part >= stream->fs_size) {
            return _File_WriteDescriptor(stream, bytes + done, part) == 0 ? len : done;
        }
        if (part > stream->fs_size - stream->fs_tail) {
            part = stream->fs_size - stream->fs_tail;
        }
        _Str_CopyBytes(stream->fs_buf + stream->fs_tail, bytes + done, part);
        stream->fs_tail += part;
        done += part;
        if (stream->fs_tail == stream->fs_size && _File_FlushBuffer(stream) != 0) {
            return done - part;
        }
    }
    if (stream->fs_mode == _FILE_LINE && _Str_FindByte(data, '\n', len) != _FILE_NULL && _File_FlushBuffer(stream) != 0) {
        return 0;
    }
    return len;
}

// Write len bytes of data to the stream's descriptor, however many writes.
int _File_WriteDescriptor(_File_Stream *stream, const unsigned char *data, unsigned long len)
{
    while (len > 0) {
        long done = _Sys_InvokeWrite(stream->fs_fd, data, len);

        if (done <= 0) {
            _Err_Errno = done < 0 ? (int) -done : _SYS_EIO;
            stream->fs_flags |= _FILE_ERROR;
            return _FILE_END;
        }
        data += done;
        len -= (unsigned long) done;
    }
    return 0;
}

// Flush what waits in the stream's buffer, dropping it if the write fails.
int _File_FlushBuffer(_File_Stream *stream)
{
    unsigned long len = stream->fs_tail;

    if (stream->fs_state != _FILE_WRITING || len == 0) {
        return 0;
    }
    stream->fs_tail = 0;
    return _File_WriteDescriptor(stream, stream->fs_buf, len);
}

// Flush every open stream, and return _FILE_END if any failed.
int _File_FlushStreams(void)
{
    _File_Stream *stream;
    int result = 0;

    for (stream = _File_Streams; stream != _FILE_NULL; stream = stream->fs_next) {
        if (_File_FlushBuffer(stream) != 0) {
            result = _FILE_END;
        }
    }
    return result;
}

// Flush every line-buffered stream, as a read from the host calls for.
void _File_FlushLines(void)
{
    _File_Stream *stream;

    for (stream = _File_Streams; stream != _FILE_NULL; stream = stream->fs_next) {
        if (stream->fs_mode == _FILE_LINE) {
            _File_FlushBuffer(stream);
        }
    }
}

// Count the bytes the stream took from its file and has yet to give out.
long _File_CountReadAhead(_File_Stream *stream)
{
    long ahead = 0;

    if (stream->fs_state == _FILE_READING) {
        ahead = (long) (stream->fs_tail - stream->fs_head);
    }
    return ahead + (long) stream->fs_backs;
}

// Seek the stream to offset from the origin whence, and empty its buffer.
int _File_SeekStream(_File_Stream *stream, long offset, int whence)
{
    long pos;

    if (_File_FlushBuffer(stream) != 0) {
        return _FILE_END;
    }
    if (whence == _SYS_SEEK_CUR) {
        offset -= _File_CountReadAhead(stream);
    }
    pos = _Sys_InvokeLseek(stream->fs_fd, offset, whence);
    if (pos < 0) {
        _Err_Errno = (int) -pos;
        return _FILE_END;
    }
    stream->fs_backs = 0;
    stream->fs_head = 0;
    stream->fs_tail = 0;
    return 0;
}

// Discard what the stream read ahead, moving its file back over it.
void _File_DiscardReadAhead(_File_Stream *stream)
{
    if (_File_CountReadAhead(stream) > 0) {
        _File_SeekStream(stream, 0, _SYS_SEEK_CUR);
    }
}

// Make a number that differs from run to run, by the clock and the stack.
unsigned long _File_MakeTempSeed(void)
{
    _Sys_Timespec now = {0, 0};

    _Sys_InvokeClockGettime(_SYS_CLOCK_REALTIME, &now);
    return (unsigned long) now.tv_sec ^ (unsigned long) now.tv_nsec ^ (unsigned long) &now;
}

// Write the next temporary name, one no call before wrote, into str.
void _File_WriteTempName(char *str)
{
    int k;
    unsigned long number;
    unsigned long len = sizeof(_FILE_TEMP_PREFIX) - 1;

    if (! _File_TempSeeded) {
        _File_TempNumber = _File_MakeTempSeed() % _FILE_TEMP_NAMES;
        _File_TempSeeded = 1;
    }
    _File_TempNumber = (_File_TempNumber + _FILE_TEMP_STEP) % _FILE_TEMP_NAMES;
    number = _File_TempNumber;
    _Str_CopyBytes(str, _FILE_TEMP_PREFIX, len);
    for (k = 0; k < _FILE_TEMP_DIGITS; k++) {
        str[len++] = _FILE_TEMP_LETTERS[number % _FILE_TEMP_RADIX];
        number /= _FILE_TEMP_RADIX;
    }
    str[len] = '\0';
}
