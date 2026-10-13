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

// The copies of a conversion's bytes.
#include <_str.h>

// The error a conversion reports.
#include <errno.h>

// EOF, which btowc takes and wctob gives.
#include <stdio.h>

// The conversion wcstod and its two stand on.
#include <_fp.h>

// The integer conversions wcstol and its family share.
#include <inttypes.h>

// The ranges the checks hold long and long long to.
#include <limits.h>

// Check the types _wstr counts and works in against the header's.
typedef char _Wchar_CheckSize[sizeof(unsigned long) == sizeof(size_t) ? 1 : -1];
typedef char _Wchar_CheckWide[sizeof(int) == sizeof(wchar_t) && (wchar_t) -1 < 0 ? 1 : -1];

// Check that a conversion state holds the bytes of the longest character.
typedef char _Wchar_CheckState[_WCHAR_STATE_BYTES == _UTF8_MAX ? 1 : -1];

// Check that long and long long are intmax_t, as wcstol and its family take.
typedef char _Wchar_CheckLong[LONG_MAX == INTMAX_MAX && LLONG_MAX == INTMAX_MAX && ULONG_MAX == UINTMAX_MAX && ULLONG_MAX == UINTMAX_MAX ? 1 : -1];

// The state each restartable function converts in when it is given none.
mbstate_t _Wchar_MbrlenState;
mbstate_t _Wchar_MbrtowcState;
mbstate_t _Wchar_WcrtombState;
mbstate_t _Wchar_MbsrtowcsState;
mbstate_t _Wchar_WcsrtombsState;

// Decode the character state's bytes and the n at str make, and count str's.
size_t _Wchar_DecodeChar(wchar_t *pwc, const char *str, size_t n, mbstate_t *state)
{
    int len;
    wchar_t wc;
    size_t taken;
    size_t held = state->count;
    char buf[_WCHAR_STATE_BYTES];

    // Phase: join the bytes begun with as many new ones as a character takes
    if (str == NULL) {
        pwc = NULL;
        str = "";
        n = 1;
    }
    taken = n < _WCHAR_STATE_BYTES - held ? n : _WCHAR_STATE_BYTES - held;
    _Str_CopyBytes(buf, state->bytes, held);
    _Str_CopyBytes(buf + held, str, taken);

    // Phase: decode them, and keep them if they are cut short
    len = _Utf8_DecodeChar(buf, held + taken, &wc);
    if (len == _UTF8_INVALID) {
        errno = EILSEQ;
        return _WCHAR_INVALID;
    }
    if (len == _UTF8_INCOMPLETE) {
        _Str_CopyBytes(state->bytes, buf, held + taken);
        state->count = (unsigned char) (held + taken);
        return _WCHAR_INCOMPLETE;
    }
    state->count = 0;
    if (pwc != NULL) {
        *pwc = wc;
    }
    return wc == 0 ? 0 : (size_t) len - held;
}

// Encode wc at str, or a null character at none, and give its length.
size_t _Wchar_EncodeChar(char *str, wchar_t wc, mbstate_t *state)
{
    int len;
    char buf[_WCHAR_STATE_BYTES];

    if (str == NULL) {
        str = buf;
        wc = 0;
    }
    len = _Utf8_EncodeChar(wc, str);
    if (len == _UTF8_INVALID) {
        errno = EILSEQ;
        return _WCHAR_INVALID;
    }
    if (wc == 0) {
        state->count = 0;
    }
    return (size_t) len;
}

// Convert the start of the wide string nptr to a double.
double wcstod(const wchar_t *restrict nptr, wchar_t **restrict endptr)
{
    size_t end;
    double value = _Fp_ToDouble(nptr, _FP_TEXT_WIDE, &end);

    if (endptr != NULL) {
        *endptr = (wchar_t *) nptr + end;
    }
    return value;
}

// Convert the start of the wide string nptr to a float.
float wcstof(const wchar_t *restrict nptr, wchar_t **restrict endptr)
{
    size_t end;
    float value = _Fp_ToFloat(nptr, _FP_TEXT_WIDE, &end);

    if (endptr != NULL) {
        *endptr = (wchar_t *) nptr + end;
    }
    return value;
}

// Convert the start of the wide string nptr to a long double.
long double wcstold(const wchar_t *restrict nptr, wchar_t **restrict endptr)
{
    size_t end;
    long double value = _Fp_ToLongDouble(nptr, _FP_TEXT_WIDE, &end);

    if (endptr != NULL) {
        *endptr = (wchar_t *) nptr + end;
    }
    return value;
}

