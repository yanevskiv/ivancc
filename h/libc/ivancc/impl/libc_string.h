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

#ifndef __LIBC_STRING_H__
#define __LIBC_STRING_H__

// The size and null pointer the functions take.
#include <stddef.h>

// The error number of no error.
#define __LIBC_STRING_ERROR_NONE 0

// The text strerror puts before an error number it does not know.
#define __LIBC_STRING_ERROR_UNKNOWN "Unknown error "

// The base strerror writes an unknown error number in.
#define __LIBC_STRING_ERROR_BASE 10

// The size of strerror's text for an unknown error number, INT_MIN's included.
#define __LIBC_STRING_ERROR_SIZE 32

// (S7.21.2) Copying functions
void *__libc_string_memcpy(void *restrict str1, const void *restrict str2, size_t n);
void *__libc_string_memmove(void *str1, const void *str2, size_t n);
char *__libc_string_strcpy(char *restrict str1, const char *restrict str2);
char *__libc_string_strncpy(char *restrict str1, const char *restrict str2, size_t n);

// (S7.21.3) Concatenation functions
char *__libc_string_strcat(char *restrict str1, const char *restrict str2);
char *__libc_string_strncat(char *restrict str1, const char *restrict str2, size_t n);

// (S7.21.4) Comparison functions
int __libc_string_memcmp(const void *str1, const void *str2, size_t n);
int __libc_string_strcmp(const char *str1, const char *str2);
int __libc_string_strcoll(const char *str1, const char *str2);
int __libc_string_strncmp(const char *str1, const char *str2, size_t n);
size_t __libc_string_strxfrm(char *restrict str1, const char *restrict str2, size_t n);

// (S7.21.5) Search functions
void *__libc_string_memchr(const void *str, int ch, size_t n);
char *__libc_string_strchr(const char *str, int ch);
size_t __libc_string_strcspn(const char *str1, const char *str2);
char *__libc_string_strpbrk(const char *str1, const char *str2);
char *__libc_string_strrchr(const char *str, int ch);
size_t __libc_string_strspn(const char *str1, const char *str2);
char *__libc_string_strstr(const char *str1, const char *str2);
char *__libc_string_strtok(char *restrict str1, const char *restrict str2);

// (S7.21.6) Miscellaneous functions
void *__libc_string_memset(void *str, int ch, size_t n);
char *__libc_string_strerror(int errnum);
size_t __libc_string_strlen(const char *str);

// Error texts
char *__libc_string_format_unknown(int errnum);

#endif // __LIBC_STRING_H__
