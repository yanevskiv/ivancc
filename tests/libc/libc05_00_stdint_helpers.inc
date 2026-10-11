// Includable helper file for <stdint.h> tests.

// Includes.
#include <_test.h>
#include <stddef.h>
#include <stdbool.h>
#include <iso646.h>
#include <limits.h>
#include <stdint.h>

// Return the greatest value of an unsigned type of a size, with no padding.
static unsigned long umost(size_t size)
{
    return ULONG_MAX >> ((sizeof(unsigned long) - size) * CHAR_BIT);
}

// Return the greatest value of a signed type of a size, with no padding.
static long most(size_t size)
{
    return (long) (umost(size) >> 1);
}

// Return the least value of a signed type of a size, in two's complement.
static long least(size_t size)
{
    return -most(size) - 1;
}
