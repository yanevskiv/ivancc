/*
 * C source file for general utilities.
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
#include <stdlib.h>

// The error a number out of range reports.
#include <errno.h>

// Copying memory.
#include <string.h>

// The heap malloc and its family allocate from.
#include <_mem.h>

// The signal mask abort clears and the exit.
#include <_sys.h>

// The environment getenv searches, and the hook exit flushes the streams by.
#include <_crt.h>

// The signal abort ends the program by.
#include <signal.h>

// The conversion strtod and its two stand on.
#include <_fp.h>

// The integer conversions strtol and its family share.
#include <inttypes.h>

// The encoding the multibyte functions follow, and its coding.
#include <_utf8.h>

// The state of rand, its taps and whether srand has seeded it.
unsigned int _Stdlib_RandState[_STDLIB_RAND_WORDS];
int _Stdlib_RandFront;
int _Stdlib_RandRear;
_Bool _Stdlib_RandSeeded;

// The first block of atexit's, C99's 32 without malloc, and the newest.
_Stdlib_Exits _Stdlib_ExitBase;
_Stdlib_Exits *_Stdlib_ExitTop = &_Stdlib_ExitBase;

// Pop the function atexit registered last, or return NULL when none is left.
void (*_Stdlib_PopExit(void))(void)
{
    _Stdlib_Exits *block = _Stdlib_ExitTop;

    while (block->se_count == 0 && block->se_next) {
        _Stdlib_ExitTop = block->se_next;
        free(block);
        block = _Stdlib_ExitTop;
    }
    if (block->se_count == 0) {
        return NULL;
    }
    block->se_count--;
    return block->se_func[block->se_count];
}

// Swap the size bytes at left with those at right.
void _Stdlib_SwapBytes(char *left, char *right, size_t size)
{
    char ch;
    size_t i;

    for (i = 0; i < size; i++) {
        ch = left[i];
        left[i] = right[i];
        right[i] = ch;
    }
}

// Reverse the order of nmemb elements of size bytes at base.
void _Stdlib_ReverseElements(char *base, size_t nmemb, size_t size)
{
    size_t i;

    for (i = 0; i < nmemb / 2; i++) {
        _Stdlib_SwapBytes(base + i * size, base + (nmemb - 1 - i) * size, size);
    }
}

// Rotate nmemb elements of size bytes at base so that element first leads.
void _Stdlib_RotateElements(char *base, size_t nmemb, size_t first, size_t size)
{
    _Stdlib_ReverseElements(base, first, size);
    _Stdlib_ReverseElements(base + first * size, nmemb - first, size);
    _Stdlib_ReverseElements(base, nmemb, size);
}

// Find the index of the first of nmemb sorted elements not below key.
size_t _Stdlib_FindLowerBound(const char *base, size_t nmemb, size_t size, const void *key, int (*compar)(const void *, const void *))
{
    size_t low = 0;
    size_t mid;
    size_t high = nmemb;

    while (low < high) {
        mid = low + (high - low) / 2;
        if (compar(base + mid * size, key) < 0) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return low;
}

// Find the index of the first of nmemb sorted elements above key.
size_t _Stdlib_FindUpperBound(const char *base, size_t nmemb, size_t size, const void *key, int (*compar)(const void *, const void *))
{
    size_t low = 0;
    size_t mid;
    size_t high = nmemb;

    while (low < high) {
        mid = low + (high - low) / 2;
        if (compar(key, base + mid * size) < 0) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }
    return low;
}

// Merge the sorted runs of left and right elements at base without a buffer.
void _Stdlib_MergeRunsInPlace(char *base, size_t left, size_t right, size_t size, int (*compar)(const void *, const void *))
{
    size_t cut1;
    size_t cut2;

    if (left == 0 || right == 0) {
        return;
    }
    if (left + right == 2) {
        if (compar(base + size, base) < 0) {
            _Stdlib_SwapBytes(base, base + size, size);
        }
        return;
    }
    if (left >= right) {
        cut1 = left / 2;
        cut2 = _Stdlib_FindLowerBound(base + left * size, right, size, base + cut1 * size, compar);
    } else {
        cut2 = right / 2;
        cut1 = _Stdlib_FindUpperBound(base, left, size, base + (left + cut2) * size, compar);
    }
    _Stdlib_RotateElements(base + cut1 * size, left - cut1 + cut2, left - cut1, size);
    _Stdlib_MergeRunsInPlace(base, cut1, cut2, size, compar);
    _Stdlib_MergeRunsInPlace(base + (cut1 + cut2) * size, left - cut1, right - cut2, size, compar);
}

// Merge the sorted runs of left and right elements at base through tmp.
void _Stdlib_MergeRuns(char *base, size_t left, size_t right, size_t size, int (*compar)(const void *, const void *), char *tmp)
{
    char *out = base;
    char *end = tmp + left * size;
    char *iter = tmp;
    char *next = base + left * size;
    char *stop = next + right * size;

    memcpy(tmp, base, left * size);
    while (iter < end && next < stop) {
        if (compar(next, iter) < 0) {
            memcpy(out, next, size);
            next += size;
        } else {
            memcpy(out, iter, size);
            iter += size;
        }
        out += size;
    }
    memcpy(out, iter, (size_t) (end - iter));
}

// Sort nmemb elements of size bytes at base stably, merging through tmp if any.
void _Stdlib_SortElements(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *), char *tmp)
{
    size_t half = nmemb / 2;
    char *mid = base + half * size;

    if (nmemb < 2) {
        return;
    }
    _Stdlib_SortElements(base, half, size, compar, tmp);
    _Stdlib_SortElements(mid, nmemb - half, size, compar, tmp);
    if (compar(mid, mid - size) >= 0) {
        return;
    }
    if (tmp) {
        _Stdlib_MergeRuns(base, half, nmemb - half, size, compar, tmp);
    } else {
        _Stdlib_MergeRunsInPlace(base, half, nmemb - half, size, compar);
    }
}

// Get the bytes of the longest character in LC_CTYPE's encoding.
size_t _Stdlib_GetMbCurMax(void)
{
    return _Utf8_GetCharMax();
}

// Convert the start of the string nptr to a double, as strtod does.
double atof(const char *nptr)
{
    return strtod(nptr, NULL);
}

// Convert the start of the string nptr to an int, as strtol in base 10 does.
int atoi(const char *nptr)
{
    return (int) strtol(nptr, NULL, 10);
}

// Convert the start of the string nptr to a long, as strtol in base 10 does.
long atol(const char *nptr)
{
    return strtol(nptr, NULL, 10);
}

// Convert the start of the string nptr to a long long, as strtoll does.
long long atoll(const char *nptr)
{
    return strtoll(nptr, NULL, 10);
}

// Convert the start of the string nptr to a double, correctly rounded.
double strtod(const char *restrict nptr, char **restrict endptr)
{
    size_t end;
    double value = _Fp_ToDouble(nptr, _FP_TEXT_NARROW, &end);

    if (endptr != NULL) {
        *endptr = (char *) nptr + end;
    }
    return value;
}

// Convert the start of the string nptr to a float, correctly rounded.
float strtof(const char *restrict nptr, char **restrict endptr)
{
    size_t end;
    float value = _Fp_ToFloat(nptr, _FP_TEXT_NARROW, &end);

    if (endptr != NULL) {
        *endptr = (char *) nptr + end;
    }
    return value;
}

// Convert the start of the string nptr to a long double, correctly rounded.
long double strtold(const char *restrict nptr, char **restrict endptr)
{
    size_t end;
    long double value = _Fp_ToLongDouble(nptr, _FP_TEXT_NARROW, &end);

    if (endptr != NULL) {
        *endptr = (char *) nptr + end;
    }
    return value;
}

// Convert the start of the string nptr to a long in base.
long strtol(const char *restrict nptr, char **restrict endptr, int base)
{
    return strtoimax(nptr, endptr, base);
}

// Convert the start of the string nptr to a long long in base.
long long strtoll(const char *restrict nptr, char **restrict endptr, int base)
{
    return strtoimax(nptr, endptr, base);
}

// Convert the start of the string nptr to an unsigned long in base.
unsigned long strtoul(const char *restrict nptr, char **restrict endptr, int base)
{
    return strtoumax(nptr, endptr, base);
}

// Convert the start of the string nptr to an unsigned long long in base.
unsigned long long strtoull(const char *restrict nptr, char **restrict endptr, int base)
{
    return strtoumax(nptr, endptr, base);
}

// Give the next number of the sequence srand seeded, as glibc's.
int rand(void)
{
    unsigned int value;
    unsigned int *state = _Stdlib_RandState;

    if (! _Stdlib_RandSeeded) {
        srand(1);
    }
    state[_Stdlib_RandFront] += state[_Stdlib_RandRear];
    value = state[_Stdlib_RandFront];
    _Stdlib_RandFront = (_Stdlib_RandFront + 1) % _STDLIB_RAND_WORDS;
    _Stdlib_RandRear = (_Stdlib_RandRear + 1) % _STDLIB_RAND_WORDS;
    return (int) (value >> 1);
}

// Seed the sequence rand gives, 0 as 1, as glibc's.
void srand(unsigned int seed)
{
    int word = seed == 0 ? 1 : (int) seed;
    int i;

    _Stdlib_RandState[0] = (unsigned int) word;
    for (i = 1; i < _STDLIB_RAND_WORDS; i++) {
        word = _STDLIB_RAND_MUL * (word % _STDLIB_RAND_QUO) - _STDLIB_RAND_REM * (word / _STDLIB_RAND_QUO);
        if (word < 0) {
            word += _STDLIB_RAND_MOD;
        }
        _Stdlib_RandState[i] = (unsigned int) word;
    }
    _Stdlib_RandFront = _STDLIB_RAND_SEP;
    _Stdlib_RandRear = 0;
    _Stdlib_RandSeeded = 1;
    for (i = 0; i < _STDLIB_RAND_DISCARD; i++) {
        rand();
    }
}

// Allocate nmemb objects of size bytes, all bits zero.
void *calloc(size_t nmemb, size_t size)
{
    return _Mem_AllocateObjects(nmemb, size);
}

// Free the memory at ptr, which malloc, calloc or realloc gave.
void free(void *ptr)
{
    _Mem_FreeMemory(ptr);
}

// Allocate size bytes, or return NULL with errno ENOMEM, as glibc does.
void *malloc(size_t size)
{
    return _Mem_AllocateMemory(size);
}

// Resize the memory at ptr to size bytes; a size of 0 frees it, as glibc's.
void *realloc(void *ptr, size_t size)
{
    return _Mem_ResizeMemory(ptr, size);
}

// End the program by SIGABRT past a returning handler or a block, as glibc's.
void abort(void)
{
    unsigned long set = 1UL << (SIGABRT - _SYS_SIGNAL_FIRST);

    raise(SIGABRT);
    signal(SIGABRT, SIG_DFL);
    _Sys_InvokeRtSigprocmask(_SYS_SIG_UNBLOCK, &set, NULL);
    raise(SIGABRT);
    _Sys_InvokeExitGroup(_STDLIB_ABORT_STATUS);
}

// Register func for exit to call, or return -1 out of memory, as glibc's.
int atexit(void (*func)(void))
{
    _Stdlib_Exits *block = _Stdlib_ExitTop;

    if (block->se_count == _STDLIB_EXITS) {
        block = malloc(sizeof(*block));
        if (! block) {
            return -1;
        }
        block->se_next = _Stdlib_ExitTop;
        block->se_count = 0;
        _Stdlib_ExitTop = block;
    }
    block->se_func[block->se_count] = func;
    block->se_count++;
    return 0;
}

// Call atexit's functions, newest first, then flush the streams and end.
void exit(int status)
{
    void (*func)(void);

    while ((func = _Stdlib_PopExit()) != NULL) {
        func();
    }
    if (_Crt_FlushStreams != NULL) {
        _Crt_FlushStreams();
    }
    _Exit(status);
}

// End the program with status, calling nothing atexit registered.
void _Exit(int status)
{
    _Sys_InvokeExitGroup(status);
}

// Return the value of the environment variable name, or NULL, as glibc's.
char *getenv(const char *name)
{
    size_t len = strlen(name);
    char **env;

    if (_Crt_Envp == NULL || len == 0) {
        return NULL;
    }
    for (env = _Crt_Envp; *env; env++) {
        if (strncmp(*env, name, len) == 0 && (*env)[len] == '=') {
            return *env + len + 1;
        }
    }
    return NULL;
}

// Report no command processor, so the emulator needs no fork or exec.
int system(const char *string)
{
    if (string == NULL) {
        return 0;
    }
    errno = _SYS_ENOSYS;
    return -1;
}

// Find an object of nmemb sorted at base that matches key, as glibc's does.
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    int cmp;
    size_t low = 0;
    size_t mid;
    size_t high = nmemb;
    const char *ptr;

    while (low < high) {
        mid = low + (high - low) / 2;
        ptr = (const char *) base + mid * size;
        cmp = compar(key, ptr);
        if (cmp == 0) {
            return (void *) ptr;
        }
        if (cmp < 0) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }
    return NULL;
}

// Sort nmemb objects of size bytes at base, keeping equal ones in order.
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    int saved = errno;
    size_t need = nmemb / 2 * size;
    char stack[_STDLIB_SORT_STACK];
    char *tmp = stack;

    if (need > sizeof(stack)) {
        tmp = malloc(need);
        errno = saved;
    }
    _Stdlib_SortElements(base, nmemb, size, compar, tmp);
    if (tmp != stack) {
        free(tmp);
    }
}

// Give the absolute value of j.
int abs(int j)
{
    return j < 0 ? -j : j;
}

// Give the absolute value of j.
long labs(long j)
{
    return j < 0 ? -j : j;
}

// Give the absolute value of j.
long long llabs(long long j)
{
    return j < 0 ? -j : j;
}

// Divide numer by denom, giving the quotient and the remainder.
div_t div(int numer, int denom)
{
    div_t result;

    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

// Divide numer by denom, giving the quotient and the remainder.
ldiv_t ldiv(long numer, long denom)
{
    ldiv_t result;

    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

// Divide numer by denom, giving the quotient and the remainder.
lldiv_t lldiv(long long numer, long long denom)
{
    lldiv_t result;

    result.quot = numer / denom;
    result.rem = numer % denom;
    return result;
}

// Give the length of the character in the n bytes at str, as mbtowc does.
int mblen(const char *str, size_t n)
{
    return mbtowc(NULL, str, n);
}

// Convert the character in the n bytes at str into *pwc, and give its length.
int mbtowc(wchar_t *restrict pwc, const char *restrict str, size_t n)
{
    int len;
    wchar_t wc;

    if (str == NULL) {
        return 0;
    }
    len = _Utf8_DecodeChar(str, n, &wc);
    if (len < 0) {
        if (len == _UTF8_INVALID) {
            errno = EILSEQ;
        }
        return -1;
    }
    if (pwc != NULL) {
        *pwc = wc;
    }
    return wc == 0 ? 0 : len;
}

// Convert wc to its bytes at str, and give their count, or -1 for none.
int wctomb(char *str, wchar_t wc)
{
    int len;

    if (str == NULL) {
        return 0;
    }
    len = _Utf8_EncodeChar(wc, str);
    if (len < 0) {
        errno = EILSEQ;
        return -1;
    }
    return len;
}

// Convert the multibyte string str to at most n wide characters at pwcs.
size_t mbstowcs(wchar_t *restrict pwcs, const char *restrict str, size_t n)
{
    int len;
    wchar_t wc;
    size_t count = 0;

    while (pwcs == NULL || count < n) {
        len = _Utf8_DecodeChar(str, _UTF8_MAX, &wc);
        if (len < 0) {
            errno = EILSEQ;
            return (size_t) -1;
        }
        if (pwcs != NULL) {
            pwcs[count] = wc;
        }
        if (wc == 0) {
            break;
        }
        str += len;
        count++;
    }
    return count;
}

// Convert the wide string pwcs to at most n bytes of a multibyte string at str.
size_t wcstombs(char *restrict str, const wchar_t *restrict pwcs, size_t n)
{
    int len;
    size_t count = 0;
    char buf[_UTF8_MAX];

    for (;; pwcs++) {
        if (str != NULL && count == n) {
            break;
        }
        len = _Utf8_EncodeChar(*pwcs, buf);
        if (len < 0) {
            errno = EILSEQ;
            return (size_t) -1;
        }
        if (str != NULL && (size_t) len > n - count) {
            break;
        }
        if (str != NULL) {
            memcpy(str + count, buf, (size_t) len);
        }
        if (*pwcs == 0) {
            break;
        }
        count += (size_t) len;
    }
    return count;
}
