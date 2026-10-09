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
#include <ivancc/impl/libc_string.h>

// The texts strerror gives.
#include <ivancc/libc_err.h>

// Where strtok's next search starts.
static char *__libc_impl_string_next;

// strerror's text for an unknown error number.
static char __libc_impl_string_unknown[__LIBC_IMPL_STRING_ERROR_SIZE];

// Copy n bytes from str2 to str1, which must not overlap.
void *__libc_impl_string_memcpy(void *restrict str1, const void *restrict str2, size_t n)
{
    unsigned char *to = str1;
    const unsigned char *from = str2;

    for (size_t i = 0; i < n; i++) {
        to[i] = from[i];
    }
    return str1;
}

// Copy n bytes from str2 to str1, which may overlap.
void *__libc_impl_string_memmove(void *str1, const void *str2, size_t n)
{
    unsigned char *to = str1;
    const unsigned char *from = str2;

    if (to < from) {
        for (size_t i = 0; i < n; i++) {
            to[i] = from[i];
        }
    } else {
        for (size_t i = n; i > 0; i--) {
            to[i - 1] = from[i - 1];
        }
    }
    return str1;
}

// Copy the string str2 to str1.
char *__libc_impl_string_strcpy(char *restrict str1, const char *restrict str2)
{
    size_t i = 0;

    while (str2[i] != '\0') {
        str1[i] = str2[i];
        i++;
    }
    str1[i] = '\0';
    return str1;
}

// Copy at most n characters of the string str2 to str1, padded to n with nulls.
char *__libc_impl_string_strncpy(char *restrict str1, const char *restrict str2, size_t n)
{
    size_t i = 0;

    while (i < n && str2[i] != '\0') {
        str1[i] = str2[i];
        i++;
    }
    while (i < n) {
        str1[i] = '\0';
        i++;
    }
    return str1;
}

// Append the string str2 to str1.
char *__libc_impl_string_strcat(char *restrict str1, const char *restrict str2)
{
    __libc_impl_string_strcpy(str1 + __libc_impl_string_strlen(str1), str2);
    return str1;
}

// Append at most n characters of the string str2 to str1, and a null character.
char *__libc_impl_string_strncat(char *restrict str1, const char *restrict str2, size_t n)
{
    char *end = str1 + __libc_impl_string_strlen(str1);
    size_t i = 0;

    while (i < n && str2[i] != '\0') {
        end[i] = str2[i];
        i++;
    }
    end[i] = '\0';
    return str1;
}

// Compare n bytes of str1 and str2 as unsigned chars.
int __libc_impl_string_memcmp(const void *str1, const void *str2, size_t n)
{
    const unsigned char *left = str1;
    const unsigned char *right = str2;

    for (size_t i = 0; i < n; i++) {
        if (left[i] != right[i]) {
            return left[i] - right[i];
        }
    }
    return 0;
}

// Compare the strings str1 and str2 as unsigned chars.
int __libc_impl_string_strcmp(const char *str1, const char *str2)
{
    const unsigned char *left = (const unsigned char *) str1;
    const unsigned char *right = (const unsigned char *) str2;

    while (*left != '\0' && *left == *right) {
        left++;
        right++;
    }
    return *left - *right;
}

// Compare the strings str1 and str2 in the locale's collation order.
int __libc_impl_string_strcoll(const char *str1, const char *str2)
{
    return __libc_impl_string_strcmp(str1, str2);
}

// Compare at most n characters of the strings str1 and str2 as unsigned chars.
int __libc_impl_string_strncmp(const char *str1, const char *str2, size_t n)
{
    const unsigned char *left = (const unsigned char *) str1;
    const unsigned char *right = (const unsigned char *) str2;

    for (size_t i = 0; i < n; i++) {
        if (left[i] != right[i] || left[i] == '\0') {
            return left[i] - right[i];
        }
    }
    return 0;
}

// Transform the string str2 into str1, which strcmp orders as strcoll would.
size_t __libc_impl_string_strxfrm(char *restrict str1, const char *restrict str2, size_t n)
{
    size_t len = __libc_impl_string_strlen(str2);

    if (len < n) {
        __libc_impl_string_memcpy(str1, str2, len + 1);
    }
    return len;
}

