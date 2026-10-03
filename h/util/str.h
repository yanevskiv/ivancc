/*
 * C header file for string utilities.
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

#ifndef STR_H
#define STR_H

// Standard headers.
#include <regex.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"

// Most groups one pattern holds, plus one.
#define STR_REGEX_GROUPS 128

// String utility functions
char  *Str_Clone(const char *str);
char  *Str_Slice(const char *str, size_t start, size_t end);
char  *Str_Format(const char *fmt, ...);
char  *Str_FormatVa(const char *fmt, va_list ap);
char  *Str_ModifyExtension(const char *input, const char *suffix);
bool   Str_Equals(const char *a, const char *b);
bool   Str_StartsWith(const char *str, const char *prefix);
size_t Str_FindFirst(const char *str, const char *chars);
char  *Str_Trim(char *str);
void   Str_Free(char *str);

// String splitting
char **Str_Tokenize(const char *str, const char *sep);
void   Str_FreeTokens(char **tokens);

// Regex matches
void        Str_RegexCompile(regex_t *regex, const char *source);
bool        Str_RegexMatch(const regex_t *regex, const char *str, regmatch_t *match);
bool        Str_RegexSpan(const regex_t *regex, const char *str, regmatch_t *match);
int32_t     Str_RegexField(const regmatch_t *match, size_t index);
char       *Str_RegexFieldText(const char *str, const regmatch_t *match, size_t index);
const char *Str_RegexFieldTail(const char *str, const regmatch_t *match, size_t index);

#endif // STR_H
