// Includable helper file for <stdio.h> tests.

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

// Defines.
#define NONE   (-1)
#define SMALL  16
#define LINE   32
#define HOLD   64
#define PRIME  251
#define WIDE   (BUFSIZ + 100)
#define NAMES  25
#define BYTES  8
#define CTRL_Z 26
#define FILL   '#'
#define LONGER "a line longer than the buffer it passes through\n"
#define DIGITS "0123456789abcdefghijklmnopqrstuvwxyz"
#define UTF8   "C.UTF-8"

// Bytes a text stream might change, which a binary one must not.
static const unsigned char bytes[BYTES] = {0, UCHAR_MAX, '\n', '\r', '\n', CTRL_Z, 'a', 0};

// Objects.
static unsigned char wide[WIDE];

// A file opened in a mode should take a string and close.
// [C99:7.19.5.3p3]: fopen's modes
static bool put(const char *name, const char *mode, const char *str)
{
    int lost = 0;
    FILE *file = fopen(name, mode);

    if (! _TEST_TRUE(file != NULL)) {
        return false;
    }
    lost += ! _TEST_TRUE(fputs(str, file) >= 0);
    lost += ! _TEST_INT(fclose(file), 0);
    return lost == 0;
}

// A file should hold a string and no more.
static bool holds(const char *name, const char *str)
{
    int lost = 0;
    size_t len;
    FILE *file = fopen(name, "r");
    char got[HOLD];

    if (! _TEST_TRUE(file != NULL)) {
        return false;
    }
    len = fread(got, 1, sizeof(got) - 1, file);
    got[len] = '\0';
    lost += ! _TEST_STR(got, str);
    lost += ! _TEST_TRUE(feof(file));
    lost += ! _TEST_TRUE(! ferror(file));
    lost += ! _TEST_INT(fclose(file), 0);
    return lost == 0;
}

// fgets should read a stream's next line, or what is left.
// [C99:7.19.7.2p2]: fgets, at most n - 1 characters through a new-line
static bool reads(FILE *file, const char *want)
{
    int lost = 0;
    char got[HOLD];

    got[0] = '\0';
    lost += ! _TEST_TRUE(fgets(got, sizeof(got), file) == got);
    lost += ! _TEST_STR(got, want);
    return lost == 0;
}

// Give each byte of the wide block a value its place alone decides.
static void pattern(void)
{
    size_t k;

    for (k = 0; k < sizeof(wide); k++) {
        wide[k] = (unsigned char) (k % PRIME);
    }
}

// A format should give its text and return the text's length.
// [C99:7.19.6.12p3]: vsnprintf's count, as if n were large enough
static bool prints(const char *want, const char *format, ...)
{
    int n;
    int lost = 0;
    va_list args;
    char got[HOLD];

    va_start(args, format);
    n = vsnprintf(got, sizeof(got), format, args);
    va_end(args);
    lost += ! _TEST_STR(got, want);
    lost += ! _TEST_INT(n, (int) strlen(want));
    return lost == 0;
}

// A format should assign a count of items, the first a string.
// [C99:7.19.6.7p3]: sscanf's count of items assigned, or EOF
static bool scans(const char *input, const char *format, int count, const char *want)
{
    int lost = 0;
    char got[HOLD];

    memset(got, FILL, sizeof(got));
    lost += ! _TEST_INT(sscanf(input, format, got), count);
    if (count == 1) {
        got[sizeof(got) - 1] = '\0';
        lost += ! _TEST_STR(got, want);
    }
    return lost == 0;
}

// A format should assign a count of items, the first an int.
// [C99:7.19.6.7p3]: sscanf's count of items assigned, or EOF
static bool scans_int(const char *input, const char *format, int count, int want)
{
    int lost = 0;
    int got = NONE;

    lost += ! _TEST_INT(sscanf(input, format, &got), count);
    lost += ! _TEST_INT(got, count == 1 ? want : NONE);
    return lost == 0;
}

// A format should assign a count of items, the first unsigned.
// [C99:7.19.6.7p3]: sscanf's count of items assigned, or EOF
static bool scans_unsigned(const char *input, const char *format, int count, unsigned int want)
{
    int lost = 0;
    unsigned int got = FILL;

    lost += ! _TEST_INT(sscanf(input, format, &got), count);
    lost += ! _TEST_UINT(got, count == 1 ? want : FILL);
    return lost == 0;
}

// A format should assign a count of items, the first a float.
// [C99:7.19.6.7p3]: sscanf's count of items assigned, or EOF
static bool scans_float(const char *input, const char *format, int count, float want)
{
    int lost = 0;
    float got = NONE;

    lost += ! _TEST_INT(sscanf(input, format, &got), count);
    lost += ! _TEST_FLOAT(got, count == 1 ? want : NONE);
    return lost == 0;
}

// A format should assign a count of items, the first a double.
// [C99:7.19.6.7p3]: sscanf's count of items assigned, or EOF
static bool scans_double(const char *input, const char *format, int count, double want)
{
    int lost = 0;
    double got = NONE;

    lost += ! _TEST_INT(sscanf(input, format, &got), count);
    lost += ! _TEST_FLOAT(got, count == 1 ? want : NONE);
    return lost == 0;
}

// A format should assign a count, the first a long double.
// [C99:7.19.6.7p3]: sscanf's count of items assigned, or EOF
static bool scans_long_double(const char *input, const char *format, int count, long double want)
{
    int lost = 0;
    long double got = NONE;

    lost += ! _TEST_INT(sscanf(input, format, &got), count);
    lost += ! _TEST_FLOAT(got, count == 1 ? want : NONE);
    return lost == 0;
}

// A format should assign a count of items, the first len wide
// characters, with nothing stored after them.
// [C99:7.19.6.7p3]: sscanf's count of items assigned, or EOF
static bool scans_wide(const char *input, const char *format, int count, const wchar_t *want, size_t len)
{
    int lost = 0;
    wchar_t got[HOLD];

    for (size_t i = 0; i < HOLD; i++) {
        got[i] = FILL;
    }
    lost += ! _TEST_INT(sscanf(input, format, got), count);
    if (count == 1) {
        lost += ! _TEST_MEM(got, want, len * sizeof(wchar_t));
        lost += ! _TEST_INT(got[len], FILL);
    } else {
        lost += ! _TEST_INT(got[0], FILL);
    }
    return lost == 0;
}
