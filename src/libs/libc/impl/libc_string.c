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
#include <libc/impl/libc_string.h>

// The texts strerror gives.
#include <libc/libc_err.h>

// Where strtok's next search starts.
static char *_Libc_Impl_String_Next;

// strerror's text for an unknown error number.
static char _Libc_Impl_String_Unknown[_LIBC_IMPL_STRING_ERROR_SIZE];

// Copy n bytes from str2 to str1, which must not overlap.
void *_Libc_Impl_String_memcpy(void *restrict str1, const void *restrict str2, _Libc_Impl_Stddef_size_t n)
{
    unsigned char *to = str1;
    const unsigned char *from = str2;

    for (_Libc_Impl_Stddef_size_t i = 0; i < n; i++) {
        to[i] = from[i];
    }
    return str1;
}

// Copy n bytes from str2 to str1, which may overlap.
void *_Libc_Impl_String_memmove(void *str1, const void *str2, _Libc_Impl_Stddef_size_t n)
{
    unsigned char *to = str1;
    const unsigned char *from = str2;

    if (to < from) {
        for (_Libc_Impl_Stddef_size_t i = 0; i < n; i++) {
            to[i] = from[i];
        }
    } else {
        for (_Libc_Impl_Stddef_size_t i = n; i > 0; i--) {
            to[i - 1] = from[i - 1];
        }
    }
    return str1;
}

// Copy the string str2 to str1.
char *_Libc_Impl_String_strcpy(char *restrict str1, const char *restrict str2)
{
    _Libc_Impl_Stddef_size_t i = 0;

    while (str2[i] != '\0') {
        str1[i] = str2[i];
        i++;
    }
    str1[i] = '\0';
    return str1;
}

