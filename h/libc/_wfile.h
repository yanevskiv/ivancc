/*
 * C header file for the wide characters of a stream.
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

#ifndef _WFILE_H
#define _WFILE_H

// The streams wide characters are read from and written to.
#include <_file.h>

// A null wide string, which a line that cannot be read is.
#define _WFILE_NULL ((int *) 0)

// What a call gives at a stream's end or when it fails.
#define _WFILE_END 0xFFFFFFFFU

// How reading a wide character went.
typedef enum _Wfile_Status _Wfile_Status;
enum _Wfile_Status {
    _WFILE_STATUS_GOOD,  // a character was read
    _WFILE_STATUS_END,   // the stream ended before one began
    _WFILE_STATUS_FAILED // a read failed, or the bytes are no character
};

// Input
_Wfile_Status _Wfile_ReadChar(_File_Stream *stream, int *wc);
unsigned int  _Wfile_GetChar(_File_Stream *stream);
unsigned int  _Wfile_UngetChar(_File_Stream *stream, unsigned int wc);
int          *_Wfile_GetLine(_File_Stream *stream, int *str, unsigned long max);

// Output
unsigned int _Wfile_PutChar(_File_Stream *stream, int wc);
int          _Wfile_PutString(_File_Stream *stream, const int *str);

#endif // _WFILE_H
