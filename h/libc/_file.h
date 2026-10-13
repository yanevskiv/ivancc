/*
 * C header file for streams and the files under them.
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


#ifndef _FILE_H
#define _FILE_H

// A pointer to nothing, of any type.
#define _FILE_NULL ((void *) 0)

// What a stream may do, and what has happened to it.
#define _FILE_READ    0x1
#define _FILE_WRITE   0x2
#define _FILE_ERROR   0x4
#define _FILE_OWNBUF  0x8
#define _FILE_SETUP   0x10
#define _FILE_EOF     0x20
#define _FILE_BACK    0x40
#define _FILE_OWNFILE 0x80

// What a stream did last.
#define _FILE_IDLE    0
#define _FILE_READING 1
#define _FILE_WRITING 2

// A stream's buffering: full, by line or none.
#define _FILE_FULL 0
#define _FILE_LINE 1
#define _FILE_NONE 2

// The buffering a stream's first use picks: by line on a terminal, else full.
#define _FILE_AUTO (-1)

// The descriptor of a stream with no file.
#define _FILE_NOFILE (-1)

// What a call gives at a stream's end or when it fails.
#define _FILE_END (-1)

// The size of the buffer a stream allocates.
#define _FILE_BUFFER 8192

// A temporary name: where it is, and the characters and length of its end.
#define _FILE_TEMP_PREFIX  "/tmp/tmp."
#define _FILE_TEMP_LETTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
#define _FILE_TEMP_RADIX   62
#define _FILE_TEMP_DIGITS  8

// How many temporary names there are, and a step that shares no factor with it.
#define _FILE_TEMP_NAMES 218340105584896UL
#define _FILE_TEMP_STEP  134941606358707UL

// The size of an array that holds a temporary name.
#define _FILE_TEMP_SIZE 20

// A stream: its descriptor, its buffer, what it may do and the stream after it.
#ifndef __FILE_STREAM__
#define __FILE_STREAM__
typedef struct _File_Stream _File_Stream;
struct _File_Stream {
    int fs_fd;             // the file's descriptor, or _FILE_NOFILE
    int fs_flags;          // _FILE_READ, _FILE_WRITE and what happened
    int fs_state;          // _FILE_IDLE, _FILE_READING, _FILE_WRITING
    int fs_mode;           // _FILE_FULL, _FILE_LINE, _FILE_NONE or _FILE_AUTO
    unsigned char *fs_buf; // the buffer, the stream's own or one it was given
    unsigned long fs_size; // the buffer's size
    unsigned long fs_head; // the bytes of the buffer already read
    unsigned long fs_tail; // the bytes the buffer holds, read or to write
    unsigned char fs_byte; // an unbuffered stream's buffer
    unsigned char fs_back; // the character pushed back
    _File_Stream *fs_next; // the next of the open streams
};
#endif

// The standard streams.
extern _File_Stream _File_Stdin;
extern _File_Stream _File_Stdout;
extern _File_Stream _File_Stderr;

// The open streams, newest first.
extern _File_Stream *_File_Streams;

// The number of the temporary name made last, once a seed has begun them.
extern unsigned long _File_TempNumber;
extern _Bool _File_TempSeeded;

// The name given back when a temporary name is asked for with no array.
extern char _File_TempBuffer[_FILE_TEMP_SIZE];

// Opening
_File_Stream *_File_MakeStream(void);
_Bool         _File_IsMode(const char *mode);
int           _File_ToMode(const char *mode);
void          _File_AttachDescriptor(_File_Stream *stream, int fd, int flags);
int           _File_OpenFile(_File_Stream *stream, const char *filename, const char *mode);
int           _File_ResetStream(_File_Stream *stream);
void          _File_FreeStream(_File_Stream *stream);

// Buffering
void  _File_SetupBuffer(_File_Stream *stream);
_Bool _File_IsTerminal(int fd);
int   _File_LendBuffer(_File_Stream *stream, unsigned char *buf, unsigned long size);
int   _File_ReclaimBuffer(_File_Stream *stream);

// Input
int           _File_PrepareInput(_File_Stream *stream);
unsigned long _File_GetBytes(_File_Stream *stream, void *data, unsigned long len);
unsigned long _File_ReadBuffer(_File_Stream *stream, unsigned char *data, unsigned long len);
unsigned long _File_ReadDescriptor(_File_Stream *stream, unsigned char *data, unsigned long len);
int           _File_GetChar(_File_Stream *stream);
char         *_File_GetLine(_File_Stream *stream, char *str, unsigned long max);

// Output
int           _File_PrepareOutput(_File_Stream *stream);
unsigned long _File_PutBytes(_File_Stream *stream, const void *data, unsigned long len);
int           _File_WriteDescriptor(_File_Stream *stream, const unsigned char *data, unsigned long len);
int           _File_FlushBuffer(_File_Stream *stream);
int           _File_FlushStreams(void);
void          _File_FlushLines(void);

// Positioning
long _File_CountReadAhead(_File_Stream *stream);
int  _File_SeekStream(_File_Stream *stream, long offset, int whence);
void _File_DiscardReadAhead(_File_Stream *stream);

// Temporary names
unsigned long _File_MakeTempSeed(void);
void          _File_WriteTempName(char *str);

#endif // _FILE_H