// Convert the start of the wide string nptr to a long in base.
long wcstol(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base)
{
    return wcstoimax(nptr, endptr, base);
}

// Convert the start of the wide string nptr to a long long in base.
long long wcstoll(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base)
{
    return wcstoimax(nptr, endptr, base);
}

// Convert the start of the wide string nptr to an unsigned long in base.
unsigned long wcstoul(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base)
{
    return wcstoumax(nptr, endptr, base);
}

// Convert the start of the wide string nptr to an unsigned long long in base.
unsigned long long wcstoull(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base)
{
    return wcstoumax(nptr, endptr, base);
}

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

// Give the wide character the one byte ch is, or WEOF if it is none.
wint_t btowc(int ch)
{
    wchar_t wc;
    char byte = (char) ch;

    if (ch == EOF || _Utf8_DecodeChar(&byte, 1, &wc) != _UTF8_ASCII_LEN) {
        return WEOF;
    }
    return (wint_t) wc;
}

// Give the one byte wc is, or EOF if it takes more or has none.
int wctob(wint_t wc)
{
    char buf[_WCHAR_STATE_BYTES];

    if (_Utf8_EncodeChar((wchar_t) wc, buf) != _UTF8_ASCII_LEN) {
        return EOF;
    }
    return (unsigned char) buf[0];
}

// Check whether ps is none or describes an initial conversion state.
int mbsinit(const mbstate_t *ps)
{
    return ps == NULL || ps->count == 0;
}

// Give the length of the character in the n bytes at str, as mbrtowc does.
size_t mbrlen(const char *restrict str, size_t n, mbstate_t *restrict ps)
{
    return _Wchar_DecodeChar(NULL, str, n, ps != NULL ? ps : &_Wchar_MbrlenState);
}

// Convert the character *ps and the n bytes at str make into *pwc.
size_t mbrtowc(wchar_t *restrict pwc, const char *restrict str, size_t n, mbstate_t *restrict ps)
{
    return _Wchar_DecodeChar(pwc, str, n, ps != NULL ? ps : &_Wchar_MbrtowcState);
}

// Convert wc to its bytes at str, and give their count.
size_t wcrtomb(char *restrict str, wchar_t wc, mbstate_t *restrict ps)
{
    return _Wchar_EncodeChar(str, wc, ps != NULL ? ps : &_Wchar_WcrtombState);
}

// Convert the multibyte string *src to at most len wide characters at dst.
size_t mbsrtowcs(wchar_t *restrict dst, const char **restrict src, size_t len, mbstate_t *restrict ps)
{
    wchar_t wc;
    size_t used;
    size_t count = 0;
    const char *str = *src;
    const char *first = *src;
    mbstate_t *state = ps != NULL ? ps : &_Wchar_MbsrtowcsState;
    mbstate_t copy = *state;

    if (dst == NULL) {
        state = &copy;
    }
    while (dst == NULL || count < len) {
        used = _Wchar_DecodeChar(&wc, str, 1, state);
        if (used == _WCHAR_INVALID) {
            count = _WCHAR_INVALID;
            str = first;
            break;
        }
        str++;
        if (used == _WCHAR_INCOMPLETE) {
            continue;
        }
        if (dst != NULL) {
            dst[count] = wc;
        }
        if (wc == 0) {
            str = NULL;
            break;
        }
        first = str;
        count++;
    }
    if (dst != NULL) {
        *src = str;
    }
    return count;
}

// Convert the wide string *src to at most len bytes of a multibyte one at dst.
size_t wcsrtombs(char *restrict dst, const wchar_t **restrict src, size_t len, mbstate_t *restrict ps)
{
    size_t used;
    size_t count = 0;
    const wchar_t *str = *src;
    char buf[_WCHAR_STATE_BYTES];
    mbstate_t *state = ps != NULL ? ps : &_Wchar_WcsrtombsState;
    mbstate_t copy = *state;

    if (dst == NULL) {
        state = &copy;
    }
    for (;; str++) {
        used = _Wchar_EncodeChar(buf, *str, state);
        if (used == _WCHAR_INVALID) {
            count = _WCHAR_INVALID;
            break;
        }
        if (dst != NULL && used > len - count) {
            break;
        }
        if (dst != NULL) {
            _Str_CopyBytes(dst + count, buf, used);
        }
        if (*str == 0) {
            str = NULL;
            break;
        }
        count += used;
    }
    if (dst != NULL) {
        *src = str;
    }
    return count;
}
