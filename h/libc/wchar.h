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

// The streams, which wchar.h may not name as stdio.h does.
#include <_file.h>

// The bytes a conversion state holds, those of the longest character.
#define _WCHAR_STATE_BYTES 4

// What a conversion gives for a character that is invalid or cut short.
#define _WCHAR_INVALID    ((size_t) -1)
#define _WCHAR_INCOMPLETE ((size_t) -2)

// The sizes of one conversion of wcsftime's format and of what it writes.
#define _WCHAR_FORMAT_SIZE 4
#define _WCHAR_FIELD_SIZE  64

// [C99:S7.24.1] Introduction
#define NULL ((void *) 0)

#define WCHAR_MIN (-2147483647 - 1)
#define WCHAR_MAX 2147483647

#ifndef WEOF
#define WEOF 0xFFFFFFFFU
#endif

// The argument list, which wchar.h may not name.
typedef __builtin_va_list _Wchar_VaList;

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

// [C99:S7.24.1] Introduction
typedef struct {
    unsigned char bytes[_WCHAR_STATE_BYTES]; // a character begun, its bytes
    unsigned char count;                     // how many they are, 0 at first
} mbstate_t;

struct tm;

// The state each restartable function converts in when it is given none.
extern mbstate_t _Wchar_MbrlenState;
extern mbstate_t _Wchar_MbrtowcState;
extern mbstate_t _Wchar_WcrtombState;
extern mbstate_t _Wchar_MbsrtowcsState;
extern mbstate_t _Wchar_WcsrtombsState;

// Restartable conversions
size_t _Wchar_DecodeChar(wchar_t *pwc, const char *str, size_t n, mbstate_t *state);
size_t _Wchar_EncodeChar(char *str, wchar_t wc, mbstate_t *state);

// [C99:S7.24.2] Formatted wide character input/output functions
int fwprintf(_File_Stream *restrict stream, const wchar_t *restrict format, ...);
int fwscanf(_File_Stream *restrict stream, const wchar_t *restrict format, ...);
int swprintf(wchar_t *restrict str, size_t n, const wchar_t *restrict format, ...);
int swscanf(const wchar_t *restrict str, const wchar_t *restrict format, ...);
int vfwprintf(_File_Stream *restrict stream, const wchar_t *restrict format, _Wchar_VaList arg);
int vfwscanf(_File_Stream *restrict stream, const wchar_t *restrict format, _Wchar_VaList arg);
int vswprintf(wchar_t *restrict str, size_t n, const wchar_t *restrict format, _Wchar_VaList arg);
int vswscanf(const wchar_t *restrict str, const wchar_t *restrict format, _Wchar_VaList arg);
int vwprintf(const wchar_t *restrict format, _Wchar_VaList arg);
int vwscanf(const wchar_t *restrict format, _Wchar_VaList arg);
int wprintf(const wchar_t *restrict format, ...);
int wscanf(const wchar_t *restrict format, ...);

// [C99:S7.24.3] Wide character input/output functions
wint_t   fgetwc(_File_Stream *stream);
wchar_t *fgetws(wchar_t *restrict str, int n, _File_Stream *restrict stream);
wint_t   fputwc(wchar_t wc, _File_Stream *stream);
int      fputws(const wchar_t *restrict str, _File_Stream *restrict stream);
int      fwide(_File_Stream *stream, int mode);
wint_t   getwc(_File_Stream *stream);
wint_t   getwchar(void);
wint_t   putwc(wchar_t wc, _File_Stream *stream);
wint_t   putwchar(wchar_t wc);
wint_t   ungetwc(wint_t wc, _File_Stream *stream);

// [C99:S7.24.4.1] Wide string numeric conversion functions
double             wcstod(const wchar_t *restrict nptr, wchar_t **restrict endptr);
float              wcstof(const wchar_t *restrict nptr, wchar_t **restrict endptr);
long double        wcstold(const wchar_t *restrict nptr, wchar_t **restrict endptr);
long               wcstol(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base);
long long          wcstoll(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base);
unsigned long      wcstoul(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base);
unsigned long long wcstoull(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base);

// [C99:S7.24.4.2] Wide string copying functions
wchar_t *wcscpy(wchar_t *restrict str1, const wchar_t *restrict str2);
wchar_t *wcsncpy(wchar_t *restrict str1, const wchar_t *restrict str2, size_t n);
wchar_t *wmemcpy(wchar_t *restrict str1, const wchar_t *restrict str2, size_t n);
wchar_t *wmemmove(wchar_t *str1, const wchar_t *str2, size_t n);

// [C99:S7.24.4.3] Wide string concatenation functions
wchar_t *wcscat(wchar_t *restrict str1, const wchar_t *restrict str2);
wchar_t *wcsncat(wchar_t *restrict str1, const wchar_t *restrict str2, size_t n);

// [C99:S7.24.4.4] Wide string comparison functions
int    wcscmp(const wchar_t *str1, const wchar_t *str2);
int    wcscoll(const wchar_t *str1, const wchar_t *str2);
int    wcsncmp(const wchar_t *str1, const wchar_t *str2, size_t n);
size_t wcsxfrm(wchar_t *restrict str1, const wchar_t *restrict str2, size_t n);
int    wmemcmp(const wchar_t *str1, const wchar_t *str2, size_t n);

// [C99:S7.24.4.5] Wide string search functions
wchar_t *wcschr(const wchar_t *str, wchar_t wc);
size_t   wcscspn(const wchar_t *str1, const wchar_t *str2);
wchar_t *wcspbrk(const wchar_t *str1, const wchar_t *str2);
wchar_t *wcsrchr(const wchar_t *str, wchar_t wc);
size_t   wcsspn(const wchar_t *str1, const wchar_t *str2);
wchar_t *wcsstr(const wchar_t *str1, const wchar_t *str2);
wchar_t *wcstok(wchar_t *restrict str1, const wchar_t *restrict str2, wchar_t **restrict ptr);
wchar_t *wmemchr(const wchar_t *str, wchar_t wc, size_t n);

// [C99:S7.24.4.6] Miscellaneous functions
size_t   wcslen(const wchar_t *str);
wchar_t *wmemset(wchar_t *str, wchar_t wc, size_t n);

// [C99:S7.24.5] Wide character time conversion functions
size_t wcsftime(wchar_t *restrict str, size_t maxsize, const wchar_t *restrict format, const struct tm *restrict timeptr);

// [C99:S7.24.6.1] Single-byte/wide character conversion functions
wint_t btowc(int ch);
int    wctob(wint_t wc);

// [C99:S7.24.6.2] Conversion state functions
int mbsinit(const mbstate_t *ps);

// [C99:S7.24.6.3] Restartable multibyte/wide character conversion functions
size_t mbrlen(const char *restrict str, size_t n, mbstate_t *restrict ps);
size_t mbrtowc(wchar_t *restrict pwc, const char *restrict str, size_t n, mbstate_t *restrict ps);
size_t wcrtomb(char *restrict str, wchar_t wc, mbstate_t *restrict ps);

// [C99:S7.24.6.4] Restartable multibyte/wide string conversion functions
size_t mbsrtowcs(wchar_t *restrict dst, const char **restrict src, size_t len, mbstate_t *restrict ps);
size_t wcsrtombs(char *restrict dst, const wchar_t **restrict src, size_t len, mbstate_t *restrict ps);

#endif // _WCHAR_H
