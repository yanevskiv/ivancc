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

#ifndef __LIBC_IMPL_STRING_H__
#define __LIBC_IMPL_STRING_H__

// The size and null pointer the functions take.
#include <libc/impl/libc_stddef.h>

// The base strerror writes an unknown error number in.
#define __LIBC_IMPL_STRING_ERROR_BASE 10

// The size of strerror's text for an unknown error number, INT_MIN's included.
#define __LIBC_IMPL_STRING_ERROR_SIZE 32

// Copying functions
void *__libc_impl_string_memcpy(void *restrict str1, const void *restrict str2, __libc_impl_stddef_size_t n);
void *__libc_impl_string_memmove(void *str1, const void *str2, __libc_impl_stddef_size_t n);
char *__libc_impl_string_strcpy(char *restrict str1, const char *restrict str2);
char *__libc_impl_string_strncpy(char *restrict str1, const char *restrict str2, __libc_impl_stddef_size_t n);

// Concatenation functions
char *__libc_impl_string_strcat(char *restrict str1, const char *restrict str2);
char *__libc_impl_string_strncat(char *restrict str1, const char *restrict str2, __libc_impl_stddef_size_t n);

// Comparison functions
int __libc_impl_string_memcmp(const void *str1, const void *str2, __libc_impl_stddef_size_t n);
int __libc_impl_string_strcmp(const char *str1, const char *str2);
int __libc_impl_string_strcoll(const char *str1, const char *str2);
int __libc_impl_string_strncmp(const char *str1, const char *str2, __libc_impl_stddef_size_t n);
__libc_impl_stddef_size_t __libc_impl_string_strxfrm(char *restrict str1, const char *restrict str2, __libc_impl_stddef_size_t n);

// Search functions
void *__libc_impl_string_memchr(const void *str, int ch, __libc_impl_stddef_size_t n);
char *__libc_impl_string_strchr(const char *str, int ch);
__libc_impl_stddef_size_t __libc_impl_string_strcspn(const char *str1, const char *str2);
char *__libc_impl_string_strpbrk(const char *str1, const char *str2);
char *__libc_impl_string_strrchr(const char *str, int ch);
__libc_impl_stddef_size_t __libc_impl_string_strspn(const char *str1, const char *str2);
char *__libc_impl_string_strstr(const char *str1, const char *str2);
char *__libc_impl_string_strtok(char *restrict str1, const char *restrict str2);

// Miscellaneous functions
void *__libc_impl_string_memset(void *str, int ch, __libc_impl_stddef_size_t n);
char *__libc_impl_string_strerror(int errnum);
__libc_impl_stddef_size_t __libc_impl_string_strlen(const char *str);

// Error texts
char *__libc_impl_string_format_unknown(int errnum);

#endif // __LIBC_IMPL_STRING_H__
