/*
 * C header file for extended multibyte and wide character utilities.
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

#ifndef _WCHAR_H
#define _WCHAR_H

// The bytes a conversion state holds, those of the longest character.
#define _WCHAR_STATE_BYTES 6

// (S7.24.1) Introduction
#define NULL ((void *) 0)

#define WCHAR_MIN (-2147483647 - 1)
#define WCHAR_MAX 2147483647

#ifndef WEOF
#define WEOF 0xFFFFFFFFU
#endif

#ifndef __SIZE_T__
#define __SIZE_T__
typedef unsigned long size_t;
#endif

#ifndef __WCHAR_T__
#define __WCHAR_T__
typedef int wchar_t;
#endif

#ifndef __WINT_T__
#define __WINT_T__
typedef unsigned int wint_t;
#endif

// (S7.24.1) Introduction
typedef struct {
    unsigned char bytes[_WCHAR_STATE_BYTES]; // a character begun, its bytes
    unsigned char count;                     // how many they are, 0 at first
} mbstate_t;

struct tm;

// (S7.24.4.2) Wide string copying functions
wchar_t *wcscpy(wchar_t *restrict str1, const wchar_t *restrict str2);
wchar_t *wcsncpy(wchar_t *restrict str1, const wchar_t *restrict str2, size_t n);
wchar_t *wmemcpy(wchar_t *restrict str1, const wchar_t *restrict str2, size_t n);
wchar_t *wmemmove(wchar_t *str1, const wchar_t *str2, size_t n);

// (S7.24.4.3) Wide string concatenation functions
wchar_t *wcscat(wchar_t *restrict str1, const wchar_t *restrict str2);
wchar_t *wcsncat(wchar_t *restrict str1, const wchar_t *restrict str2, size_t n);

// (S7.24.4.4) Wide string comparison functions
int    wcscmp(const wchar_t *str1, const wchar_t *str2);
int    wcscoll(const wchar_t *str1, const wchar_t *str2);
int    wcsncmp(const wchar_t *str1, const wchar_t *str2, size_t n);
size_t wcsxfrm(wchar_t *restrict str1, const wchar_t *restrict str2, size_t n);
int    wmemcmp(const wchar_t *str1, const wchar_t *str2, size_t n);

// (S7.24.4.5) Wide string search functions
wchar_t *wcschr(const wchar_t *str, wchar_t wc);
size_t   wcscspn(const wchar_t *str1, const wchar_t *str2);
wchar_t *wcspbrk(const wchar_t *str1, const wchar_t *str2);
wchar_t *wcsrchr(const wchar_t *str, wchar_t wc);
size_t   wcsspn(const wchar_t *str1, const wchar_t *str2);
wchar_t *wcsstr(const wchar_t *str1, const wchar_t *str2);
wchar_t *wcstok(wchar_t *restrict str1, const wchar_t *restrict str2, wchar_t **restrict ptr);
wchar_t *wmemchr(const wchar_t *str, wchar_t wc, size_t n);

// (S7.24.4.6) Miscellaneous functions
size_t   wcslen(const wchar_t *str);
wchar_t *wmemset(wchar_t *str, wchar_t wc, size_t n);

#endif // _WCHAR_H
