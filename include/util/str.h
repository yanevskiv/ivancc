// C header file for string utilities.

#ifndef STR_H
#define STR_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

// Capacity a new Str_Buf starts with.
#define STR_BUF_MIN_CAP 64

// A list of owned strings, as produced by Str_Split.
typedef struct Str_List Str_List;
struct Str_List {
    char **sl_items;
    size_t sl_count;
};

// A growable NUL-terminated string.
typedef struct Str_Buf Str_Buf;

// String utility functions
char *Str_Clone(const char *str);
char *Str_Slice(const char *str, size_t start, size_t end);
char *Str_Format(const char *fmt, ...);
char *Str_FormatVa(const char *fmt, va_list ap);
char *Str_ChangeOrAppendExt(const char *input, const char *suffix);
bool Str_Equals(const char *a, const char *b);
bool Str_StartsWith(const char *str, const char *prefix);
char *Str_Trim(char *str);
void Str_Free(char *str);

// String splitting
Str_List Str_Split(const char *str, const char *sep);
void Str_ListFree(Str_List *list);

// Growable strings
Str_Buf *Str_BufNew(void);
const char *Str_BufData(const Str_Buf *buf);
size_t Str_BufLen(const Str_Buf *buf);
void Str_BufReserve(Str_Buf *buf, size_t n);
void Str_BufPutByte(Str_Buf *buf, char byte);
void Str_BufPutBytes(Str_Buf *buf, const char *data, size_t len);
void Str_BufPutText(Str_Buf *buf, const char *text);
void Str_BufPrint(Str_Buf *buf, const char *fmt, ...);
char *Str_BufTake(Str_Buf *buf);
void Str_BufFree(Str_Buf *buf);

#endif // STR_H
