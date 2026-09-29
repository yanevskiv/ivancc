/*
 * C header file for growable strings.
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
 * You should have received a copy of the GNU General Public License
 * along with ivancc.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef BUF_H
#define BUF_H

// Standard headers.
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Capacity a new Buf starts with.
#define BUF_MIN_CAP 64

// A growable NUL-terminated string.
typedef struct Buf Buf;

// Growable strings
Buf        *Buf_New(void);
const char *Buf_Data(const Buf *buf);
size_t      Buf_Len(const Buf *buf);
void        Buf_Reserve(Buf *buf, size_t n);
void        Buf_PutByte(Buf *buf, char byte);
void        Buf_PutBytes(Buf *buf, const char *data, size_t len);
void        Buf_PutText(Buf *buf, const char *text);
void        Buf_Print(Buf *buf, const char *fmt, ...);
char       *Buf_Release(Buf *buf);
void        Buf_Free(Buf *buf);

#endif // BUF_H