// Copy at most n characters of the string str2 to str1, padded to n with nulls.
char *_Libc_Impl_String_strncpy(char *restrict str1, const char *restrict str2, _Libc_Impl_Stddef_size_t n)
{
    _Libc_Impl_Stddef_size_t i = 0;

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
char *_Libc_Impl_String_strcat(char *restrict str1, const char *restrict str2)
{
    _Libc_Impl_String_strcpy(str1 + _Libc_Impl_String_strlen(str1), str2);
    return str1;
}

// Append at most n characters of the string str2 to str1, and a null character.
char *_Libc_Impl_String_strncat(char *restrict str1, const char *restrict str2, _Libc_Impl_Stddef_size_t n)
{
    char *end = str1 + _Libc_Impl_String_strlen(str1);
    _Libc_Impl_Stddef_size_t i = 0;

    while (i < n && str2[i] != '\0') {
        end[i] = str2[i];
        i++;
    }
    end[i] = '\0';
    return str1;
}

// Compare n bytes of str1 and str2 as unsigned chars.
int _Libc_Impl_String_memcmp(const void *str1, const void *str2, _Libc_Impl_Stddef_size_t n)
{
    const unsigned char *left = str1;
    const unsigned char *right = str2;

    for (_Libc_Impl_Stddef_size_t i = 0; i < n; i++) {
        if (left[i] != right[i]) {
            return left[i] - right[i];
        }
    }
    return 0;
}

// Compare the strings str1 and str2 as unsigned chars.
int _Libc_Impl_String_strcmp(const char *str1, const char *str2)
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
int _Libc_Impl_String_strcoll(const char *str1, const char *str2)
{
    return _Libc_Impl_String_strcmp(str1, str2);
}

// Compare at most n characters of the strings str1 and str2 as unsigned chars.
int _Libc_Impl_String_strncmp(const char *str1, const char *str2, _Libc_Impl_Stddef_size_t n)
{
    const unsigned char *left = (const unsigned char *) str1;
    const unsigned char *right = (const unsigned char *) str2;

    for (_Libc_Impl_Stddef_size_t i = 0; i < n; i++) {
        if (left[i] != right[i] || left[i] == '\0') {
            return left[i] - right[i];
        }
    }
    return 0;
}

// Transform the string str2 into str1, which strcmp orders as strcoll would.
_Libc_Impl_Stddef_size_t _Libc_Impl_String_strxfrm(char *restrict str1, const char *restrict str2, _Libc_Impl_Stddef_size_t n)
{
    _Libc_Impl_Stddef_size_t len = _Libc_Impl_String_strlen(str2);

    if (len < n) {
        _Libc_Impl_String_memcpy(str1, str2, len + 1);
    }
    return len;
}

// Find the first byte ch in n bytes of str.
void *_Libc_Impl_String_memchr(const void *str, int ch, _Libc_Impl_Stddef_size_t n)
{
    const unsigned char *ptr = str;

    for (_Libc_Impl_Stddef_size_t i = 0; i < n; i++) {
        if (ptr[i] == (unsigned char) ch) {
            return (void *) (ptr + i);
        }
    }
    return _LIBC_IMPL_STDDEF_NULL;
}

// Find the first character ch in the string str, its null character included.
char *_Libc_Impl_String_strchr(const char *str, int ch)
{
    const char *ptr = str;

    while (*ptr != (char) ch) {
        if (*ptr == '\0') {
            return _LIBC_IMPL_STDDEF_NULL;
        }
        ptr++;
    }
    return (char *) ptr;
}

// Count the characters at the start of the string str1 that are not in str2.
_Libc_Impl_Stddef_size_t _Libc_Impl_String_strcspn(const char *str1, const char *str2)
{
    _Libc_Impl_Stddef_size_t len = 0;

    while (str1[len] != '\0' && _Libc_Impl_String_strchr(str2, str1[len]) == _LIBC_IMPL_STDDEF_NULL) {
        len++;
    }
    return len;
}

// Find the first character of the string str1 that is in str2.
char *_Libc_Impl_String_strpbrk(const char *str1, const char *str2)
{
    const char *ptr = str1 + _Libc_Impl_String_strcspn(str1, str2);

    if (*ptr == '\0') {
        return _LIBC_IMPL_STDDEF_NULL;
    }
    return (char *) ptr;
}

// Find the last character ch in the string str, its null character included.
char *_Libc_Impl_String_strrchr(const char *str, int ch)
{
    const char *found = _LIBC_IMPL_STDDEF_NULL;
    const char *ptr = str;

    do {
        if (*ptr == (char) ch) {
            found = ptr;
        }
    } while (*ptr++ != '\0');
    return (char *) found;
}

// Count the characters at the start of the string str1 that are in str2.
_Libc_Impl_Stddef_size_t _Libc_Impl_String_strspn(const char *str1, const char *str2)
{
    _Libc_Impl_Stddef_size_t len = 0;

    while (str1[len] != '\0' && _Libc_Impl_String_strchr(str2, str1[len]) != _LIBC_IMPL_STDDEF_NULL) {
        len++;
    }
    return len;
}

// Find the first occurrence of the string str2 in str1.
char *_Libc_Impl_String_strstr(const char *str1, const char *str2)
{
    _Libc_Impl_Stddef_size_t len = _Libc_Impl_String_strlen(str2);

    for (const char *ptr = str1; ; ptr++) {
        if (_Libc_Impl_String_strncmp(ptr, str2, len) == 0) {
            return (char *) ptr;
        }
        if (*ptr == '\0') {
            return _LIBC_IMPL_STDDEF_NULL;
        }
    }
}

// Split the string str1 into tokens between characters of str2, one per call.
char *_Libc_Impl_String_strtok(char *restrict str1, const char *restrict str2)
{
    char *start = str1 != _LIBC_IMPL_STDDEF_NULL ? str1 : _Libc_Impl_String_Next;
    char *end;

    if (start == _LIBC_IMPL_STDDEF_NULL) {
        return _LIBC_IMPL_STDDEF_NULL;
    }
    start += _Libc_Impl_String_strspn(start, str2);
    if (*start == '\0') {
        _Libc_Impl_String_Next = start;
        return _LIBC_IMPL_STDDEF_NULL;
    }
    end = start + _Libc_Impl_String_strcspn(start, str2);
    if (*end != '\0') {
        *end = '\0';
        end++;
    }
    _Libc_Impl_String_Next = end;
    return start;
}

// Set n bytes of str to the byte ch.
void *_Libc_Impl_String_memset(void *str, int ch, _Libc_Impl_Stddef_size_t n)
{
    unsigned char *to = str;

    for (_Libc_Impl_Stddef_size_t i = 0; i < n; i++) {
        to[i] = (unsigned char) ch;
    }
    return str;
}

// Name the error number errnum.
char *_Libc_Impl_String_strerror(int errnum)
{
    const char *text = _Libc_Err_ErrnoText(errnum);

    if (text == _LIBC_IMPL_STDDEF_NULL) {
        return _Libc_Impl_String_FormatUnknown(errnum);
    }
    return (char *) text;
}

// Count the characters of the string str before its null character.
_Libc_Impl_Stddef_size_t _Libc_Impl_String_strlen(const char *str)
{
    _Libc_Impl_Stddef_size_t len = 0;

    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Write strerror's text for the unknown error number errnum.
char *_Libc_Impl_String_FormatUnknown(int errnum)
{
    char *ptr = _Libc_Impl_String_Unknown + _LIBC_IMPL_STRING_ERROR_SIZE - 1;
    unsigned int value = errnum < 0 ? 0U - (unsigned int) errnum : (unsigned int) errnum;
    _Libc_Impl_Stddef_size_t len = _Libc_Impl_String_strlen(_LIBC_ERR_STRING_UNKNOWN);

    *ptr = '\0';
    do {
        ptr--;
        *ptr = (char) ('0' + value % _LIBC_IMPL_STRING_ERROR_BASE);
        value /= _LIBC_IMPL_STRING_ERROR_BASE;
    } while (value != 0);
    if (errnum < 0) {
        ptr--;
        *ptr = '-';
    }
    ptr -= len;
    _Libc_Impl_String_memcpy(ptr, _LIBC_ERR_STRING_UNKNOWN, len);
    return ptr;
}
