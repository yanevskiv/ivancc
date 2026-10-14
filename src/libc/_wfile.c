/*
 * C source file for the wide characters of a stream.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivcc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivcc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivcc; see
 * the files LICENSE and COPYING.RUNTIME respectively.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

// Module header.
#include <_wfile.h>

// The error bytes that are no character report.
#include <_sys.h>
#include <_err.h>

// The encoding a stream's bytes are a wide character in.
#include <_utf8.h>

// Read the stream's next wide character into *wc, a byte no part of it left.
_Wfile_Status _Wfile_ReadChar(_File_Stream *stream, int *wc)
{
    int ch;
    int len;
    unsigned long count = 0;
    char buf[_UTF8_MAX];

    // Phase: take bytes until they are a character, or cannot be one
    _File_OrientStream(stream, _FILE_ORIENT_WIDE);
    if (_File_PrepareInput(stream) != 0) {
        return _WFILE_STATUS_FAILED;
    }
    while ((ch = _File_PeekChar(stream)) != _FILE_END) {
        buf[count] = (char) ch;
        len = _Utf8_DecodeChar(buf, count + 1, wc);
        if (len != _UTF8_INVALID || count == 0) {
            _File_SkipChar(stream);
            count++;
        }
        if (len > 0) {
            return _WFILE_STATUS_GOOD;
        }
        if (len == _UTF8_INVALID) {
            break;
        }
    }

    // Phase: tell the stream's end and a failed read from bytes that are none
    if (ch == _FILE_END && (count == 0 || ! (stream->fs_flags & _FILE_EOF))) {
        return stream->fs_flags & _FILE_EOF ? _WFILE_STATUS_END : _WFILE_STATUS_FAILED;
    }
    _Err_Errno = _SYS_EILSEQ;
    stream->fs_flags |= _FILE_ERROR;
    return _WFILE_STATUS_FAILED;
}

// Get the stream's next wide character, or _WFILE_END.
unsigned int _Wfile_GetChar(_File_Stream *stream)
{
    int wc;

    return _Wfile_ReadChar(stream, &wc) == _WFILE_STATUS_GOOD ? (unsigned int) wc : _WFILE_END;
}

// Push the wide character wc back for the stream's next read, or give none.
unsigned int _Wfile_UngetChar(_File_Stream *stream, unsigned int wc)
{
    int len;
    char buf[_UTF8_MAX];

    _File_OrientStream(stream, _FILE_ORIENT_WIDE);
    len = _Utf8_EncodeChar((int) wc, buf);
    if (len < 0 || (unsigned long) len > _FILE_BACKS - stream->fs_backs) {
        return _WFILE_END;
    }
    while (len > 0) {
        if (_File_UngetChar(stream, (unsigned char) buf[--len]) == _FILE_END) {
            return _WFILE_END;
        }
    }
    return wc;
}

// Get a line of at most max wide characters, its new-line among them.
int *_Wfile_GetLine(_File_Stream *stream, int *str, unsigned long max)
{
    int wc = 0;
    unsigned long len = 0;
    _Wfile_Status status = _WFILE_STATUS_GOOD;

    while (len < max && wc != '\n' && (status = _Wfile_ReadChar(stream, &wc)) == _WFILE_STATUS_GOOD) {
        str[len++] = wc;
    }
    if (status == _WFILE_STATUS_FAILED || (status == _WFILE_STATUS_END && len == 0)) {
        return _WFILE_NULL;
    }
    str[len] = 0;
    return str;
}

// Put the wide character wc in the stream as its bytes, and give it back.
unsigned int _Wfile_PutChar(_File_Stream *stream, int wc)
{
    int len;
    char buf[_UTF8_MAX];

    _File_OrientStream(stream, _FILE_ORIENT_WIDE);
    len = _Utf8_EncodeChar(wc, buf);
    if (len < 0) {
        _Err_Errno = _SYS_EILSEQ;
        return _WFILE_END;
    }
    if (_File_PutBytes(stream, buf, (unsigned long) len) != (unsigned long) len) {
        return _WFILE_END;
    }
    return (unsigned int) wc;
}

// Put the wide string str in the stream, and give 0, or _FILE_END for failure.
int _Wfile_PutString(_File_Stream *stream, const int *str)
{
    for (unsigned long i = 0; str[i] != 0; i++) {
        if (_Wfile_PutChar(stream, str[i]) == _WFILE_END) {
            return _FILE_END;
        }
    }
    return 0;
}
