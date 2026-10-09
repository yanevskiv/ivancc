/*
 * C header file for string handling.
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

#ifndef _LIBC_IMPL_STRING_H
#define _LIBC_IMPL_STRING_H

// The size and null pointer the functions take.
#include <libc/impl/libc_stddef.h>

// The base strerror writes an unknown error number in.
#define _LIBC_IMPL_STRING_ERROR_BASE 10

// The size of strerror's text for an unknown error number, INT_MIN's included.
#define _LIBC_IMPL_STRING_ERROR_SIZE 32

// Copying functions
void *_Libc_Impl_String_memcpy(void *restrict str1, const void *restrict str2, _Libc_Impl_Stddef_size_t n);
void *_Libc_Impl_String_memmove(void *str1, const void *str2, _Libc_Impl_Stddef_size_t n);
char *_Libc_Impl_String_strcpy(char *restrict str1, const char *restrict str2);
char *_Libc_Impl_String_strncpy(char *restrict str1, const char *restrict str2, _Libc_Impl_Stddef_size_t n);

// Concatenation functions
char *_Libc_Impl_String_strcat(char *restrict str1, const char *restrict str2);
char *_Libc_Impl_String_strncat(char *restrict str1, const char *restrict str2, _Libc_Impl_Stddef_size_t n);

// Comparison functions
int _Libc_Impl_String_memcmp(const void *str1, const void *str2, _Libc_Impl_Stddef_size_t n);
int _Libc_Impl_String_strcmp(const char *str1, const char *str2);
int _Libc_Impl_String_strcoll(const char *str1, const char *str2);
int _Libc_Impl_String_strncmp(const char *str1, const char *str2, _Libc_Impl_Stddef_size_t n);
_Libc_Impl_Stddef_size_t _Libc_Impl_String_strxfrm(char *restrict str1, const char *restrict str2, _Libc_Impl_Stddef_size_t n);

// Search functions
void *_Libc_Impl_String_memchr(const void *str, int ch, _Libc_Impl_Stddef_size_t n);
char *_Libc_Impl_String_strchr(const char *str, int ch);
_Libc_Impl_Stddef_size_t _Libc_Impl_String_strcspn(const char *str1, const char *str2);
char *_Libc_Impl_String_strpbrk(const char *str1, const char *str2);
char *_Libc_Impl_String_strrchr(const char *str, int ch);
_Libc_Impl_Stddef_size_t _Libc_Impl_String_strspn(const char *str1, const char *str2);
char *_Libc_Impl_String_strstr(const char *str1, const char *str2);
char *_Libc_Impl_String_strtok(char *restrict str1, const char *restrict str2);

// Miscellaneous functions
void *_Libc_Impl_String_memset(void *str, int ch, _Libc_Impl_Stddef_size_t n);
char *_Libc_Impl_String_strerror(int errnum);
_Libc_Impl_Stddef_size_t _Libc_Impl_String_strlen(const char *str);

// Error texts
char *_Libc_Impl_String_FormatUnknown(int errnum);

#endif // _LIBC_IMPL_STRING_H
