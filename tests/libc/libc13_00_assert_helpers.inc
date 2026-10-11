// Includable helper file for <assert.h> tests.

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

// Defines.
#define FIVE 5
#define TEN  10

// Objects.
static int count;

// Return the number of calls made since count was cleared, this one counted.
static int bump(void)
{
    count++;
    return count;
}
