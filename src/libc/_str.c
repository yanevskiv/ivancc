/*
 * C source file for strings and arrays of bytes.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivcc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivcc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivcc; see
 * the files LICENSE and COPYING.RUNTIME respectively.  If not, see
 * <https://www.gnu.org/licenses/>.
 */


// Module header.
#include <_str.h>

// Copy n bytes from from to to, which must not overlap.
void *_Str_CopyBytes(void *to, const void *from, unsigned long n)
{
    unsigned char *dest = to;
    const unsigned char *src = from;

    for (unsigned long i = 0; i < n; i++) {
        dest[i] = src[i];
    }
    return to;
}

// Move n bytes from from to to, which may overlap.
void *_Str_MoveBytes(void *to, const void *from, unsigned long n)
{
    unsigned char *dest = to;
    const unsigned char *src = from;

    if (dest < src) {
        for (unsigned long i = 0; i < n; i++) {
            dest[i] = src[i];
        }
    } else {
        for (unsigned long i = n; i > 0; i--) {
            dest[i - 1] = src[i - 1];
        }
    }
    return to;
}

// Copy the string from to to.
char *_Str_CopyString(char *to, const char *from)
{
    unsigned long i = 0;

    while (from[i] != '\0') {
        to[i] = from[i];
        i++;
    }
    to[i] = '\0';
    return to;
}

// Copy at most n characters of the string from to to, padded to n with nulls.
char *_Str_CopyChars(char *to, const char *from, unsigned long n)
{
    unsigned long i = 0;

    while (i < n && from[i] != '\0') {
        to[i] = from[i];
        i++;
    }
    while (i < n) {
        to[i] = '\0';
        i++;
    }
    return to;
}

// Append the string from to the string to.
char *_Str_AppendString(char *to, const char *from)
{
    _Str_CopyString(to + _Str_GetLength(to), from);
    return to;
}

// Append at most n characters of the string from to to, and a null character.
char *_Str_AppendChars(char *to, const char *from, unsigned long n)
{
    char *end = to + _Str_GetLength(to);
    unsigned long i = 0;

    while (i < n && from[i] != '\0') {
        end[i] = from[i];
        i++;
    }
    end[i] = '\0';
    return to;
}

// Compare n bytes of left and right as unsigned chars.
int _Str_CompareBytes(const void *left, const void *right, unsigned long n)
{
    const unsigned char *one = left;
    const unsigned char *two = right;

    for (unsigned long i = 0; i < n; i++) {
        if (one[i] != two[i]) {
            return one[i] - two[i];
        }
    }
    return 0;
}

// Compare the strings left and right as unsigned chars.
int _Str_CompareStrings(const char *left, const char *right)
{
    const unsigned char *one = (const unsigned char *) left;
    const unsigned char *two = (const unsigned char *) right;

    while (*one != '\0' && *one == *two) {
        one++;
        two++;
    }
    return *one - *two;
}

// Compare at most n characters of the strings left and right as unsigned chars.
int _Str_CompareChars(const char *left, const char *right, unsigned long n)
{
    const unsigned char *one = (const unsigned char *) left;
    const unsigned char *two = (const unsigned char *) right;

    for (unsigned long i = 0; i < n; i++) {
        if (one[i] != two[i] || one[i] == '\0') {
            return one[i] - two[i];
        }
    }
    return 0;
}

// Find the first byte ch in n bytes of str.
void *_Str_FindByte(const void *str, int ch, unsigned long n)
{
    const unsigned char *ptr = str;

    for (unsigned long i = 0; i < n; i++) {
        if (ptr[i] == (unsigned char) ch) {
            return (void *) (ptr + i);
        }
    }
    return _STR_NULL;
}

// Find the first character ch in the string str, its null character included.
char *_Str_FindChar(const char *str, int ch)
{
    const char *ptr = str;

    while (*ptr != (char) ch) {
        if (*ptr == '\0') {
            return _STR_NULL;
        }
        ptr++;
    }
    return (char *) ptr;
}

// Count the characters at the start of the string str that are not in set.
unsigned long _Str_CountUnlisted(const char *str, const char *set)
{
    unsigned long len = 0;

    while (str[len] != '\0' && _Str_FindChar(set, str[len]) == _STR_NULL) {
        len++;
    }
    return len;
}

// Find the first character of the string str that is in set.
char *_Str_FindListed(const char *str, const char *set)
{
    const char *ptr = str + _Str_CountUnlisted(str, set);

    if (*ptr == '\0') {
        return _STR_NULL;
    }
    return (char *) ptr;
}

// Find the last character ch in the string str, its null character included.
char *_Str_FindLastChar(const char *str, int ch)
{
    const char *found = _STR_NULL;
    const char *ptr = str;

    do {
        if (*ptr == (char) ch) {
            found = ptr;
        }
    } while (*ptr++ != '\0');
    return (char *) found;
}

// Count the characters at the start of the string str that are in set.
unsigned long _Str_CountListed(const char *str, const char *set)
{
    unsigned long len = 0;

    while (str[len] != '\0' && _Str_FindChar(set, str[len]) != _STR_NULL) {
        len++;
    }
    return len;
}

// Find the first occurrence of the string part in str.
char *_Str_FindString(const char *str, const char *part)
{
    unsigned long len = _Str_GetLength(part);

    for (const char *ptr = str; ; ptr++) {
        if (_Str_CompareChars(ptr, part, len) == 0) {
            return (char *) ptr;
        }
        if (*ptr == '\0') {
            return _STR_NULL;
        }
    }
}

// Split the next token off str, or off *next if str is null, between set's.
char *_Str_SplitToken(char *str, const char *set, char **next)
{
    char *start = str != _STR_NULL ? str : *next;
    char *end;

    if (start == _STR_NULL) {
        return _STR_NULL;
    }
    start += _Str_CountListed(start, set);
    if (*start == '\0') {
        *next = start;
        return _STR_NULL;
    }
    end = start + _Str_CountUnlisted(start, set);
    if (*end != '\0') {
        *end = '\0';
        end++;
    }
    *next = end;
    return start;
}

// Fill n bytes of str with the byte ch.
void *_Str_FillBytes(void *str, int ch, unsigned long n)
{
    unsigned char *to = str;

    for (unsigned long i = 0; i < n; i++) {
        to[i] = (unsigned char) ch;
    }
    return str;
}

// Get the length of the string str, the characters before its null character.
unsigned long _Str_GetLength(const char *str)
{
    unsigned long len = 0;

    while (str[len] != '\0') {
        len++;
    }
    return len;
}
