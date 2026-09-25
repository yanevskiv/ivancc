// C header file for string utilities.

#ifndef STR_H
#define STR_H

// Standard headers.
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"

// A list of owned strings, as produced by Str_Split.
typedef struct Str_List Str_List;
struct Str_List {
    char **sl_items;
    size_t sl_count;
};

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

#endif // STR_H
