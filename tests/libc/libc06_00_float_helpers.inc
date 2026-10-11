// Includable helper file for <float.h> tests.

// Includes.
#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>

// Return a value times the radix to a power, which no rounding touches.
static long double scaled(long double value, int power)
{
    while (power > 0) {
        value *= FLT_RADIX;
        power--;
    }
    while (power < 0) {
        value /= FLT_RADIX;
        power++;
    }
    return value;
}
