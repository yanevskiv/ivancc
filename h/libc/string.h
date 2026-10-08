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

#ifndef __STRING_H__
#define __STRING_H__

// (S7.21.1) String function conventions
#ifndef __SIZE_T__
#define __SIZE_T__
typedef unsigned long size_t;
#endif

#define NULL ((void *) 0)

// (S7.21.2) Copying functions
void *memcpy(void *restrict str1, const void *restrict str2, size_t n);
void *memmove(void *str1, const void *str2, size_t n);
char *strcpy(char *restrict str1, const char *restrict str2);
char *strncpy(char *restrict str1, const char *restrict str2, size_t n);

// (S7.21.3) Concatenation functions
char *strcat(char *restrict str1, const char *restrict str2);
char *strncat(char *restrict str1, const char *restrict str2, size_t n);

// (S7.21.4) Comparison functions
int memcmp(const void *str1, const void *str2, size_t n);
int strcmp(const char *str1, const char *str2);
int strcoll(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t n);
size_t strxfrm(char *restrict str1, const char *restrict str2, size_t n);

// (S7.21.5) Search functions
void *memchr(const void *str, int ch, size_t n);
char *strchr(const char *str, int ch);
size_t strcspn(const char *str1, const char *str2);
char *strpbrk(const char *str1, const char *str2);
char *strrchr(const char *str, int ch);
size_t strspn(const char *str1, const char *str2);
char *strstr(const char *str1, const char *str2);
char *strtok(char *restrict str1, const char *restrict str2);

// (S7.21.6) Miscellaneous functions
void *memset(void *str, int ch, size_t n);
char *strerror(int errnum);
size_t strlen(const char *str);

#endif // __STRING_H__
