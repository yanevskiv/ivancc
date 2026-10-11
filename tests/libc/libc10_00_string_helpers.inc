// Includable helper file for <string.h> tests.

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

// Defines.
#define NONE   (-1)
#define SIZE   32
#define FILLED 9

// Fill a buffer's first bytes by no function under test.
static void put(char *to, const char *from, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++) {
        to[i] = from[i];
    }
}

// Return how far into a string a search's result points, or NONE for null.
static long offset(const char *str, const void *found)
{
    return found == NULL ? NONE : (const char *) found - str;
}
