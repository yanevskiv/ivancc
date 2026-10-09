/*
 * C source file for string handling.
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

// Module header.
#include <string.h>

// The implementation.
#include <ivancc/impl/libc_string.h>

// Copy n bytes from str2 to str1, which must not overlap.
void *memcpy(void *restrict str1, const void *restrict str2, size_t n)
{
    return __libc_string_memcpy(str1, str2, n);
}

// Copy n bytes from str2 to str1, which may overlap.
void *memmove(void *str1, const void *str2, size_t n)
{
    return __libc_string_memmove(str1, str2, n);
}

// Copy the string str2 to str1.
char *strcpy(char *restrict str1, const char *restrict str2)
{
    return __libc_string_strcpy(str1, str2);
}

// Copy at most n characters of the string str2 to str1, padded to n with nulls.
char *strncpy(char *restrict str1, const char *restrict str2, size_t n)
{
    return __libc_string_strncpy(str1, str2, n);
}

// Append the string str2 to str1.
char *strcat(char *restrict str1, const char *restrict str2)
{
    return __libc_string_strcat(str1, str2);
}

// Append at most n characters of the string str2 to str1, and a null character.
char *strncat(char *restrict str1, const char *restrict str2, size_t n)
{
    return __libc_string_strncat(str1, str2, n);
}

// Compare n bytes of str1 and str2 as unsigned chars.
int memcmp(const void *str1, const void *str2, size_t n)
{
    return __libc_string_memcmp(str1, str2, n);
}

// Compare the strings str1 and str2 as unsigned chars.
int strcmp(const char *str1, const char *str2)
{
    return __libc_string_strcmp(str1, str2);
}

// Compare the strings str1 and str2 in the locale's collation order.
int strcoll(const char *str1, const char *str2)
{
    return __libc_string_strcoll(str1, str2);
}

// Compare at most n characters of the strings str1 and str2 as unsigned chars.
int strncmp(const char *str1, const char *str2, size_t n)
{
    return __libc_string_strncmp(str1, str2, n);
}

// Transform the string str2 into str1, which strcmp orders as strcoll would.
size_t strxfrm(char *restrict str1, const char *restrict str2, size_t n)
{
    return __libc_string_strxfrm(str1, str2, n);
}

// Find the first byte ch in n bytes of str.
void *memchr(const void *str, int ch, size_t n)
{
    return __libc_string_memchr(str, ch, n);
}

// Find the first character ch in the string str, its null character included.
char *strchr(const char *str, int ch)
{
    return __libc_string_strchr(str, ch);
}

// Count the characters at the start of the string str1 that are not in str2.
size_t strcspn(const char *str1, const char *str2)
{
    return __libc_string_strcspn(str1, str2);
}

// Find the first character of the string str1 that is in str2.
char *strpbrk(const char *str1, const char *str2)
{
    return __libc_string_strpbrk(str1, str2);
}

// Find the last character ch in the string str, its null character included.
char *strrchr(const char *str, int ch)
{
    return __libc_string_strrchr(str, ch);
}

// Count the characters at the start of the string str1 that are in str2.
size_t strspn(const char *str1, const char *str2)
{
    return __libc_string_strspn(str1, str2);
}

// Find the first occurrence of the string str2 in str1.
char *strstr(const char *str1, const char *str2)
{
    return __libc_string_strstr(str1, str2);
}

// Split the string str1 into tokens between characters of str2, one per call.
char *strtok(char *restrict str1, const char *restrict str2)
{
    return __libc_string_strtok(str1, str2);
}

// Set n bytes of str to the byte ch.
void *memset(void *str, int ch, size_t n)
{
    return __libc_string_memset(str, ch, n);
}

// Name the error number errnum.
char *strerror(int errnum)
{
    return __libc_string_strerror(errnum);
}

// Count the characters of the string str before its null character.
size_t strlen(const char *str)
{
    return __libc_string_strlen(str);
}
