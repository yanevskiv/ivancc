// Includable helper file for <wchar.h> tests.

// Includes.
#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
#include <setjmp.h>
#include <assert.h>
#include <time.h>
#include <signal.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <wctype.h>
#include <wchar.h>

// Defines.
#define NONE    (-1)
#define SIZE    32
#define FILLED  9
#define WIDE(n) ((n) * sizeof(wchar_t))

// Fill a buffer's first wide characters by no function under test.
static void put(wchar_t *to, const wchar_t *from, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++) {
        to[i] = from[i];
    }
}

// Return how far into a wide string a search's result points, or NONE for null.
static long offset(const wchar_t *str, const wchar_t *found)
{
    return found == NULL ? NONE : found - str;
}

// Return -1, 0 or 1 as a comparison's result is negative, zero or positive.
static int sign(int value)
{
    return (value > 0) - (value < 0);
}

// Make a file of n bytes by no function under test, and open it to read.
static FILE *holding(const char *name, const char *bytes, size_t n)
{
    FILE *file = fopen(name, "wb");

    if (file == NULL || fwrite(bytes, 1, n, file) != n || fclose(file) != 0) {
        return NULL;
    }
    return fopen(name, "rb");
}

// A file should hold n bytes and no more, read by no function under test.
static bool holds(const char *name, const char *bytes, size_t n)
{
    int lost = 0;
    char buf[SIZE];
    size_t len = 0;
    FILE *file = fopen(name, "rb");

    if (! _TEST_TRUE(file != NULL)) {
        return false;
    }
    len = fread(buf, 1, sizeof(buf), file);
    lost += ! _TEST_UINT(len, n);
    lost += ! _TEST_MEM(buf, bytes, n < len ? n : len);
    lost += ! _TEST_INT(fclose(file), 0);
    return lost == 0;
}
