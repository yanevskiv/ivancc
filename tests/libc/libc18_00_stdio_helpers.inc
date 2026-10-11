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
#define LONGER "a line longer than the buffer it passes through\n"
#define DIGITS "0123456789abcdefghijklmnopqrstuvwxyz"

// Bytes a text stream might change, which a binary one must not.
static const unsigned char bytes[BYTES] = {0, UCHAR_MAX, '\n', '\r', '\n', CTRL_Z, 'a', 0};

// Objects.
static unsigned char wide[WIDE];

// (S7.19.5.3p3) A file opened in a mode should take a string and close.
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

// (S7.19.7.2p2) fgets should read a stream's next line, or what is left.
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
