/*
 * C source file for string handling.
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
#include <string.h>

// The loops every function here stands on.
#include <_str.h>

// The texts strerror gives.
#include <_err.h>

// Where strtok's next search starts.
char *_String_Next;

// strerror's text for an unknown error number.
char _String_Unknown[_STRING_ERROR_SIZE];

// Format strerror's text for the unknown error number errnum.
char *_String_FormatUnknown(int errnum)
{
    char *ptr = _String_Unknown + _STRING_ERROR_SIZE - 1;
    unsigned int value = errnum < 0 ? 0U - (unsigned int) errnum : (unsigned int) errnum;
    size_t len = _Str_GetLength(_ERR_STRING_UNKNOWN);

    *ptr = '\0';
    do {
        ptr--;
        *ptr = (char) ('0' + value % _STRING_ERROR_BASE);
        value /= _STRING_ERROR_BASE;
    } while (value != 0);
    if (errnum < 0) {
        ptr--;
        *ptr = '-';
    }
    ptr -= len;
    _Str_CopyBytes(ptr, _ERR_STRING_UNKNOWN, len);
    return ptr;
}

// Copy n bytes from str2 to str1, which must not overlap.
void *memcpy(void *restrict str1, const void *restrict str2, size_t n)
{
    return _Str_CopyBytes(str1, str2, n);
}

// Copy n bytes from str2 to str1, which may overlap.
void *memmove(void *str1, const void *str2, size_t n)
{
    return _Str_MoveBytes(str1, str2, n);
}

// Copy the string str2 to str1.
char *strcpy(char *restrict str1, const char *restrict str2)
{
    return _Str_CopyString(str1, str2);
}

// Copy at most n characters of the string str2 to str1, padded to n with nulls.
char *strncpy(char *restrict str1, const char *restrict str2, size_t n)
{
    return _Str_CopyChars(str1, str2, n);
}

// Append the string str2 to str1.
char *strcat(char *restrict str1, const char *restrict str2)
{
    return _Str_AppendString(str1, str2);
}

// Append at most n characters of the string str2 to str1, and a null character.
char *strncat(char *restrict str1, const char *restrict str2, size_t n)
{
    return _Str_AppendChars(str1, str2, n);
}

// Compare n bytes of str1 and str2 as unsigned chars.
int memcmp(const void *str1, const void *str2, size_t n)
{
    return _Str_CompareBytes(str1, str2, n);
}

// Compare the strings str1 and str2 as unsigned chars.
int strcmp(const char *str1, const char *str2)
{
    return _Str_CompareStrings(str1, str2);
}

// Compare the strings str1 and str2 in the locale's collation order.
int strcoll(const char *str1, const char *str2)
{
    return _Str_CompareStrings(str1, str2);
}

// Compare at most n characters of the strings str1 and str2 as unsigned chars.
int strncmp(const char *str1, const char *str2, size_t n)
{
    return _Str_CompareChars(str1, str2, n);
}

// Transform the string str2 into str1, which strcmp orders as strcoll would.
size_t strxfrm(char *restrict str1, const char *restrict str2, size_t n)
{
    size_t len = _Str_GetLength(str2);

    if (len < n) {
        _Str_CopyBytes(str1, str2, len + 1);
    }
    return len;
}

// Find the first byte ch in n bytes of str.
void *memchr(const void *str, int ch, size_t n)
{
    return _Str_FindByte(str, ch, n);
}

// Find the first character ch in the string str, its null character included.
char *strchr(const char *str, int ch)
{
    return _Str_FindChar(str, ch);
}

// Count the characters at the start of the string str1 that are not in str2.
size_t strcspn(const char *str1, const char *str2)
{
    return _Str_CountUnlisted(str1, str2);
}

// Find the first character of the string str1 that is in str2.
char *strpbrk(const char *str1, const char *str2)
{
    return _Str_FindListed(str1, str2);
}

// Find the last character ch in the string str, its null character included.
char *strrchr(const char *str, int ch)
{
    return _Str_FindLastChar(str, ch);
}

// Count the characters at the start of the string str1 that are in str2.
size_t strspn(const char *str1, const char *str2)
{
    return _Str_CountListed(str1, str2);
}

// Find the first occurrence of the string str2 in str1.
char *strstr(const char *str1, const char *str2)
{
    return _Str_FindString(str1, str2);
}

// Split the string str1 into tokens between characters of str2, one per call.
char *strtok(char *restrict str1, const char *restrict str2)
{
    return _Str_SplitToken(str1, str2, &_String_Next);
}

// Set n bytes of str to the byte ch.
void *memset(void *str, int ch, size_t n)
{
    return _Str_FillBytes(str, ch, n);
}

// Name the error number errnum.
char *strerror(int errnum)
{
    const char *text = _Err_FindErrnoText(errnum);

    if (text == NULL) {
        return _String_FormatUnknown(errnum);
    }
    return (char *) text;
}

// Count the characters of the string str before its null character.
size_t strlen(const char *str)
{
    return _Str_GetLength(str);
}
