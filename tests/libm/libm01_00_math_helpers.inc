// Includable helper file for <math.h> tests.

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
#include <math.h>
#include <fenv.h>

// Return the long double of an x87 sign and exponent, and a significand
// whose top bit is the integer bit, so the encodings the x87 refuses too.
static long double ldouble_of(uint16_t top, uint64_t sig)
{
    long double value = 0.0L;
    memcpy(&value, &sig, sizeof(sig));
    memcpy((unsigned char *) &value + sizeof(sig), &top, sizeof(top));
    return value;
}
