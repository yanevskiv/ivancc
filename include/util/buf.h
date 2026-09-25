// C header file for growable strings.

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
char       *Buf_Take(Buf *buf);
void        Buf_Free(Buf *buf);

#endif // BUF_H
