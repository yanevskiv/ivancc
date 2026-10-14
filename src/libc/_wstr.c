/*
 * C source file for the loops over wide characters and wide strings.
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
#include <_wstr.h>

// Copy n wide characters from from to to, which must not overlap.
int *_Wstr_CopyArray(int *to, const int *from, unsigned long n)
{
    for (unsigned long i = 0; i < n; i++) {
        to[i] = from[i];
    }
    return to;
}

// Move n wide characters from from to to, which may overlap.
int *_Wstr_MoveArray(int *to, const int *from, unsigned long n)
{
    if (to < from) {
        for (unsigned long i = 0; i < n; i++) {
            to[i] = from[i];
        }
    } else {
        for (unsigned long i = n; i > 0; i--) {
            to[i - 1] = from[i - 1];
        }
    }
    return to;
}

// Copy the wide string from to to.
int *_Wstr_CopyString(int *to, const int *from)
{
    unsigned long i = 0;

    while (from[i] != L'\0') {
        to[i] = from[i];
        i++;
    }
    to[i] = L'\0';
    return to;
}

// Copy at most n characters of the wide string from to to, padded with nulls.
int *_Wstr_CopyChars(int *to, const int *from, unsigned long n)
{
    unsigned long i = 0;

    while (i < n && from[i] != L'\0') {
        to[i] = from[i];
        i++;
    }
    while (i < n) {
        to[i] = L'\0';
        i++;
    }
    return to;
}

// Append the wide string from to the wide string to.
int *_Wstr_AppendString(int *to, const int *from)
{
    _Wstr_CopyString(to + _Wstr_GetLength(to), from);
    return to;
}

// Append at most n characters of the wide string from to to, and a null.
int *_Wstr_AppendChars(int *to, const int *from, unsigned long n)
{
    int *end = to + _Wstr_GetLength(to);
    unsigned long i = 0;

    while (i < n && from[i] != L'\0') {
        end[i] = from[i];
        i++;
    }
    end[i] = L'\0';
    return to;
}

// Compare n wide characters of left and right, each by its value.
int _Wstr_CompareArrays(const int *left, const int *right, unsigned long n)
{
    for (unsigned long i = 0; i < n; i++) {
        if (left[i] != right[i]) {
            return (left[i] > right[i]) - (left[i] < right[i]);
        }
    }
    return 0;
}

// Compare the wide strings left and right, each character by its value.
int _Wstr_CompareStrings(const int *left, const int *right)
{
    while (*left != L'\0' && *left == *right) {
        left++;
        right++;
    }
    return (*left > *right) - (*left < *right);
}

// Compare at most n characters of the wide strings left and right.
int _Wstr_CompareChars(const int *left, const int *right, unsigned long n)
{
    for (unsigned long i = 0; i < n; i++) {
        if (left[i] != right[i]) {
            return (left[i] > right[i]) - (left[i] < right[i]);
        }
        if (left[i] == L'\0') {
            return 0;
        }
    }
    return 0;
}

// Find the first wide character wc in n of str.
int *_Wstr_FindInArray(const int *str, int wc, unsigned long n)
{
    for (unsigned long i = 0; i < n; i++) {
        if (str[i] == wc) {
            return (int *) (str + i);
        }
    }
    return _WSTR_NULL;
}

// Find the first wc in the wide string str, its null character included.
int *_Wstr_FindChar(const int *str, int wc)
{
    const int *ptr = str;

    while (*ptr != wc) {
        if (*ptr == L'\0') {
            return _WSTR_NULL;
        }
        ptr++;
    }
    return (int *) ptr;
}

// Count the characters at the start of the wide string str that are not in set.
unsigned long _Wstr_CountUnlisted(const int *str, const int *set)
{
    unsigned long len = 0;

    while (str[len] != L'\0' && _Wstr_FindChar(set, str[len]) == _WSTR_NULL) {
        len++;
    }
    return len;
}

// Find the first character of the wide string str that is in set.
int *_Wstr_FindListed(const int *str, const int *set)
{
    const int *ptr = str + _Wstr_CountUnlisted(str, set);

    if (*ptr == L'\0') {
        return _WSTR_NULL;
    }
    return (int *) ptr;
}

// Find the last wc in the wide string str, its null character included.
int *_Wstr_FindLastChar(const int *str, int wc)
{
    const int *found = _WSTR_NULL;
    const int *ptr = str;

    do {
        if (*ptr == wc) {
            found = ptr;
        }
    } while (*ptr++ != L'\0');
    return (int *) found;
}

// Count the characters at the start of the wide string str that are in set.
unsigned long _Wstr_CountListed(const int *str, const int *set)
{
    unsigned long len = 0;

    while (str[len] != L'\0' && _Wstr_FindChar(set, str[len]) != _WSTR_NULL) {
        len++;
    }
    return len;
}

// Find the first occurrence of the wide string part in str.
int *_Wstr_FindString(const int *str, const int *part)
{
    unsigned long len = _Wstr_GetLength(part);

    for (const int *ptr = str; ; ptr++) {
        if (_Wstr_CompareChars(ptr, part, len) == 0) {
            return (int *) ptr;
        }
        if (*ptr == L'\0') {
            return _WSTR_NULL;
        }
    }
}

// Split the next token off str, or off *next if str is null, between set's.
int *_Wstr_SplitToken(int *str, const int *set, int **next)
{
    int *start = str != _WSTR_NULL ? str : *next;
    int *end;

    if (start == _WSTR_NULL) {
        return _WSTR_NULL;
    }
    start += _Wstr_CountListed(start, set);
    if (*start == L'\0') {
        *next = start;
        return _WSTR_NULL;
    }
    end = start + _Wstr_CountUnlisted(start, set);
    if (*end != L'\0') {
        *end = L'\0';
        end++;
    }
    *next = end;
    return start;
}

// Fill n wide characters of str with wc.
int *_Wstr_FillArray(int *str, int wc, unsigned long n)
{
    for (unsigned long i = 0; i < n; i++) {
        str[i] = wc;
    }
    return str;
}

// Get the length of the wide string str, the characters before its null.
unsigned long _Wstr_GetLength(const int *str)
{
    unsigned long len = 0;

    while (str[len] != L'\0') {
        len++;
    }
    return len;
}
