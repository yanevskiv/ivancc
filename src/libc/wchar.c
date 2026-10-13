/*
 * C source file for extended multibyte and wide character utilities.
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
#include <wchar.h>

// The loops the wide string functions stand on.
#include <_wstr.h>

// The longest character a conversion state holds the bytes of.
#include <_utf8.h>

// The limits of wchar_t the header gives again.
#include <stdint.h>

// Check the types _wstr counts and works in against the header's.
typedef char _Wchar_CheckSize[sizeof(unsigned long) == sizeof(size_t) ? 1 : -1];
typedef char _Wchar_CheckWide[sizeof(int) == sizeof(wchar_t) && (wchar_t) -1 < 0 ? 1 : -1];

// Check that a conversion state holds the bytes of the longest character.
typedef char _Wchar_CheckState[_WCHAR_STATE_BYTES == _UTF8_MAX ? 1 : -1];

// Copy the wide string str2 to str1.
wchar_t *wcscpy(wchar_t *restrict str1, const wchar_t *restrict str2)
{
    return _Wstr_CopyString(str1, str2);
}

// Copy at most n characters of the wide string str2 to str1, padded with nulls.
wchar_t *wcsncpy(wchar_t *restrict str1, const wchar_t *restrict str2, size_t n)
{
    return _Wstr_CopyChars(str1, str2, n);
}

// Copy n wide characters from str2 to str1, which must not overlap.
wchar_t *wmemcpy(wchar_t *restrict str1, const wchar_t *restrict str2, size_t n)
{
    return _Wstr_CopyArray(str1, str2, n);
}

// Copy n wide characters from str2 to str1, which may overlap.
wchar_t *wmemmove(wchar_t *str1, const wchar_t *str2, size_t n)
{
    return _Wstr_MoveArray(str1, str2, n);
}

// Append the wide string str2 to str1.
wchar_t *wcscat(wchar_t *restrict str1, const wchar_t *restrict str2)
{
    return _Wstr_AppendString(str1, str2);
}

// Append at most n characters of the wide string str2 to str1, and a null.
wchar_t *wcsncat(wchar_t *restrict str1, const wchar_t *restrict str2, size_t n)
{
    return _Wstr_AppendChars(str1, str2, n);
}

// Compare the wide strings str1 and str2.
int wcscmp(const wchar_t *str1, const wchar_t *str2)
{
    return _Wstr_CompareStrings(str1, str2);
}

// Compare the wide strings str1 and str2 in the locale's collation order.
int wcscoll(const wchar_t *str1, const wchar_t *str2)
{
    return _Wstr_CompareStrings(str1, str2);
}

// Compare at most n characters of the wide strings str1 and str2.
int wcsncmp(const wchar_t *str1, const wchar_t *str2, size_t n)
{
    return _Wstr_CompareChars(str1, str2, n);
}

// Transform the wide string str2 into str1, which wcscmp orders as wcscoll.
size_t wcsxfrm(wchar_t *restrict str1, const wchar_t *restrict str2, size_t n)
{
    size_t len = _Wstr_GetLength(str2);

    if (len < n) {
        _Wstr_CopyArray(str1, str2, len + 1);
    }
    return len;
}

// Compare n wide characters of str1 and str2.
int wmemcmp(const wchar_t *str1, const wchar_t *str2, size_t n)
{
    return _Wstr_CompareArrays(str1, str2, n);
}

// Find the first wc in the wide string str.
wchar_t *wcschr(const wchar_t *str, wchar_t wc)
{
    return _Wstr_FindChar(str, wc);
}

// Count the characters at the start of the wide string str1 not in str2.
size_t wcscspn(const wchar_t *str1, const wchar_t *str2)
{
    return _Wstr_CountUnlisted(str1, str2);
}

// Find the first character of the wide string str1 that is in str2.
wchar_t *wcspbrk(const wchar_t *str1, const wchar_t *str2)
{
    return _Wstr_FindListed(str1, str2);
}

// Find the last wc in the wide string str.
wchar_t *wcsrchr(const wchar_t *str, wchar_t wc)
{
    return _Wstr_FindLastChar(str, wc);
}

// Count the characters at the start of the wide string str1 that are in str2.
size_t wcsspn(const wchar_t *str1, const wchar_t *str2)
{
    return _Wstr_CountListed(str1, str2);
}

// Find the first occurrence of the wide string str2 in str1.
wchar_t *wcsstr(const wchar_t *str1, const wchar_t *str2)
{
    return _Wstr_FindString(str1, str2);
}

// Split the wide string str1 into tokens between characters of str2, by *ptr.
wchar_t *wcstok(wchar_t *restrict str1, const wchar_t *restrict str2, wchar_t **restrict ptr)
{
    return _Wstr_SplitToken(str1, str2, ptr);
}

// Find the first wc in n wide characters of str.
wchar_t *wmemchr(const wchar_t *str, wchar_t wc, size_t n)
{
    return _Wstr_FindInArray(str, wc, n);
}

// Get the length of the wide string str.
size_t wcslen(const wchar_t *str)
{
    return _Wstr_GetLength(str);
}

// Set n wide characters of str to wc.
wchar_t *wmemset(wchar_t *str, wchar_t wc, size_t n)
{
    return _Wstr_FillArray(str, wc, n);
}