// Find the first byte ch in n bytes of str.
void *__libc_impl_string_memchr(const void *str, int ch, size_t n)
{
    const unsigned char *ptr = str;

    for (size_t i = 0; i < n; i++) {
        if (ptr[i] == (unsigned char) ch) {
            return (void *) (ptr + i);
        }
    }
    return NULL;
}

// Find the first character ch in the string str, its null character included.
char *__libc_impl_string_strchr(const char *str, int ch)
{
    const char *ptr = str;

    while (*ptr != (char) ch) {
        if (*ptr == '\0') {
            return NULL;
        }
        ptr++;
    }
    return (char *) ptr;
}

// Count the characters at the start of the string str1 that are not in str2.
size_t __libc_impl_string_strcspn(const char *str1, const char *str2)
{
    size_t len = 0;

    while (str1[len] != '\0' && __libc_impl_string_strchr(str2, str1[len]) == NULL) {
        len++;
    }
    return len;
}

// Find the first character of the string str1 that is in str2.
char *__libc_impl_string_strpbrk(const char *str1, const char *str2)
{
    const char *ptr = str1 + __libc_impl_string_strcspn(str1, str2);

    if (*ptr == '\0') {
        return NULL;
    }
    return (char *) ptr;
}

// Find the last character ch in the string str, its null character included.
char *__libc_impl_string_strrchr(const char *str, int ch)
{
    const char *found = NULL;
    const char *ptr = str;

    do {
        if (*ptr == (char) ch) {
            found = ptr;
        }
    } while (*ptr++ != '\0');
    return (char *) found;
}

// Count the characters at the start of the string str1 that are in str2.
size_t __libc_impl_string_strspn(const char *str1, const char *str2)
{
    size_t len = 0;

    while (str1[len] != '\0' && __libc_impl_string_strchr(str2, str1[len]) != NULL) {
        len++;
    }
    return len;
}

// Find the first occurrence of the string str2 in str1.
char *__libc_impl_string_strstr(const char *str1, const char *str2)
{
    size_t len = __libc_impl_string_strlen(str2);

    for (const char *ptr = str1; ; ptr++) {
        if (__libc_impl_string_strncmp(ptr, str2, len) == 0) {
            return (char *) ptr;
        }
        if (*ptr == '\0') {
            return NULL;
        }
    }
}

// Split the string str1 into tokens between characters of str2, one per call.
char *__libc_impl_string_strtok(char *restrict str1, const char *restrict str2)
{
    char *start = str1 != NULL ? str1 : __libc_impl_string_next;
    char *end;

    if (start == NULL) {
        return NULL;
    }
    start += __libc_impl_string_strspn(start, str2);
    if (*start == '\0') {
        __libc_impl_string_next = start;
        return NULL;
    }
    end = start + __libc_impl_string_strcspn(start, str2);
    if (*end != '\0') {
        *end = '\0';
        end++;
    }
    __libc_impl_string_next = end;
    return start;
}

// Set n bytes of str to the byte ch.
void *__libc_impl_string_memset(void *str, int ch, size_t n)
{
    unsigned char *to = str;

    for (size_t i = 0; i < n; i++) {
        to[i] = (unsigned char) ch;
    }
    return str;
}

// Name the error number errnum.
char *__libc_impl_string_strerror(int errnum)
{
    const char *text = __libc_err_errno_text(errnum);

    if (text == NULL) {
        return __libc_impl_string_format_unknown(errnum);
    }
    return (char *) text;
}

// Count the characters of the string str before its null character.
size_t __libc_impl_string_strlen(const char *str)
{
    size_t len = 0;

    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Write strerror's text for the unknown error number errnum.
char *__libc_impl_string_format_unknown(int errnum)
{
    char *ptr = __libc_impl_string_unknown + __LIBC_IMPL_STRING_ERROR_SIZE - 1;
    unsigned int value = errnum < 0 ? 0U - (unsigned int) errnum : (unsigned int) errnum;
    size_t len = __libc_impl_string_strlen(__LIBC_ERR_STRING_UNKNOWN);

    *ptr = '\0';
    do {
        ptr--;
        *ptr = (char) ('0' + value % __LIBC_IMPL_STRING_ERROR_BASE);
        value /= __LIBC_IMPL_STRING_ERROR_BASE;
    } while (value != 0);
    if (errnum < 0) {
        ptr--;
        *ptr = '-';
    }
    ptr -= len;
    __libc_impl_string_memcpy(ptr, __LIBC_ERR_STRING_UNKNOWN, len);
    return ptr;
}
