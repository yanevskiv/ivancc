/*
 * C header file for general utilities.
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

#ifndef _STDLIB_H
#define _STDLIB_H

// The words of rand's state, the gap of its taps and the outputs srand skips.
#define _STDLIB_RAND_WORDS   31
#define _STDLIB_RAND_SEP     3
#define _STDLIB_RAND_DISCARD 310

// The multiplier, modulus, quotient and remainder srand's generator takes.
#define _STDLIB_RAND_MUL 16807
#define _STDLIB_RAND_MOD 2147483647
#define _STDLIB_RAND_QUO 127773
#define _STDLIB_RAND_REM 2836

// The functions a block of atexit's holds, C99's least [C99 S7.20.4.2p2].
#define _STDLIB_EXITS 32

// The status abort exits with if SIGABRT did not end the program, as glibc's.
#define _STDLIB_ABORT_STATUS 127

// The bytes of the merge buffer qsort keeps on the stack, as glibc's.
#define _STDLIB_SORT_STACK 1024

// [C99 S7.20] General utilities
#define NULL ((void *) 0)

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define RAND_MAX 2147483647

#define MB_CUR_MAX (_Stdlib_GetMbCurMax())

#ifndef __SIZE_T__
#define __SIZE_T__
typedef unsigned long size_t;
#endif

#ifndef __WCHAR_T__
#define __WCHAR_T__
typedef int wchar_t;
#endif

// A block of the functions atexit registered, and the block before it.
typedef struct _Stdlib_Exits _Stdlib_Exits;
struct _Stdlib_Exits {
    _Stdlib_Exits *se_next;               // the block filled before this one
    int se_count;                         // the functions in this block
    void (*se_func[_STDLIB_EXITS])(void); // the functions, oldest first
};

// [C99 S7.20] General utilities
typedef struct {
    int quot; // the quotient
    int rem;  // the remainder
} div_t;

typedef struct {
    long quot; // the quotient
    long rem;  // the remainder
} ldiv_t;

typedef struct {
    long long quot; // the quotient
    long long rem;  // the remainder
} lldiv_t;

// The state of rand, its taps and whether srand has seeded it.
extern unsigned int _Stdlib_RandState[_STDLIB_RAND_WORDS];
extern int _Stdlib_RandFront;
extern int _Stdlib_RandRear;
extern _Bool _Stdlib_RandSeeded;

// The first block of atexit's, and the newest.
extern _Stdlib_Exits _Stdlib_ExitBase;
extern _Stdlib_Exits *_Stdlib_ExitTop;

// Exit
void (*_Stdlib_PopExit(void))(void);

// Sorting
void   _Stdlib_SwapBytes(char *left, char *right, size_t size);
void   _Stdlib_ReverseElements(char *base, size_t nmemb, size_t size);
void   _Stdlib_RotateElements(char *base, size_t nmemb, size_t first, size_t size);
size_t _Stdlib_FindLowerBound(const char *base, size_t nmemb, size_t size, const void *key, int (*compar)(const void *, const void *));
size_t _Stdlib_FindUpperBound(const char *base, size_t nmemb, size_t size, const void *key, int (*compar)(const void *, const void *));
void   _Stdlib_MergeRunsInPlace(char *base, size_t left, size_t right, size_t size, int (*compar)(const void *, const void *));
void   _Stdlib_MergeRuns(char *base, size_t left, size_t right, size_t size, int (*compar)(const void *, const void *), char *tmp);
void   _Stdlib_SortElements(char *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *), char *tmp);

// Multibyte
size_t _Stdlib_GetMbCurMax(void);

// [C99 S7.20.1] Numeric conversion functions
double             atof(const char *nptr);
int                atoi(const char *nptr);
long               atol(const char *nptr);
long long          atoll(const char *nptr);
double             strtod(const char *restrict nptr, char **restrict endptr);
float              strtof(const char *restrict nptr, char **restrict endptr);
long double        strtold(const char *restrict nptr, char **restrict endptr);
long               strtol(const char *restrict nptr, char **restrict endptr, int base);
long long          strtoll(const char *restrict nptr, char **restrict endptr, int base);
unsigned long      strtoul(const char *restrict nptr, char **restrict endptr, int base);
unsigned long long strtoull(const char *restrict nptr, char **restrict endptr, int base);

// [C99 S7.20.2] Pseudo-random sequence generation functions
int  rand(void);
void srand(unsigned int seed);

// [C99 S7.20.3] Memory management functions
void *calloc(size_t nmemb, size_t size);
void  free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);

// [C99 S7.20.4] Communication with the environment
void  abort(void);
int   atexit(void (*func)(void));
void  exit(int status);
void  _Exit(int status);
char *getenv(const char *name);
int   system(const char *string);

// [C99 S7.20.5] Searching and sorting utilities
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
void  qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

// [C99 S7.20.6] Integer arithmetic functions
int       abs(int j);
long      labs(long j);
long long llabs(long long j);
div_t     div(int numer, int denom);
ldiv_t    ldiv(long numer, long denom);
lldiv_t   lldiv(long long numer, long long denom);

// [C99 S7.20.7] Multibyte/wide character conversion functions
int mblen(const char *str, size_t n);
int mbtowc(wchar_t *restrict pwc, const char *restrict str, size_t n);
int wctomb(char *str, wchar_t wc);

// [C99 S7.20.8] Multibyte/wide string conversion functions
size_t mbstowcs(wchar_t *restrict pwcs, const char *restrict str, size_t n);
size_t wcstombs(char *restrict str, const wchar_t *restrict pwcs, size_t n);

#endif // _STDLIB_H
