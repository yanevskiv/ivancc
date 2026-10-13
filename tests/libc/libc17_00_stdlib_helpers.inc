// Includable helper file for <stdlib.h> tests.

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

// Defines.
#define NONE      (-1)
#define SLOTS     100
#define SMALL     16
#define SORTED    2000
#define RANGE     1000
#define TEXT      16
#define DBL_LEAST (DBL_MIN / 4503599627370496.0)

// The strictest alignment of the basic types, which malloc's memory must meet.
struct Strictest {
    char st_tag;
    union {
        long double st_ld;
        long long st_ll;
        double st_dbl;
        void *st_ptr;
    } st_all;
};

// Objects.
static unsigned char *slot[SLOTS];
static unsigned long seed = 1;
static volatile size_t huge = SIZE_MAX;
static volatile double nought = 0.0;
static int sorted[SORTED];

// Return the next number of a sequence that is the same in every run.
static unsigned long next(void)
{
    seed = seed * 6364136223846793005UL + 1442695040888963407UL;
    return seed >> 33;
}

// Return how far a pointer is past the strictest alignment.
static unsigned long misaligned(const void *ptr)
{
    return (uintptr_t) ptr % offsetof(struct Strictest, st_all);
}

// Write n bytes that count up from tag.
static void fill(unsigned char *ptr, size_t n, unsigned char tag)
{
    size_t i;

    for (i = 0; i < n; i++) {
        ptr[i] = (unsigned char) (tag + i);
    }
}

// A block should count up from tag: return the first of n bytes that does not.
static long spoiled(const unsigned char *ptr, size_t n, unsigned char tag)
{
    size_t i;

    for (i = 0; i < n; i++) {
        if (ptr[i] != (unsigned char) (tag + i)) {
            return (long) i;
        }
    }
    return NONE;
}

// [C99/S7.20.1.3p4] strtod should read a string as a value, bit for bit, and stop.
static bool reads(const char *str, double value, long end)
{
    int lost = 0;
    char *stop = NULL;
    double got = strtod(str, &stop);

    lost += ! _TEST_FLOAT(got, value);
    lost += ! _TEST_INT(stop - str, end);
    return lost == 0;
}

// Order two ints.
static int compare_int(const void *left, const void *right)
{
    int x = *(const int *) left;
    int y = *(const int *) right;

    return (x > y) - (x < y);
}

// [C99/S7.20.5.2p2] sorted should ascend: return the first place that descends.
static int descent(void)
{
    int i;

    for (i = 1; i < SORTED; i++) {
        if (sorted[i - 1] > sorted[i]) {
            return i;
        }
    }
    return NONE;
}
